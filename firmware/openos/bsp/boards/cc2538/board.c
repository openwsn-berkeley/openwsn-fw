/**
\brief CC2538-specific definition of the "board" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, August 2013.
*/


#include "board.h"
// bsp modules
#include "leds.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

extern int mote_main();

int main() {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {

   leds_init();

}

void board_sleep() {

}

void board_reset() {

}

//=========================== private =========================================
