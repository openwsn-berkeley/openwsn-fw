/**
\brief nRF52840-specific definition of the "debugpins" bsp module.

\author Tamas Harczos <tamas.harczos@imms.de>, April 2018.
\author Tengfei Chang <tengfeichang@hkust-gz.edu.cn>, April 2023.
*/

#include "nrf52840.h"
#include "board_info.h"
#include "debugpins.h"

//=========================== defines =========================================

// board debug PINS defines

#define DEBUGPIN_FRAME  NRF_GPIO_PIN_MAP(0,26)
#define DEBUGPIN_SLOT   NRF_GPIO_PIN_MAP(0,27)
#define DEBUGPIN_FSM    NRF_GPIO_PIN_MAP(0,28)
#define DEBUGPIN_TASK   NRF_GPIO_PIN_MAP(0,29)
#define DEBUGPIN_ISR    NRF_GPIO_PIN_MAP(0,30)
#define DEBUGPIN_RADIO  NRF_GPIO_PIN_MAP(0,31)

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {

    NRF_P0->DIRSET = 1<<DEBUGPIN_FRAME;
    NRF_P0->DIRSET = 1<<DEBUGPIN_SLOT;
    NRF_P0->DIRSET = 1<<DEBUGPIN_FSM;
    NRF_P0->DIRSET = 1<<DEBUGPIN_TASK;
    NRF_P0->DIRSET = 1<<DEBUGPIN_ISR;
    NRF_P0->DIRSET = 1<<DEBUGPIN_RADIO;

}

void debugpins_frame_set(void) {

    NRF_P0->OUTSET = 1<<DEBUGPIN_FRAME;
}

void debugpins_frame_clr(void) {

    NRF_P0->OUTCLR = 1<<DEBUGPIN_FRAME;
}

void debugpins_frame_toggle(void) {

    if ((NRF_P0->OUT & (1<<DEBUGPIN_FRAME))!=0) {        
        NRF_P0->OUTCLR = 1<<DEBUGPIN_FRAME;
    } else {
        NRF_P0->OUTSET = 1<<DEBUGPIN_FRAME;
    }
}

void debugpins_slot_set(void) {

    NRF_P0->OUTSET = 1<<DEBUGPIN_SLOT;
}

void debugpins_slot_clr(void) {
    
    NRF_P0->OUTCLR = 1<<DEBUGPIN_SLOT;
}

void debugpins_slot_toggle(void) {

    if ((NRF_P0->OUT & (1<<DEBUGPIN_SLOT))!=0) {        
        NRF_P0->OUTCLR = 1<<DEBUGPIN_SLOT;
    } else {
        NRF_P0->OUTSET = 1<<DEBUGPIN_SLOT;
    }
}


void debugpins_fsm_set(void) {

    NRF_P0->OUTSET = 1<<DEBUGPIN_FSM;
}

void debugpins_fsm_clr(void) {
    
    NRF_P0->OUTCLR = 1<<DEBUGPIN_FSM;
}

void debugpins_fsm_toggle(void) {

    if ((NRF_P0->OUT & (1<<DEBUGPIN_FSM))!=0) {        
        NRF_P0->OUTCLR = 1<<DEBUGPIN_FSM;
    } else {
        NRF_P0->OUTSET = 1<<DEBUGPIN_FSM;
    }
}

void debugpins_task_set(void) {

    NRF_P0->OUTSET = 1<<DEBUGPIN_TASK;
}

void debugpins_task_clr(void) {

    NRF_P0->OUTCLR = 1<<DEBUGPIN_TASK;
}

void debugpins_task_toggle(void) {

    if ((NRF_P0->OUT & (1<<DEBUGPIN_TASK))!=0) {        
        NRF_P0->OUTCLR = 1<<DEBUGPIN_TASK;
    } else {
        NRF_P0->OUTSET = 1<<DEBUGPIN_TASK;
    }
}

void debugpins_isr_set(void) {
    
    NRF_P0->OUTSET = 1<<DEBUGPIN_ISR;
}

void debugpins_isr_clr(void) {

    NRF_P0->OUTCLR = 1<<DEBUGPIN_ISR;
}

void debugpins_isr_toggle(void) {

    if ((NRF_P0->OUT & (1<<DEBUGPIN_ISR))!=0) {        
        NRF_P0->OUTCLR = 1<<DEBUGPIN_ISR;
    } else {
        NRF_P0->OUTSET = 1<<DEBUGPIN_ISR;
    }
}

void debugpins_radio_set(void) {
    
    NRF_P0->OUTSET = 1<<DEBUGPIN_RADIO;
}

void debugpins_radio_clr(void) {
    
    NRF_P0->OUTCLR = 1<<DEBUGPIN_RADIO;
}

void debugpins_radio_toggle(void) {
    
    if ((NRF_P0->OUT & (1<<DEBUGPIN_RADIO))!=0) {        
        NRF_P0->OUTCLR = 1<<DEBUGPIN_RADIO;
    } else {
        NRF_P0->OUTSET = 1<<DEBUGPIN_RADIO;
    }
}