/**
\brief This is a program which shows how to use the "opentimers "driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
// driver modules required
#include "opentimers.h"

//=========================== defines =========================================

//=========================== variables =======================================

/*
typedef struct {
   
} app_vars_t;

app_vars_t app_vars;
*/

//=========================== prototypes ======================================

void cb_timer1();
void cb_timer2();
void cb_timer3();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void) {  
   board_init();
   opentimers_init();
   
   
}

//=========================== callbacks =======================================

void cb_timer1() {
   leds_error_toggle();
}

void cb_timer2() {
   leds_radio_toggle();
}

void cb_timer3() {
   leds_sync_toggle();
}