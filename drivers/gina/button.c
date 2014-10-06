#include "button.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void button_init() {
   // button connected to P2.7, i.e. configuration 0x80 in P2XX register
   P2DIR  &= ~0x80;                              // input direction
   P2REN  |=  0x80;                              // enable internal resistor
   P2OUT  |=  0x80;                              // put pin high as pushing button brings low
   P2IES  |=  0x80;                              // interrup when transition is high-to-low
   P2IE   |=  0x80;                              // enable interrupts
}

//=========================== private =========================================

