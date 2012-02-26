/**
\brief This program shows the use of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "leds.h"

//=========================== defines =========================================

#define LENGTH_PACKET 100
#define CHANNEL       11

//=========================== variables =======================================

void cb_radioTimerOverflows();
void cb_radioTimerCompare();
void cb_startFrame(uint16_t timestamp);
void cb_endFrame(uint16_t timestamp);

typedef struct {
   uint8_t radio_busy;
   uint8_t num_overflow;
   uint8_t num_compare;
   uint8_t num_startFrame;
   uint8_t num_endFrame;
} app_vars_t;

app_vars_t app_vars;

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void)
{  
   uint8_t packet[LENGTH_PACKET];
   uint8_t i;
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   
   // add callback functions from radio
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setCompareCb(cb_radioTimerCompare);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare packet
   for (i=0;i<sizeof(packet);i++) {
      packet[i] = i;
   }
   
   // send packet
   radio_setFrequency(CHANNEL);
   radio_rfOn();
   radio_loadPacket(packet,sizeof(packet));
   radio_txEnable();
   radio_txNow();
   app_vars.radio_busy = 1;
   while (app_vars.radio_busy==1) {
      board_sleep();
   }
   radio_rfOff();
   led_radio_toggle();
   
   // go back to sleep
   board_sleep();
}

//=========================== callbacks =======================================

void cb_radioTimerOverflows() {
   app_vars.num_overflow++;
}

void cb_radioTimerCompare() {
   app_vars.num_compare++;
}

void cb_startFrame(uint16_t timestamp) {
   app_vars.num_startFrame++;
}

void cb_endFrame(uint16_t timestamp) {
   app_vars.radio_busy = 0;
   app_vars.num_endFrame++;
}