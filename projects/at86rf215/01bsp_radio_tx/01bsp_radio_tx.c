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

#define LENGTH_PACKET   2043+LENGTH_CRC     // maximum length is 2047 bytes
#define CHANNEL         0                   // 
#define CHANNEL_SPACING 400                 // 400 kHz
#define FREQUENCY_0     863225            // 864.225 MHz
#define TIMER_PERIOD    (32768>>2)          // (32768>>2) = 250ms @ 32kHz
//#define TIMER_PERIOD    (65535)          // 2s @ 32kHz
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
   uint16_t             txpk_len;
   uint16_t             txpk_num;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_radioTimerOverflows(void);
void cb_radioTimerCompare(void);
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint16_t  i,x,y;
   x = 0;
   y = 0;
   //PORT_RADIOTIMER_WIDTH delay_tx;
   //delay_tx = 32768>>6;
   
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   P3DIR |= BIT4 | BIT5 | BIT6 | BIT7 ;   
   P3OUT |= BIT4 | BIT5 | BIT6 | BIT7 ;
   // add radio callback functions
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setCompareCb(cb_radioTimerCompare);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL_SPACING, FREQUENCY_0, CHANNEL) ; 
   radio_rfOff();

   // start periodic overflow
    radiotimer_start(TIMER_PERIOD);
   
    radio_change_size(&app_vars.txpk_len);
           
   //prepare packet
   for (i=2;i<app_vars.txpk_len;i++) {
        app_vars.txpk_buf[i] = i-1;
    }

    while(1){
        P3OUT     ^=  BIT4;
        P3OUT     ^=  BIT6;
      //while(y<21){
      while(y<4){

        while(x<100) {
                 
            // led
            leds_error_toggle();
      
            // prepare packet
            app_vars.txpk_num++;
            //app_vars.txpk_len           = sizeof(app_vars.txpk_buf);
            app_vars.txpk_buf[0]        = (uint8_t)((app_vars.txpk_num)>>8);
            app_vars.txpk_buf[1]        = (uint8_t)((app_vars.txpk_num)&0xFF);
            //for (i=2;i<app_vars.txpk_len;i++) {
            //    app_vars.txpk_buf[i] = i-1;
            //}
      
            // send packet
            radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
            radio_txEnable();
            P3OUT     ^=  BIT4;
            radio_txNow();
            P3OUT     ^=  BIT4;
            P3OUT     ^=  BIT6;
            x++;
            radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
            // wait for timer to elapse
            //app_vars.txpk_txNow = 0;
            //radiotimer_schedule( delay_tx + radiotimer_getPeriod());
            //while (app_vars.txpk_txNow==0) {
                //board_sleep();
            //}
            //radiotimer_cancel();
        }
        x = 0;
        y++;
        radio_change_size(&app_vars.txpk_len);
        //radio_change_modulation();
        app_vars.txpk_txNow = 0;
        while (app_vars.txpk_txNow==0) {
            board_sleep();
        }
        app_vars.txpk_txNow = 0;
        while (app_vars.txpk_txNow==0) {
            board_sleep();
        }
      }
      y = 0;
//      radio_change_modulation();
      //radio_change_size(&app_vars.txpk_len);
      //P3OUT     ^=  BIT4;
   }
}

//=========================== callbacks =======================================

void cb_radioTimerCompare(void) {
   
   // update debug vals
   app_dbg.num_radioTimerCompare++;
   //app_vars.txpk_txNow = 1;
}

void cb_radioTimerOverflows(void) {
   
   // update debug vals
   app_dbg.num_radioTimerOverflows++;
   
   // ready to send next packet
   app_vars.txpk_txNow = 1;
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
   
   // led
   leds_sync_off();
}
