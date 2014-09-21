/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

This application places the mote in receive mode, and prints, over the serial
port, all information about the received packet. The frame printed over the
serial port for each received packet is formatted as follows:
- [1B] the length of the packet, an unsigned integer
- [1B] the first byte of the packet, an unsigned integer
- [1B] the receive signal strength of tehe packet, an signed integer
- [1B] the link quality indicator, an unsigned integer
- [1B] whether the receive packet passed CRC (1) or not (0)
- [3B] closing flags, each of value 0xff

You can run the 01bsp_radio_rx.py script to listen to your mote and parse those 
serial frames. The application can connect directly to the mote's serial port,
or to its TCP port when running on the IoT-LAB platform.

Example when running locally:
----------------------------

 ___                 _ _ _  ___  _ _
| . | ___  ___ ._ _ | | | |/ __>| \ |
| | || . \/ ._>| ' || | | |\__ \|   |
`___'|  _/\___.|_|_||__/_/ <___/|_\_|
     |_|                  openwsn.org

running IoT-lAB? (Y|N): N
name of serial port (e.g. COM10): COM25
len=127 num=176 rssi=-43  lqi=107 crc=1
len=127 num=177 rssi=-43  lqi=107 crc=1
len=127 num=178 rssi=-43  lqi=106 crc=1
len=127 num=179 rssi=-43  lqi=107 crc=1
len=127 num=180 rssi=-43  lqi=108 crc=1
len=127 num=181 rssi=-43  lqi=107 crc=1
len=127 num=182 rssi=-43  lqi=107 crc=1
len=127 num=183 rssi=-43  lqi=107 crc=1


Example when running on the IoT-LAB platform:
--------------------------------------------

 ___                 _ _ _  ___  _ _
| . | ___  ___ ._ _ | | | |/ __>| \ |
| | || . \/ ._>| ' || | | |\__ \|   |
`___'|  _/\___.|_|_||__/_/ <___/|_\_|
     |_|                  openwsn.org

running IoT-lAB? (Y|N): Y
motename? (e.g. wsn430-35): wsn430-35
len=17  num=84  rssi=-80  lqi=107 crc=1
len=17  num=84  rssi=-81  lqi=107 crc=1
len=17  num=84  rssi=-80  lqi=107 crc=1
len=17  num=84  rssi=-81  lqi=105 crc=1
len=17  num=84  rssi=-80  lqi=108 crc=1
len=17  num=84  rssi=-81  lqi=108 crc=1


\author Xavi Vilajosana xvilajosana@eecs.berkeley.edu>, June 2012.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "uart.h"

//=========================== defines =========================================

#define LENGTH_PACKET        125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL              11             ///< 11 = 2.405GHz
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
   // rx packet
   volatile   uint8_t    rxpk_done;
              uint8_t    rxpk_buf[LENGTH_PACKET];
              uint8_t    rxpk_len;
              uint8_t    rxpk_num;
              int8_t     rxpk_rssi;
              uint8_t    rxpk_lqi;
              bool       rxpk_crc;
   // uart
              uint8_t    uart_txFrame[LENGTH_SERIAL_FRAME];
              uint8_t    uart_lastTxByte;
   volatile   uint8_t    uart_done;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

// radiotimer
void cb_radioTimerOverflows(void);
void cb_radioTimerCompare(void);
// radio
void cb_startFrame(uint16_t timestamp);
void cb_endFrame(uint16_t timestamp);
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
   
   // add callback functions radio
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setCompareCb(cb_radioTimerCompare);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // setup UART
   uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   // switch in RX
   radio_rxEnable();
   
   while (1) {
      
      // sleep while waiting for at least one of the rxpk_done to be set
      app_vars.rxpk_done = 0;
      while (app_vars.rxpk_done==0) {
         board_sleep();
      }
      
      // if I get here, I just received a packet
      
      //===== get packet from radio
      
      // led
      leds_sync_on();
      
      // get packet from radio
      radio_getReceivedFrame(
         app_vars.rxpk_buf,
         &app_vars.rxpk_len,
         sizeof(app_vars.rxpk_buf),
         &app_vars.rxpk_rssi,
         &app_vars.rxpk_lqi,
         &app_vars.rxpk_crc
      );
      
      // read the packet number
      app_vars.rxpk_num = app_vars.rxpk_buf[0];
      
      // led
      leds_sync_off();
      
      //===== send notification over serial port
      
      // led
      leds_error_on();
      
      // format frame to send over serial port
      app_vars.uart_txFrame[0] = app_vars.rxpk_len;  // packet length
      app_vars.uart_txFrame[1] = app_vars.rxpk_num;  // packet number
      app_vars.uart_txFrame[2] = app_vars.rxpk_rssi; // RSSI
      app_vars.uart_txFrame[3] = app_vars.rxpk_lqi;  // LQI
      app_vars.uart_txFrame[4] = app_vars.rxpk_crc;  // CRC
      app_vars.uart_txFrame[5] = 0xff;               // closing flag
      app_vars.uart_txFrame[6] = 0xff;               // closing flag
      app_vars.uart_txFrame[7] = 0xff;               // closing flag
      
      app_vars.uart_done          = 0;
      app_vars.uart_lastTxByte    = 0;
      
      // send app_vars.uart_txFrame over UART
      uart_clearTxInterrupts();
      uart_clearRxInterrupts();
      uart_enableInterrupts();
      uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
      while (app_vars.uart_done==0); // busy wait to finish
      uart_disableInterrupts();
      
      // led
      leds_error_off();
   }
}

//=========================== callbacks =======================================

//===== radiotimer

void cb_radioTimerOverflows(void) {
   
   // update debug stats
   app_dbg.num_radioTimerOverflows++;
}

void cb_radioTimerCompare(void) {
   
   // update debug stats
   app_dbg.num_radioTimerCompare++;
}

//===== radio

void cb_startFrame(uint16_t timestamp) {
   
   // update debug stats
   app_dbg.num_startFrame++;
}

void cb_endFrame(uint16_t timestamp) {
   
   // update debug stats
   app_dbg.num_endFrame++;
   
   // indicate I just received a packet
   app_vars.rxpk_done = 1;
}

//===== uart

void cb_uartTxDone(void) {
   
   uart_clearTxInterrupts();
   
   // prepare to send the next byte
   app_vars.uart_lastTxByte++;
   
   if (app_vars.uart_lastTxByte<sizeof(app_vars.uart_txFrame)) {
      uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
   } else {
      app_vars.uart_done=1;
   }
}

void cb_uartRxCb(void) {
   
   //  uint8_t byte;
   uart_clearRxInterrupts();
   
   // toggle LED
   leds_debug_toggle();
}