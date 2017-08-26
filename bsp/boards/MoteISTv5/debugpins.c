/**
\brief MoteISTv5-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
\author Diogo Guerra <diogoguerra@ist.utl.pt>, <dy090.guerra@gmail.com>, July 2015.
*/

#include "hal_MoteISTv5.h"
#include "debugpins.h"

//=========================== defines =========================================

#define PW0 BIT4  //P8.4   -frame
#define PW1 BIT3  //P8.3   -slot
#define PW2 BIT2  //P8.2   -fsm
#define PW3 BIT1  //P8.1   -task
#define PW4 BIT0  //P8.0   -isr
#define PW5 BIT3  //P7.3   -radio
#define PW6 BIT2  //P7.2   -NOTUSED

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {
   // all pins off
   debugpins_frame_clr();
   debugpins_slot_clr();
   debugpins_fsm_clr();
   debugpins_task_clr();
   debugpins_isr_clr();
   debugpins_radio_clr();
   
   // all pins as output and as General I/O
   P7DIR |= (PW5 | PW6);
   P7SEL &= ~(PW5 | PW6);
   P8DIR |= (PW0 | PW1 | PW2 | PW3 | PW4);
   P8SEL &= ~(PW0 | PW1 | PW2 | PW3 | PW4);
}

//P8.4   -frame
void debugpins_frame_toggle(void) {
   P8OUT ^=  PW0;
}
void debugpins_frame_clr(void) {
   P8OUT &= ~PW0;
}
void debugpins_frame_set(void) {
   P8OUT |=  PW0;
}

//P8.3   -slot
void debugpins_slot_toggle(void) {
   P8OUT ^=  PW1;
}
void debugpins_slot_clr(void) {
   P8OUT &= ~PW1;
}
void debugpins_slot_set(void) {
   P8OUT |=  PW1;
}

//P8.2   -fsm
void debugpins_fsm_toggle(void) {
   P8OUT ^=  PW2;
}
void debugpins_fsm_clr(void) {
   P8OUT &= ~PW2;
}
void debugpins_fsm_set(void) {
   P8OUT |=  PW2;
}

//P8.1   -task
void debugpins_task_toggle(void) {
   P8OUT ^=  PW3;
}
void debugpins_task_clr(void) {
   P8OUT &= ~PW3;
}
void debugpins_task_set(void) {
   P8OUT |=  PW3;
}

//P8.0   -isr
void debugpins_isr_toggle(void) {
   P8OUT ^=  PW4;
}
void debugpins_isr_clr(void) {
   P8OUT &= ~PW4;
}
void debugpins_isr_set(void) {
   P8OUT |=  PW4;
}

// radio [P6.0]
void debugpins_radio_toggle(void) {
   P7OUT ^=  PW5;
}
void debugpins_radio_clr(void) {
   P7OUT &= ~PW5;
}
void debugpins_radio_set(void) {
   P7OUT |=  PW5;
}

//=========================== private =========================================