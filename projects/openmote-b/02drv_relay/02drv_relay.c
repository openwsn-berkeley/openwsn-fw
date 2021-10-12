/**
\brief This is a program which shows how to use the bsp modules for the board
and UART.

\note: Since the bsp modules for different platforms have the same declaration,
you can use this project with any platform.

Load this program on your board. Open a serial terminal client (e.g. PuTTY or
TeraTerm):
- You will read "Hello World!" printed over and over on your terminal client.
- when you enter a character on the client, the board echoes it back (i.e. you
see the character on the terminal client) and the "ERROR" led blinks.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "stdio.h"
#include "string.h"
// bsp modules required
#include "board.h"
#include "uart.h"
#include "sctimer.h"
#include "leds.h"
#include "radio.h"

//=========================== defines =========================================
#define CPU_CLOCK_FREQ             32768  // 32kHz = 1s

// Serial input

#define SCTIMER_PERIOD     CPU_CLOCK_FREQ*5 
#define MAX_STRING_SIZE     100

// Protocol stack

#define LENGTH_PACKET   125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         11             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define TIMER_PERIOD    (32768>>1)     // (32768>>1) = 500ms @ 32kHz
#define NETID           38000          // Grenoble postal code
#define NETID_SIZE      2              // 2 bytes

uint8_t stringReceived[MAX_STRING_SIZE];
uint8_t writeIndex;
bool IAmTransmitter;

uint16_t net_id;
uint8_t payloadString[MAX_STRING_SIZE];
uint8_t payloadSize;


//=========================== variables =======================================

typedef struct {
  
  // UART TX related
  uint8_t uart_lastTxByteIndex;
  volatile   uint8_t uartDone;
  volatile   uint8_t uartSendNow;
  
  // MAC related
  
  // tx
  uint8_t               txpk_txNow;
  uint8_t               txpk_buf[LENGTH_PACKET];
  uint8_t               txpk_len;
  uint8_t               txpk_num;
  
  //rx
  volatile uint8_t      rxpk_done;
  uint8_t               rxpk_buf[LENGTH_PACKET];
  uint8_t              rxpk_len;
  uint8_t               rxpk_num;
  uint8_t               rxpk_src;
  int8_t                rxpk_rssi;
  uint8_t               rxpk_lqi;
  bool                  rxpk_crc;
  uint8_t               rxpk_freq_offset;
} app_vars_t;

app_vars_t app_vars;


typedef struct {
  uint8_t              num_scTimerCompare;
  uint8_t              num_startFrame;
  uint8_t              num_endFrame;
  uint8_t              counter;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

// timer callback
void cb_compare(void);

// UART callbacks
void cb_uartTxDone(void);
uint8_t cb_uartRxCb(void);

// radio callbacks
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
  
  // clear local variable
  memset(&app_vars,0,sizeof(app_vars_t));
  memset(&stringReceived,0,MAX_STRING_SIZE);
  memset(&payloadString,0,MAX_STRING_SIZE);
  writeIndex= 0;
  IAmTransmitter = false;
  app_dbg.counter = 0;
  
  app_vars.uartSendNow = 1;
  
  // initialize the board
  board_init();
  
  // setup UART
  uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);
  uart_enableInterrupts();
  
  // setup sctimer
  sctimer_set_callback(cb_compare);
  sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
  
  // setup the radios
  
  
  
  // cc2538 callbacks 
  radio_setConfig (RADIOSETTING_24GHZ);
  radio_setStartFrameCb(cb_startFrame);
  radio_setEndFrameCb(cb_startFrame);
  
  // atmel callbacks
  radio_setConfig (RADIOSETTING_FSK_OPTION1_FEC);
  radio_setStartFrameCb(cb_startFrame);
  radio_setEndFrameCb(cb_endFrame);
  
  // radio frequency
  radio_setFrequency(CHANNEL, FREQ_TX);
  radio_rfOn();
  
  while(1) {
    
    // if I am a designated transmitter, transmit. 
    while (IAmTransmitter == true){
      
      uint8_t i = 0;
      
      // prepare packet
      
      /*
        1B packet number
        1B hop number
        2B NET ID
        1B payload size
        100B payload
      */          
      app_vars.txpk_num++;
      app_vars.txpk_len             = sizeof(app_vars.txpk_buf);
      app_vars.txpk_buf[i++]        = app_vars.txpk_num;
      app_vars.txpk_buf[i++]        = 0;     // hop number
      app_vars.txpk_buf[i++]        = NETID >>8;
      app_vars.txpk_buf[i++]        = NETID & 0xFF;
      app_vars.txpk_buf[i++]        = payloadSize; 
      
      
      for (uint8_t j = 0; j< payloadSize; j++)
      {
        app_vars.txpk_buf[i++] = payloadString[j];
      }
      
      // now packet is ready
      
      while (app_vars.txpk_txNow==0) {
        board_sleep();
      }
      app_vars.txpk_txNow = 0;
      app_dbg.counter++;
      
      // transmit
      leds_error_toggle();
      radio_rfOff();
      radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
      radio_txEnable();
      radio_txNow();
      
      
    }
    
    // otherwise, listen and relay
    radio_setFrequency(CHANNEL, FREQ_RX);
    radio_rxEnable();
    radio_rxNow();
    
    app_vars.rxpk_done = 0;
    while (app_vars.rxpk_done==0 && IAmTransmitter == 0) {
      board_sleep();
    }
    // if I get here, I just received a packet
    
    
    //while (app_vars.uartSendNow==0);
    //app_vars.uartSendNow = 0;
    
    // send string over UART
    // app_vars.uartDone              = 0;
    // app_vars.uart_lastTxByteIndex  = 0;
    // uart_writeByte(payloadString[app_vars.uart_lastTxByteIndex]);
    // while(app_vars.uartDone==0);
  }
}

//=========================== callbacks =======================================

void cb_compare(void) {
  
  // send packet
  app_vars.txpk_txNow = 1;
  
  // app_vars.uartSendNow = 1;
  
  // schedule again
  sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
}

void cb_uartTxDone(void) {
  app_vars.uart_lastTxByteIndex++;
  if (app_vars.uart_lastTxByteIndex<sizeof(payloadString)) {
    uart_writeByte(payloadString[app_vars.uart_lastTxByteIndex]);
  } else {
    app_vars.uartDone = 1;
  }
}

uint8_t cb_uartRxCb(void) {
  uint8_t byte;
  
  
  // toggle LED
  leds_error_toggle();
  
  // read received byte
  byte = uart_readByte();
  
  // echo it back to the terminal
  uart_writeByte (byte);
  
  // store the received byte
  if (writeIndex < MAX_STRING_SIZE && byte !='\r')
  {
    stringReceived [writeIndex++] = byte;
  }else{
    // string complete
    payloadSize = writeIndex;
    writeIndex = 0;
    memcpy(payloadString, stringReceived,sizeof(stringReceived)+1);
    memset(&stringReceived,0,sizeof(stringReceived));
    
    IAmTransmitter = true;
  }
  return 0;
}

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
  
  // update debug vals
  app_dbg.num_startFrame++;
  
  // led
  leds_sync_on();
  
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
  
  // update debug vals
  app_dbg.num_endFrame++;
  
  bool     expectedFrame;
  
  // led
  leds_sync_off();
  
  app_dbg.num_endFrame++;
  app_vars.rxpk_done = 1 ;
  memset(&app_vars.rxpk_buf[0],0,LENGTH_PACKET);
  
  app_vars.rxpk_freq_offset = radio_getFrequencyOffset();
  
  // get packet from radio
  radio_getReceivedFrame(
                         app_vars.rxpk_buf,
                         &app_vars.rxpk_len,
                         sizeof(app_vars.rxpk_buf),
                         &app_vars.rxpk_rssi,
                         &app_vars.rxpk_lqi,
                         &app_vars.rxpk_crc
                           );
  
  // check the frame is sent by radio_tx project
  expectedFrame = TRUE;
}