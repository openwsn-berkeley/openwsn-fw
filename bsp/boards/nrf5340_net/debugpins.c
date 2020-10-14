/**
\brief nRF5340_network-specific definition of the "debugpins" bsp module.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "nRF5340_network.h"
#include "nrf5340_network_bitfields.h"
#include "debugpins.h"

//=========================== defines =========================================

#define PIN_PORT          1

#define PIN_FRAME         11    // p1.11  // left
#define PIN_SLOT          12    // p1.12  // right
#define PIN_FSM           13    // p1.13  // down
#define PIN_TASK          14    // p1.14  // up
#define PIN_ISR           25    // p1.25
#define PIN_RADIO         26    // p1.26

//=========================== variables =======================================

//=========================== prototypes ======================================

extern void nrf_gpio_cfg_output(uint8_t port_number, uint32_t pin_number);

//=========================== public ==========================================

// init
void debugpins_init(void) {

    nrf_gpio_cfg_output(PIN_PORT, PIN_FRAME);
    nrf_gpio_cfg_output(PIN_PORT, PIN_SLOT);
    nrf_gpio_cfg_output(PIN_PORT, PIN_FSM);
    nrf_gpio_cfg_output(PIN_PORT, PIN_TASK);
    nrf_gpio_cfg_output(PIN_PORT, PIN_ISR);
    nrf_gpio_cfg_output(PIN_PORT, PIN_RADIO);
}

// frame
void debugpins_frame_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_FRAME)) & (NRF_P1_NS->OUT);

    if (output_status>0) {
        // it is on , turn off led
        NRF_P1_NS->OUTCLR =  1 << PIN_FRAME;
    } else {
        // it is off, turn on led
        NRF_P1_NS->OUTSET =  1 << PIN_FRAME;
    }
}

void debugpins_frame_clr(void) {

    NRF_P1_NS->OUTCLR =  1 << PIN_FRAME;
}

void debugpins_frame_set(void) {
    
    NRF_P1_NS->OUTSET =  1 << PIN_FRAME;
}

// slot
void debugpins_slot_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_SLOT)) & (NRF_P1_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P1_NS->OUTCLR =  1 << PIN_SLOT;
    } else {
        // it is off, turn on led
        NRF_P1_NS->OUTSET =  1 << PIN_SLOT;
    }
}

void debugpins_slot_clr(void) {
    
    NRF_P1_NS->OUTCLR =  1 << PIN_SLOT;
}

void debugpins_slot_set(void) {
    
    NRF_P1_NS->OUTSET =  1 << PIN_SLOT;
}

// fsm
void debugpins_fsm_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_FSM)) & (NRF_P1_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P1_NS->OUTCLR =  1 << PIN_FSM;
    } else {
        // it is off, turn on led
        NRF_P1_NS->OUTSET =  1 << PIN_FSM;
    }
}

void debugpins_fsm_clr(void) {
    
     NRF_P1_NS->OUTCLR =  1 << PIN_FSM;
}

void debugpins_fsm_set(void) {
    
    NRF_P1_NS->OUTSET =  1 << PIN_FSM;
}

// task
void debugpins_task_toggle(void) {
    
        
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_TASK)) & (NRF_P1_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P1_NS->OUTCLR =  1 << PIN_TASK;
    } else {
        // it is off, turn on led
        NRF_P1_NS->OUTSET =  1 << PIN_TASK;
    }
}

void debugpins_task_clr(void) {

    NRF_P1_NS->OUTCLR =  1 << PIN_TASK;
}

void debugpins_task_set(void) {
    
    NRF_P1_NS->OUTSET =  1 << PIN_TASK;
}

// isr
void debugpins_isr_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISR)) & (NRF_P1_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P1_NS->OUTCLR =  1 << PIN_ISR;
    } else {
        // it is off, turn on led
        NRF_P1_NS->OUTSET =  1 << PIN_ISR;
    }
}

void debugpins_isr_clr(void) {
    
    NRF_P1_NS->OUTCLR=  1 << PIN_ISR;
}

void debugpins_isr_set(void) {
    
    NRF_P1_NS->OUTSET =  1 << PIN_ISR;
}

// radio
void debugpins_radio_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_RADIO)) & (NRF_P1_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P1_NS->OUTCLR =  1 << PIN_RADIO;
    } else {
        // it is off, turn on led
        NRF_P1_NS->OUTSET =  1 << PIN_RADIO;
    }
}

void debugpins_radio_clr(void) {
    
    NRF_P1_NS->OUTCLR=  1 << PIN_RADIO;
}

void debugpins_radio_set(void) {
    
    NRF_P1_NS->OUTSET =  1 << PIN_RADIO;
}

// isruarttx
void debugpins_isruarttx_toggle(void) {
    
    // TODO
}

void debugpins_isruarttx_clr(void) {
    
    // TODO
}

void debugpins_isruarttx_set(void) {

    // TODO
}

// isruartrx
void debugpins_isruartrx_toggle(void) {
    
    // TODO
}

void debugpins_isruartrx_clr(void) {
    
    // TODO
}

void debugpins_isruartrx_set(void) {
    
    // TODO
}

// intdisabled
void debugpins_intdisabled_toggle(void) {
    
    // TODO
}
void debugpins_intdisabled_clr(void) {

    // TODO
}
void debugpins_intdisabled_set(void) {
    
    // TODO
}

//=========================== private =========================================
