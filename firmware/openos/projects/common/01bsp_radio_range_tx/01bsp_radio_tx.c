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

#define LENGTH_PACKET   125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         26            // 2.480GHz
#define TIMER_ID        0
#define TIMER_PERIOD    32768          // 1s @ 32kHz

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
   uint8_t     packet_num;
    int8_t     rxpk_rssi;
   uint8_t     rxpk_lqi;
   uint8_t     rxpk_crc;
   uint8_t     lock;//semaphore
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

uint16_t getRandomPeriod();
void     cb_radioTimerOverflows();
void     cb_radioTimerCompare();
void     cb_startFrame(uint16_t timestamp);
void     cb_endFrame(uint16_t timestamp);
void     cb_timer();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t i;
   uint16_t j;
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   leds_error_on(); 
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
   
   bsp_timer_set_callback(cb_timer);
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL); 
   radio_rfOff();
               
   // start transmitting packet
   leds_radio_off();
   leds_sync_on();
   for (i=0;i<255;i++) {
      app_vars.packet_num=i;
      app_vars.packet[0]=i;//set the packet number.
      radio_loadPacket(app_vars.packet,app_vars.packet_len);
      radio_txEnable();
      radio_txNow();
      app_vars.lock=1;//get semaphore unitl tx done
      while ( app_vars.lock);
      for (j=0;j<0xFFFF;j++);
              leds_circular_shift();
            for (j=0;j<0xFFFF;j++);
                      leds_circular_shift();
                        for (j=0;j<0xFFFF;j++);
//      app_vars.lock=1;//get semaphore
//      bsp_timer_scheduleIn(327);//1ms
//    
//      while(app_vars.lock);//delay 1ms aprox
      leds_radio_toggle();
    }//end send.
   leds_sync_off();
   leds_radio_on();
   
   app_vars.state = APP_STATE_TX;
   while (1){
     board_sleep();
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
   
    app_vars.lock=0;//release semaphore
     
}

void cb_timer() {
   // set flag
   app_vars.flags |= APP_FLAG_TIMER;
   // update debug stats
   app_dbg.num_timer++;
   // release sem
   app_vars.lock=0;
}