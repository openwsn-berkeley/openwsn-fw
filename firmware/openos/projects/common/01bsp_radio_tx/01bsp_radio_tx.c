/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
#include "bsp_timer.h"

//=========================== defines =========================================

#define LENGTH_PACKET   30+LENGTH_CRC  // maximum length is 127 bytes
#define CHANNEL         20             // 2.480GHz
#define TIMER_PERIOD    (32768>>1)     // 500ms @ 32kHz

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
   uint8_t              num_radioTimerCompare;
   uint8_t              num_radioTimerOverflows;
   uint8_t              num_startFrame;
   uint8_t              num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t              packet[LENGTH_PACKET];
   uint8_t              packet_len;
   uint8_t              packet_num;
   uint8_t              sendPacket;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_radioTimerOverflows();
void cb_radioTimerCompare();
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
   
   // prepare packet
   app_vars.packet_len = sizeof(app_vars.packet);
   for (i=0;i<app_vars.packet_len;i++) {
      app_vars.packet[i] = i;
   }
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL); 
   radio_rfOff();
   
   // start periodic overflow
   radiotimer_start(TIMER_PERIOD);
   
   while(1) {
      
      // wait for timer to elapse
      app_vars.sendPacket = 0;
      while (app_vars.sendPacket==0) {
         board_sleep();
      }
      
      // led
      leds_error_toggle();
      
      // prepare packet
      app_vars.packet_num++;
      app_vars.packet[0]  = app_vars.packet_num;
      
      // send packet
      radio_loadPacket(app_vars.packet,app_vars.packet_len);
      radio_txEnable();
      radio_txNow();
   }
}

//=========================== callbacks =======================================

void cb_radioTimerCompare() {
   // update debug vals
   app_dbg.num_radioTimerCompare++;
}

void cb_radioTimerOverflows() {
   // update debug vals
   app_dbg.num_radioTimerOverflows++;
   
   // ready to send next packet
   app_vars.sendPacket = 1;
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
