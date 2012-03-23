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
#include "bsp_timers.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         26             // 2.480GHz
#define TIMER_ID        0
#define TIMER_DURATION  32768          // ~1s @ 32kHz

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
   uint8_t     flags;
   app_state_t state;
   uint8_t     packet[LENGTH_PACKET];
   uint8_t     packet_len;
    int8_t     rxpk_rssi;
   uint8_t     rxpk_lqi;
   uint8_t     rxpk_crc;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_radioTimerOverflows();
void cb_radioTimerCompare();
void cb_startFrame(uint16_t timestamp);
void cb_endFrame(uint16_t timestamp);
void cb_timer();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void)
{  
   uint8_t i;
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   
   // add callback functions radio
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setCompareCb(cb_radioTimerCompare);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare packet
   app_vars.packet_len = sizeof(app_vars.packet);
   for (i=0;i<app_vars.packet_len;i++) {
      app_vars.packet[i] = i;
   }
   
   // start timer
   timers_start(TIMER_ID,
                TIMER_DURATION,
                TIMER_PERIODIC,
                cb_timer);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   // switch in RX by default
   radio_rxEnable();
   app_vars.state = APP_STATE_RX;
   
   // start by a transmit
   app_vars.flags |= APP_FLAG_TIMER;

   while (1) {
      // sleep while waiting for at least one of the flags to be set
      while (app_vars.flags==0x00) {
         board_sleep();
      }
      // handle and clear every flag
      while (app_vars.flags) {

    	  if (app_vars.flags & APP_FLAG_START_FRAME) {
            // start of frame
            
            switch (app_vars.state) {
               case APP_STATE_RX:
                  // started receiving a packet
                  leds_error_on();
                  break;
               case APP_STATE_TX:
                  // started sending a packet
                  leds_sync_on();
                  break;
            }
            // clear flag
            app_vars.flags &= ~APP_FLAG_START_FRAME;
         }
         
         if (app_vars.flags & APP_FLAG_END_FRAME) {
            // end of frame
            
            switch (app_vars.state) {
               case APP_STATE_RX:
                  // done receiving a packet
                  
                  // get packet from radio
                  radio_getReceivedFrame(app_vars.packet,
                                         &app_vars.packet_len,
                                         sizeof(app_vars.packet),
                                         &app_vars.rxpk_rssi,
                                         &app_vars.rxpk_lqi,
                                         &app_vars.rxpk_crc);
                  
                  leds_error_off();
                  break;
               case APP_STATE_TX:
                  // done sending a packet
                  
                  // switch to RX mode
                  radio_rxEnable();
                  app_vars.state = APP_STATE_RX;
                  
                  leds_sync_off();
                  break;
            }
            // clear flag
            app_vars.flags &= ~APP_FLAG_END_FRAME;
         }
         
         if (app_vars.flags & APP_FLAG_TIMER) {
            // timer fired
            
            if (app_vars.state==APP_STATE_RX) {
               // stop listening
               radio_rfOff();
               
               // start transmitting packet
               radio_loadPacket(app_vars.packet,app_vars.packet_len);
               radio_txEnable();
               radio_txNow();
               
               app_vars.state = APP_STATE_TX;
            }
            
            // clear flag
            app_vars.flags &= ~APP_FLAG_TIMER;
         }
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
   app_vars.flags |= APP_FLAG_START_FRAME;
   // update debug stats
   app_dbg.num_startFrame++;
}

void cb_endFrame(uint16_t timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_END_FRAME;
   // update debug stats
   app_dbg.num_endFrame++;
}

void cb_timer() {
   // set flag
   app_vars.flags |= APP_FLAG_TIMER;
   // update debug stats
   app_dbg.num_timer++;
}
