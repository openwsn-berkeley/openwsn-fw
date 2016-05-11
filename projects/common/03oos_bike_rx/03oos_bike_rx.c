/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

The board running this program will send a packet on channel CHANNEL every
TIMER_PERIOD ticks. The packet contains LENGTH_PACKET bytes. The first byte
is the packet number, which increments for each transmitted packet. The
remainder of the packet contains an incrementing bytes.

\author Xavier Vilajosana <xvilajosana@uoc.edu>, May 2016.
\author Pere Tuset <peretuset@uoc.edu>, May 2016.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "uart.h"
#include "openrandom.h"
#include "leds.h"
#include "bsp_timer.h"
#include "idmanager.h"
#include "openhdlc.h"

//=========================== defines =========================================

#define LENGTH_PACKET        98+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL              26             ///< 11 = 2.405GHz
#define LENGTH_SERIAL_FRAME  8              ///< length of the serial frame

//=========================== variables =======================================

typedef struct {
   uint8_t    num_radioTimerOverflows;
   uint8_t    num_radioTimerCompare;
   uint8_t    num_startFrame;
   uint8_t    num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   // tx packet
   uint8_t               txpk_txNow;
   uint8_t               txpk_buf[LENGTH_PACKET];
   uint8_t               txpk_len;
   // rx packet
   volatile   uint8_t    rxpk_done;
              uint8_t    rxpk_buf[LENGTH_PACKET];
              uint8_t    rxpk_len;
              uint8_t    rxpk_num;
              int8_t     rxpk_rssi;
              uint8_t    rxpk_lqi;
              bool       rxpk_crc;
   // packet counter
   uint16_t              packet_counter;
   uint8_t               rollover;
   open_addr_t*          address;
   uint8_t               waitPacketEnd;
   // uart
              uint8_t    uart_txFrame[LENGTH_SERIAL_FRAME];
              uint8_t    uart_lastTxByte;
   volatile   uint8_t    uart_done;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void create_hdlc_frame(uint8_t* buffer, uint8_t* data, uint16_t length);

// radiotimer
void cb_radioTimerOverflows(void);
// radio
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);
// uart
void cb_uartTxDone(void);
void cb_uartRxCb(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   openrandom_init();
   idmanager_init();

   // Get EUI64
   app_vars.address = idmanager_getMyID(ADDR_64B);

   // Add radio callback functions
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);

   // Prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   radio_rfOff();
   
   while (1) {
      // Try to receive packet
      app_vars.rxpk_done = 0;
      radio_rfOn();
      radio_rxEnable();
      radio_rxNow();
      while (app_vars.rxpk_done == 0);

      // CRC should be valid and packet length should be 100 bytes
      if (app_vars.rxpk_crc == true && app_vars.rxpk_len == 100) {

         // Check that the packet comes from a bike/motorbike/car
         if (app_vars.rxpk_buf[0] == 0xBB ||
             app_vars.rxpk_buf[0] == 0xCC ||
             app_vars.rxpk_buf[0] == 0xDD) {

            // Append the RSSI and LQI
            app_vars.rxpk_buf[app_vars.rxpk_len++] = app_vars.rxpk_rssi;
            app_vars.rxpk_buf[app_vars.rxpk_len++] = app_vars.rxpk_lqi;

            // Create HDLC packet by copying all the bytes
            create_hdlc_frame(app_vars.uart_txFrame, app_vars.rxpk_buf, app_vars.rxpk_len); 

            // Send HDLC frame over UART
            uart_clearTxInterrupts();
            uart_clearRxInterrupts();
            uart_enableInterrupts();

            app_vars.uart_lastTxByte = 0;
            uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);

            // Busy wait to finish
            while (app_vars.uart_done == 0);

            // Disable UART interrupts
            uart_disableInterrupts();
         }
      }
   }
}

//=========================== callbacks =======================================

//===== radiotimer

void cb_radioTimerOverflows(void) {
   // update debug stats
   app_dbg.num_radioTimerOverflows++;
}

//===== radio

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   // update debug stats
   app_dbg.num_startFrame++;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   // Get packet from radio
   radio_getReceivedFrame(
      app_vars.rxpk_buf,
      &app_vars.rxpk_len,
      sizeof(app_vars.rxpk_buf),
      &app_vars.rxpk_rssi,
      &app_vars.rxpk_lqi,
      &app_vars.rxpk_crc
   );

   // Indicate I just received a packet
   app_vars.rxpk_done = 1;
}

//===== uart

void cb_uartTxDone(void) {
   // Clear the UART interrupt
   uart_clearTxInterrupts();
   
   // Prepare to send the next byte
   app_vars.uart_lastTxByte++;
   
   // Check 
   if (app_vars.uart_lastTxByte < sizeof(app_vars.uart_txFrame)) {
      uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
   } else {
      app_vars.uart_done = 1;
   }
}

void cb_uartRxCb(void) {
   uart_clearRxInterrupts();
   
   // toggle LED
   leds_debug_toggle();
}

//===== hdlc

void create_hdlc_frame(uint8_t* buffer, uint8_t* data, uint16_t length) {
   uint16_t crc, byte;
   uint16_t i = 0;
   uint16_t j = 0;

   // Initialize the value of the CRC
   crc = HDLC_CRCINIT;
   
   // write the opening HDLC flag
   buffer[j++] = HDLC_FLAG;

   // Iterate over all data to create the HDLC frame
   while (length > 0) {
      byte = data[i++];
      
      // Iterate the CRC
      crcIteration(crc, byte);

      // Add byte to buffer
      if (byte == HDLC_FLAG || byte == HDLC_ESCAPE) {
         buffer[j++] = HDLC_ESCAPE;
         byte = byte ^ HDLC_ESCAPE_MASK;
      }
   
      // Increment buffer pointer
      buffer[j++] = byte;
      length--;
   }
    
   // Finalize the calculation of the CRC
   crc = ~crc;
   
   // Write the CRC value
   buffer[j++] = (crc >> 0) & 0xFF;
   buffer[j++] = (crc >> 0) & 0xFF;
   
   // Write the closing HDLC flag
   buffer[j++] = HDLC_FLAG;
}  
