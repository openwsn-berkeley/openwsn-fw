/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

The board running this program will send a packet on channel CHANNEL every
TIMER_PERIOD ticks. The packet contains LENGTH_PACKET bytes. The first byte
is the packet number, which increments for each transmitted packet. The
remainder of the packet contains an incrementing bytes.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "bsp_timer.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         11             // 11 = 2.405GHz
#define TIMER_PERIOD    (32768>>1)     // (32768>>1) = 500ms @ 32kHz

//=========================== variables =======================================

typedef struct {
   uint8_t              num_radioTimerCompare;
   uint8_t              num_radioTimerOverflows;
   uint8_t              num_startFrame;
   uint8_t              num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t              txpk_txNow;
   uint8_t              txpk_buf[LENGTH_PACKET];
   uint8_t              txpk_len;
   uint8_t              txpk_num;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_radioTimerOverflows(void);
void cb_radioTimerCompare(void);
void cb_startFrame(uint16_t timestamp);
void cb_endFrame(uint16_t timestamp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t  i;
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   
   // add radio callback functions
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setCompareCb(cb_radioTimerCompare);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL); 
   radio_rfOff();
   
   // start periodic overflow
   radiotimer_start(TIMER_PERIOD);
   
   while(1) {
      
      // wait for timer to elapse
      app_vars.txpk_txNow = 0;
      while (app_vars.txpk_txNow==0) {
         board_sleep();
      }
      
      // led
      leds_error_toggle();
      
      // prepare packet
      app_vars.txpk_num++;
      app_vars.txpk_len           = sizeof(app_vars.txpk_buf);
      app_vars.txpk_buf[0]        = app_vars.txpk_num;
      for (i=1;i<app_vars.txpk_len;i++) {
         app_vars.txpk_buf[i] = i;
      }
      
      // send packet
      radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
      radio_txEnable();
      radio_txNow();
   }
}

//=========================== callbacks =======================================

void cb_radioTimerCompare(void) {
   
   // update debug vals
   app_dbg.num_radioTimerCompare++;
}

void cb_radioTimerOverflows(void) {
   
   // update debug vals
   app_dbg.num_radioTimerOverflows++;
   
   // ready to send next packet
   app_vars.txpk_txNow = 1;
}

void cb_startFrame(uint16_t timestamp) {
   
   // update debug vals
   app_dbg.num_startFrame++;
   
   // led
   leds_sync_on();
}

void cb_endFrame(uint16_t timestamp) {
   
   // update debug vals
   app_dbg.num_endFrame++;
   
   // led
   leds_sync_off();
}
