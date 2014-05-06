/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author xavi vilajosana xvilajosana@eecs.berkeley.edu>, June 2012
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "bsp_timer.h"
#include "uart.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         26             // 2.480GHz
#define TIMER_ID        0
#define TIMER_PERIOD    32768          // 1s @ 32kHz
#define TRUE 1
#define FALSE 0

//=========================== variables =======================================

enum {
   APP_FLAG_START_FRAME = 0x01,
   APP_FLAG_END_FRAME   = 0x02,
   APP_FLAG_TIMER       = 0x04,
};

typedef enum {
   APP_STATE_TX         = 0x01,
   APP_STATE_RX         = 0x02,
} app_state_t;

typedef struct {
   uint8_t num_radioTimerOverflows;
   uint8_t num_radioTimerCompare;
   uint8_t num_startFrame;
   uint8_t num_endFrame;
   uint8_t num_timer;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   volatile uint8_t     flags;
   volatile app_state_t state;
   uint8_t     packet[LENGTH_PACKET];
   uint8_t     packet_len;
   uint8_t     packet_num;
   int8_t     rxpk_rssi;
   uint8_t     rxpk_lqi;
   uint8_t     rxpk_crc;
   volatile uint8_t     uart_lastTxByte;
   volatile uint8_t     uart_end;//flag to indicate end of tx
} app_vars_t;

app_vars_t app_vars;

uint8_t stringToSend[5];//pkt,rssi,lqi,crc,FF(final)


//=========================== prototypes ======================================

uint16_t getRandomPeriod();
void     cb_radioTimerOverflows();
void     cb_radioTimerCompare();
void     cb_startFrame(uint16_t timestamp);
void     cb_endFrame(uint16_t timestamp);
void     cb_timer();
void cb_uartTxDone();
void cb_uartRxCb();


//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main() {
   
   // needed since we are disabling/enabling interrupts below
   INTERRUPT_DECLARATION();
   
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
   
   app_vars.uart_end=FALSE;
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   // switch in RX by default
   radio_rxEnable();
   
   app_vars.flags=0x00; //wait for rx
   
   while (1) {
      // sleep while waiting for at least one of the flags to be set
      while (app_vars.flags==0x00) {
         board_sleep();
      }
      // handle and clear every flag
      while(app_vars.flags) {
         DISABLE_INTERRUPTS();
         leds_sync_on();
         // done receiving a packet
         // get packet from radio
         radio_getReceivedFrame(app_vars.packet,
                                &app_vars.packet_len,
                                sizeof(app_vars.packet),
                                &app_vars.rxpk_rssi,
                                &app_vars.rxpk_lqi,
                                &app_vars.rxpk_crc);
         
         app_vars.packet_num=app_vars.packet[0];//packet number
         leds_error_off();
         stringToSend[0]=app_vars.packet_num;
         stringToSend[1]=app_vars.rxpk_rssi;
         stringToSend[2]=app_vars.rxpk_lqi;
         stringToSend[3]=app_vars.rxpk_crc;
         stringToSend[4]= 0xFF;
         
         //clear this interrupt.
         app_vars.flags = 0x00;
         app_vars.uart_end=FALSE;
         app_vars.uart_lastTxByte = 0;
         ENABLE_INTERRUPTS();  
         // send stringToSend over UART
         
         
         uart_clearTxInterrupts();
         uart_clearRxInterrupts();
         //uart_clearTxInterrupts();
         uart_enableInterrupts();
         uart_writeByte(stringToSend[app_vars.uart_lastTxByte]);
         
         while (app_vars.uart_end==FALSE);//wait to finish              
         uart_disableInterrupts();
         
         // clear flag
         
         leds_sync_off(); 
      }
   }
}

//=========================== callbacks =======================================

void cb_radioTimerOverflows() {
   app_dbg.num_radioTimerOverflows++;
}

void cb_radioTimerCompare() {
   app_dbg.num_radioTimerCompare++;
}

void cb_startFrame(uint16_t timestamp) {
   // set flag
   //app_vars.flags |= APP_FLAG_START_FRAME;
   // update debug stats
   app_dbg.num_startFrame++;
}

void cb_endFrame(uint16_t timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_END_FRAME;
   // update debug stats
   app_dbg.num_endFrame++;
}

void cb_uartTxDone() {
   uart_clearTxInterrupts();
   app_vars.uart_lastTxByte++;
   if (app_vars.uart_lastTxByte<sizeof(stringToSend)) {
      uart_writeByte(stringToSend[app_vars.uart_lastTxByte]);
   }else{
      app_vars.uart_end=TRUE;
   }
}

void cb_uartRxCb() {
   //  uint8_t byte;
   uart_clearRxInterrupts();
   // toggle LED
   leds_debug_toggle();
   
   // read received byte
   //byte = uart_readByte();
   
   // echo that byte over serial
   //uart_writeByte(byte);
}