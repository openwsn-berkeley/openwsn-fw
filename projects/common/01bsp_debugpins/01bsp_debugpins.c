/**
\brief This is a program to verify the correct functionality of the "debugpins"
       bsp module.

\note: Since the bsp modules for different platforms have the same declaration,
       you can use this project with any platform

Load this program on your board. When you run it, the error LED should blink.
Use a logic analyzer to see the activity on the 6 debug pins; one after another
they will transition 4 times.

\author Chuang Qian <cqian@berkeley.edu>, May 2012.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "debugpins.h"
#include "leds.h"

void some_delay(void);

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   board_init();
   leds_error_on();
   
   debugpins_frame_set();    some_delay();
   debugpins_frame_toggle(); some_delay();
   debugpins_frame_toggle(); some_delay();
   debugpins_frame_clr();    some_delay();
   
   debugpins_slot_set();     some_delay();
   debugpins_slot_toggle();  some_delay();
   debugpins_slot_toggle();  some_delay();
   debugpins_slot_clr();     some_delay();
   
   debugpins_fsm_set();      some_delay();
   debugpins_fsm_toggle();   some_delay();
   debugpins_fsm_toggle();   some_delay();
   debugpins_fsm_clr();      some_delay();
   
   debugpins_task_set();     some_delay();
   debugpins_task_toggle();  some_delay();
   debugpins_task_toggle();  some_delay();
   debugpins_task_clr();     some_delay();
   
   debugpins_isr_set();      some_delay();
   debugpins_isr_toggle();   some_delay();
   debugpins_isr_toggle();   some_delay();
   debugpins_isr_clr();      some_delay();
   
   debugpins_radio_set();    some_delay();
   debugpins_radio_toggle(); some_delay();
   debugpins_radio_toggle(); some_delay();
   debugpins_radio_clr();    some_delay();
   
   board_reset();
   
   return 0;
}

void some_delay(void) {
   volatile uint8_t delay;
   for (delay=0xff;delay>0;delay--);
}