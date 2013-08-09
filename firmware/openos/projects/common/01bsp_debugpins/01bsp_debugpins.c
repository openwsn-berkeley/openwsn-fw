/**
\brief This is a program to verify the correct functionality of the "debugpins"
       bsp module.

\author Chuang Qian <cqian@berkeley.edu>, May 2012.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "debugpins.h"

/**
\brief The program starts executing here.
*/
int mote_main() {
   uint16_t delay;
   
   board_init();
   
   while (1) {
      for (delay=0xffff;delay>0;delay--);
      debugpins_frame_toggle();
      debugpins_slot_toggle();
      debugpins_fsm_toggle();
      debugpins_task_toggle();
      debugpins_isr_toggle();
      debugpins_radio_toggle();
   }
}