/**
\brief This program shows the use of the "radio" as a jammer

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

The board running this program will continuously send a packet on channel CHANNEL
The packet contains LENGTH_PACKET bytes. The send bytes are random.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, September 2016.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"
//#include "bsp_timer.h"

//=========================== defines =========================================

#define LENGTH_PACKET   100+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL         11             // 11 = 2.405GHz
#define RANDOM_SEED     65521          // prime

//=========================== variables =======================================

typedef struct {
   uint8_t              num_startFrame;
   uint8_t              num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t              txpk_txNow;
   uint8_t              txpk_buf[LENGTH_PACKET];
   uint8_t              txpk_len;
   uint8_t              txpk_num;
   uint16_t             shift_reg;  // Galois shift register used to obtain a pseudo-random number
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

uint16_t getrandom16b(void);
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t  i;
   uint16_t ran;
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();

   app_vars.shift_reg = RANDOM_SEED;

   
   // add radio callback functions
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL,FREQ_RX); 
   radio_set_modulation (FSK_OPTION1_FEC);
   radio_rfOff();
   //delay
   //for (j=0;j<0xffff;j++);;
         
   while(1) {

      // led
      leds_error_on();
      
      // prepare packet
      app_vars.txpk_num++;
      app_vars.txpk_len           = sizeof(app_vars.txpk_buf);
      app_vars.txpk_buf[0]        = app_vars.txpk_num;
      for (i=1;i<app_vars.txpk_len-1;i++) {

         ran = getrandom16b();

         app_vars.txpk_buf[i] = ran & 0xff;
         app_vars.txpk_buf[i+1] = (ran >> 8) & 0xff;
      }
      
      // send packet
      radio_loadPacket(app_vars.txpk_buf,app_vars.txpk_len);
      radio_set_modulation (FSK_OPTION1_FEC);
      radio_txEnable();
      //set a semaphore to wait until the tx finished
      app_vars.txpk_txNow = 0;

      radio_txNow();
      //wait until done
      while (app_vars.txpk_txNow==0) {
         board_sleep();
      }
      leds_error_off();
      radio_rfOff();
   }
}

//=========================== callbacks =======================================


void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   
   // update debug vals
   app_dbg.num_startFrame++;
   
   // led
   leds_sync_on();
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   
   // update debug vals
   app_dbg.num_endFrame++;
   //toggle the semaphore
   app_vars.txpk_txNow = 1;
   // led
   leds_sync_off();
}


uint16_t getrandom16b() {
   uint8_t  i;
   uint16_t random_value;
   random_value = 0;
   for(i=0;i<16;i++) {
      // Galois shift register
      // taps: 16 14 13 11
      // characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1
      random_value          |= (app_vars.shift_reg & 0x01)<<i;
      app_vars.shift_reg  = (app_vars.shift_reg>>1)^(-(int16_t)(app_vars.shift_reg & 1)&0xb400);
   }
   return random_value;
}

