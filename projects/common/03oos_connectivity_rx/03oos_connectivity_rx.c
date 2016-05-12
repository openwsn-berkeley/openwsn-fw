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

//=========================== includes ========================================

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

#define LENGTH_PACKET        ( 98 + LENGTH_CRC )
#define RADIO_CHANNEL        ( 26 )
#define LENGTH_SERIAL_FRAME  ( 64 )
#define TIMER_PERIOD         ( 32768 )

//=========================== variables =======================================

typedef struct {
   // rx packet
   volatile bool         rxpk_isRx;
   volatile bool         rxpk_busy;
   volatile bool         rxpk_done;
            uint8_t      rxpk_buf[LENGTH_PACKET];
            uint8_t      rxpk_len;
            uint8_t      rxpk_num;
            int8_t       rxpk_rssi;
            uint8_t      rxpk_lqi;
            bool         rxpk_crc;
   
   // tx packet
   volatile bool         txpk_isTx;
   volatile bool         txpk_busy;
   volatile bool         txpk_done;
            uint8_t      txpk_buf[LENGTH_PACKET];
            uint8_t      txpk_len;
   
   // packet counter
   uint16_t              packet_counter;
   uint8_t               rollover;
   open_addr_t*          address;

   // timer
   bool                  transmitFrame;
   PORT_RADIOTIMER_WIDTH packet_period;

   // UNIX timestamp
   uint8_t               timestamp_0;
   uint8_t               timestamp_1;
   uint8_t               timestamp_2;
   uint8_t               timestamp_3;
   
   // uart tx
            uint8_t      uart_txFrame[LENGTH_SERIAL_FRAME];
            uint8_t      uart_txLastByte;
            uint8_t      uart_txCounter;
   volatile uint8_t      uart_txDone;
   
   // uart rx
            uint8_t      uart_rxFrame[LENGTH_SERIAL_FRAME];
            bool         uart_rxReceiving;
            bool         uart_rxEscaping;
            uint16_t     uart_rxCrc;
            uint8_t      uart_rxLastByte;
            uint8_t      uart_rxCounter;
   volatile uint8_t      uart_rxDone;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

bool radio_try_rx_frame(void);
void radio_rx_process(void);
void prepare_radio_tx_frame(void);
void radio_tx_frame(void);

uint16_t create_hdlc_frame(uint8_t* buffer, uint8_t* data, uint16_t length);
bool append_hdlc_frame(uint8_t byte);
void process_hdlc_byte(uint8_t byte);
void process_hdlc_frame(void);

// radiotimer
void cb_radioTimerOverflows(void);
// radio
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);
// uart
void cb_uartTxDone(void);
void cb_uartRxCb(void);

//=========================== main ============================================

int mote_main(void) {
   // Clear local variables
   memset(&app_vars, 0, sizeof(app_vars_t));

   // Init the time stamp to a known value
   app_vars.timestamp_0 = 0xBA;
   app_vars.timestamp_1 = 0xBE;
   app_vars.timestamp_2 = 0xFA;
   app_vars.timestamp_3 = 0xCE;
   
   // Initialize board
   board_init();
   openrandom_init();
   idmanager_init();

   // Get EUI64
   app_vars.address = idmanager_getMyID(ADDR_64B);

   // Setup UART callback functions
   uart_setCallbacks(cb_uartTxDone, cb_uartRxCb);

   // Prepare UART
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();
   uart_enableInterrupts();

   // Add radio callback functions
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);

   // Prepare radio
   radio_rfOn();
   radio_setFrequency(RADIO_CHANNEL);
   radio_rfOff();

   // Random packet transmission rate
   app_vars.packet_period = openrandom_get16b() % TIMER_PERIOD + TIMER_PERIOD;
   radiotimer_start(app_vars.packet_period);
   
   // Forever and ever!
   while (true) {
      // Try to receive packet, blocking
      // Can be unlocked when a frame is received
      // or when a timeout expires
      if (radio_try_rx_frame() == true) {
         // Process received frame, blocking
         radio_rx_process();
      }

      // Check if we need to update the timestamp
      // This happens asynchronously when a timeout expires
      // The timestamp is updated asynchronously from the UART
      if (app_vars.transmitFrame == true) {
         // Prepare the packet
         prepare_radio_tx_frame();

         // Transmit the packet
         radio_tx_frame();
      }
   }
}

bool radio_try_rx_frame(void) {
   // Restore variables state
   app_vars.rxpk_isRx = true;
   app_vars.rxpk_busy = false;
   app_vars.rxpk_done = false;
   app_vars.rxpk_crc  = false;
   app_vars.rxpk_len  = 0;

   // Turn on radio to receive
   radio_rfOn();
   radio_rxEnable();
   radio_rxNow();

   // Wait until we receive a packet or a timeout from the timer
   while (app_vars.rxpk_isRx == true);

   // Return if we have received a valid frame
   return (app_vars.rxpk_done & app_vars.rxpk_crc);
}

void radio_rx_process(void) {
   // Packet length should be 100 bytes and
   // comes from either a bike or motorbike/car
   if ((app_vars.rxpk_len == 100) &&
      ((app_vars.rxpk_buf[0] == 0xAA) || (app_vars.rxpk_buf[0] == 0x55))) {
      
      // Move RSSI and LQI to discard other data
      app_vars.rxpk_buf[16] = app_vars.rxpk_rssi;
      app_vars.rxpk_buf[17] = app_vars.rxpk_lqi;
      app_vars.rxpk_len     = 18;

      // Create HDLC packet by copying all the bytes
      app_vars.uart_txCounter = create_hdlc_frame(app_vars.uart_txFrame, app_vars.rxpk_buf, app_vars.rxpk_len); 

      // Restore UART variables
      app_vars.uart_txDone     = false;
      app_vars.uart_txLastByte = 0;

      // Write first byte to UART to trigger interrupt-based transmission
      uart_writeByte(app_vars.uart_txFrame[app_vars.uart_txLastByte]);

      // Busy wait to finish
      while (app_vars.uart_txDone == false);
   }
}

void prepare_radio_tx_frame(void) {
   uint8_t i;

   // Upate packet length
   app_vars.txpk_len = sizeof(app_vars.txpk_buf);
   
   // Mote type (0xAA = Bike, 0x55 = Motorike / Car)
   app_vars.txpk_buf[0] = 0x55;

   // Copy EUI64 as identifier
   memcpy(&app_vars.txpk_buf[1], &app_vars.address->addr_64b[0], 8);

   // Increment packet counter
   app_vars.packet_counter++;

   // Detecting rollover with lollipop counter
   if (app_vars.packet_counter % 0xFFFFFFFF == 0){
      app_vars.rollover++;
      app_vars.packet_counter = 256;
   }

   // Fill in packet counter and rollover counter
   app_vars.txpk_buf[9]  = (app_vars.packet_counter >> 8) % 0xFF;
   app_vars.txpk_buf[10] = (app_vars.packet_counter >> 0) % 0xFF;;
   app_vars.txpk_buf[11] = app_vars.rollover;

   // Epoch set to last UNIX date as this is a motorbike/car
   app_vars.txpk_buf[12] = app_vars.timestamp_0;
   app_vars.txpk_buf[13] = app_vars.timestamp_1;
   app_vars.txpk_buf[14] = app_vars.timestamp_2;
   app_vars.txpk_buf[15] = app_vars.timestamp_3;

   // Fill remaining of packet
   for (i = 16; i < app_vars.txpk_len; i++) {
      app_vars.txpk_buf[i] = i;
   }
}

void radio_tx_frame(void) {
   // Enable radio
   radio_rfOn();

   // Load packet to radio
   radio_loadPacket(app_vars.txpk_buf, app_vars.txpk_len);

   // Transmit radio frame
   radio_txEnable();
   radio_txNow();

   // Radio is asynchronous, so busy-wait until the packet is complete
   app_vars.txpk_done = false;
   app_vars.txpk_isTx = true;
   while (app_vars.txpk_done == false);

   // Stop the radio once the packet is compete
   radio_rfOff();

   // Notify that have transmittd the frame
   app_vars.transmitFrame = false;
}

//=========================== callbacks =======================================

//===== radiotimer

void cb_radioTimerOverflows(void) {
   // If radio is receiving but no packet so far
   if ((app_vars.rxpk_isRx == true) && 
       (app_vars.rxpk_busy == false)) {

      // Turn off radio to let other stuff go
      radio_rfOff();

      // Notify we are no longer receiving
      app_vars.rxpk_isRx = false;
   }

   // Notify we need to transmit a frame
   app_vars.transmitFrame = true;
}

//===== radio

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   leds_error_on();

   // Radio is receiving
   if (app_vars.rxpk_isRx == true) {
      // Now, the radio is busy receiving a packet,
      // so we cannot interrupt it!
      app_vars.rxpk_busy = true;
   }

   // Radio is transmiting
   if (app_vars.txpk_isTx == true) {
      // Now, the radio is busy receiving a packet,
      // so we cannot interrupt it!
      app_vars.txpk_busy = true;
   }
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   leds_error_off();

   // Radio is receiving and busy
   if ((app_vars.rxpk_isRx == true) && 
       (app_vars.rxpk_busy == true)) {
      // Get packet out from radio
      radio_getReceivedFrame(
         app_vars.rxpk_buf,
         &app_vars.rxpk_len,
         sizeof(app_vars.rxpk_buf),
         &app_vars.rxpk_rssi,
         &app_vars.rxpk_lqi,
         &app_vars.rxpk_crc
      );

      // Indicate we just received a packet
      app_vars.rxpk_isRx = false;
      app_vars.rxpk_busy = false;
      app_vars.rxpk_done = true;
   }

   // Radio is transmiting and busy
   if ((app_vars.txpk_isTx == true) && 
       (app_vars.txpk_busy == true)) {
      
      // Indicate we just transmitted a packet
      app_vars.txpk_isTx = false;
      app_vars.txpk_busy = false;
      app_vars.txpk_done = true;
   }
}

//===== uart

void cb_uartTxDone(void) {
   // Clear the UART TX interrupt
   uart_clearTxInterrupts();
   
   // Prepare to send the next byte
   app_vars.uart_txLastByte++;
   
   // Transmit the UART bytes
   if (app_vars.uart_txLastByte < app_vars.uart_txCounter) {
      uart_writeByte(app_vars.uart_txFrame[app_vars.uart_txLastByte]);
   } else {
      app_vars.uart_txDone = true;
   }
}

void cb_uartRxCb(void) {
   uint8_t byte;
   bool status;

   // Clear the UART RX interrupt
   uart_clearRxInterrupts();

   // Get byte from the UART
   byte = uart_readByte();

   // Process the received byte
   status = append_hdlc_frame(byte);

   // If we have a HDLC frame, process it
   if (status == true) {
      process_hdlc_frame();
   }
}

//===== hdlc

uint16_t create_hdlc_frame(uint8_t* buffer, uint8_t* data, uint16_t length) {
   uint16_t counter = 0;
   uint16_t crc, byte;
   uint16_t i = 0;
   uint16_t j = 0;

   // Initialize the value of the CRC
   crc = HDLC_CRCINIT;
   
   // write the opening HDLC flag
   buffer[j++] = HDLC_FLAG;
   counter++;

   // Iterate over all data to create the HDLC frame
   while (length > 0) {
      byte = data[i++];

      // Add byte to buffer
      if (byte == HDLC_FLAG || byte == HDLC_ESCAPE) {
         buffer[j++] = HDLC_ESCAPE;
         byte = byte ^ HDLC_ESCAPE_MASK;
         counter++;
      }

      // Iterate the CRC
      crc = crcIteration(crc, byte);
   
      // Increment buffer pointer
      buffer[j++] = byte;
      length--;
      counter++;
   }
    
   // Finalize the calculation of the CRC
   crc = ~crc;
   
   // Write the CRC value
   buffer[j++] = (crc >> 0) & 0xFF;
   buffer[j++] = (crc >> 8) & 0xFF;
   
   // Write the closing HDLC flag
   buffer[j++] = HDLC_FLAG;
   counter += 3;

   return counter;
}  

bool append_hdlc_frame(uint8_t byte) {
   bool frameStatus = false;

   // Start of HDLC frame
   if ((app_vars.uart_rxReceiving == false) &&
       (app_vars.uart_rxLastByte == HDLC_FLAG) && 
       (byte != HDLC_FLAG)) {
      // I'm now receiving
      app_vars.uart_rxReceiving = true;
      
      // Reset the input buffer index
      app_vars.uart_rxCounter = 0;

      // Unitialize the value of the CRC
      app_vars.uart_rxCrc = HDLC_CRCINIT;
      
      // Add the byte just received
      process_hdlc_byte(byte);
   }
   
   // Middle of HDLC frame, append received byte
   else if ((app_vars.uart_rxReceiving == true) &&
            (byte != HDLC_FLAG)) {
      // Add the byte just received
      process_hdlc_byte(byte);
   }
   
   // End of HDLC frame, process it
   else if ((app_vars.uart_rxReceiving == true) &&
            (byte == HDLC_FLAG)) {
         // Verify the validity of the HDLC frame using CRC
         if (app_vars.uart_rxCrc == HDLC_CRCGOOD) {
            // We have received a frame, process it!
            frameStatus = true;
            // Remove the CRC from the input buffer
            app_vars.uart_rxCounter -= 2;
         } else {
            // Drop the incoming frame
            app_vars.uart_rxCounter = 0;
         }
         
         // We have finished the frame
         app_vars.uart_rxReceiving = false;
   }
   
   // Remember the last received byte
   app_vars.uart_rxLastByte = byte;

   return frameStatus;
}

void process_hdlc_byte(uint8_t byte) {
   if (byte == HDLC_ESCAPE) {
      app_vars.uart_rxEscaping = true;
   } else {
      if (app_vars.uart_rxEscaping == true) {
         byte = byte ^ HDLC_ESCAPE_MASK;
         app_vars.uart_rxEscaping = false;
      }
      
      // Add byte to input buffer
      app_vars.uart_rxFrame[app_vars.uart_rxCounter] = byte;
      app_vars.uart_rxCounter++;
      
      // Iterate through CRC calculator
      app_vars.uart_rxCrc = crcIteration(app_vars.uart_rxCrc, byte);
   }
}

void process_hdlc_frame(void) {
   // We try to receive a timestamp, 
   // which should be 5 bytes and start with 0xEE
   if ((app_vars.uart_rxCounter == 5) && 
       (app_vars.uart_rxFrame[0] == 0xEE)) {
      // Update the timestamp value
      app_vars.timestamp_0 = app_vars.uart_rxFrame[1];
      app_vars.timestamp_1 = app_vars.uart_rxFrame[2];
      app_vars.timestamp_2 = app_vars.uart_rxFrame[3];
      app_vars.timestamp_3 = app_vars.uart_rxFrame[4];
   }
}
