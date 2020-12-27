/**
\brief nRF52840-specific definition of the "debugpins" bsp module.

\author Tengfei Chang <tengfei@gmail.com>, July 2020.
*/

#include "nrf52833.h"
#include "nrf52833_bitfields.h"
#include "debugpins.h"

//=========================== defines =========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define PIN_FRAME         NRF_GPIO_PIN_MAP(0,3)     // p0.3
#define PIN_SLOT          NRF_GPIO_PIN_MAP(0,4)     // p0.4
#define PIN_FSM           NRF_GPIO_PIN_MAP(0,28)    // p0.28
#define PIN_TASK          NRF_GPIO_PIN_MAP(0,29)    // p0.29
#define PIN_ISR           NRF_GPIO_PIN_MAP(0,30)    // p0.30
#define PIN_RADIO         NRF_GPIO_PIN_MAP(0,31)    // p0.31
#define PIN_ISRUART_TX    NRF_GPIO_PIN_MAP(0,11)    // p0.11
#define PIN_ISRUART_RX    NRF_GPIO_PIN_MAP(0,12)    // p0.12
#define PIN_INTDISABLED   NRF_GPIO_PIN_MAP(0,25)    // p0.25

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

// init
void debugpins_init(void) {

#ifndef MONOLETS_TAG

    nrf_gpio_cfg_output(PIN_FRAME);
    nrf_gpio_cfg_output(PIN_SLOT);
    nrf_gpio_cfg_output(PIN_FSM);
    nrf_gpio_cfg_output(PIN_TASK);
    nrf_gpio_cfg_output(PIN_ISR);
    nrf_gpio_cfg_output(PIN_RADIO);
    nrf_gpio_cfg_output(PIN_ISRUART_TX);
    nrf_gpio_cfg_output(PIN_ISRUART_RX);
    nrf_gpio_cfg_output(PIN_INTDISABLED);

#endif

}

// frame
void debugpins_frame_toggle(void) {

#ifndef MONOLETS_TAG

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_FRAME)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_FRAME;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_FRAME;
    }

#endif
}

void debugpins_frame_clr(void) {

#ifndef MONOLETS_TAG

    NRF_P0->OUTCLR =  1 << PIN_FRAME;

#endif
}

void debugpins_frame_set(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTSET =  1 << PIN_FRAME;
#endif
}

// slot
void debugpins_slot_toggle(void) {

#ifndef MONOLETS_TAG

    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_SLOT)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_SLOT;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_SLOT;
    }

#endif
}

void debugpins_slot_clr(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTCLR =  1 << PIN_SLOT;

#endif
}

void debugpins_slot_set(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTSET =  1 << PIN_SLOT;

#endif
}

// fsm
void debugpins_fsm_toggle(void) {

#ifndef MONOLETS_TAG

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_FSM)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_FSM;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_FSM;
    }

#endif
}

void debugpins_fsm_clr(void) {

#ifndef MONOLETS_TAG
    
     NRF_P0->OUTCLR =  1 << PIN_FSM;

#endif
}

void debugpins_fsm_set(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTSET =  1 << PIN_FSM;

#endif
}

// task
void debugpins_task_toggle(void) {

#ifndef MONOLETS_TAG
        
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_TASK)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_TASK;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_TASK;
    }

#endif
}

void debugpins_task_clr(void) {

#ifndef MONOLETS_TAG

    NRF_P0->OUTCLR =  1 << PIN_TASK;

#endif
}

void debugpins_task_set(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTSET =  1 << PIN_TASK;

#endif
}

// isr
void debugpins_isr_toggle(void) {

#ifndef MONOLETS_TAG

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISR)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_ISR;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_ISR;
    }

#endif
}

void debugpins_isr_clr(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTCLR=  1 << PIN_ISR;

#endif
}

void debugpins_isr_set(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTSET =  1 << PIN_ISR;

#endif
}

// radio
void debugpins_radio_toggle(void) {

#ifndef MONOLETS_TAG

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_RADIO)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_RADIO;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_RADIO;
    }

#endif
}

void debugpins_radio_clr(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTCLR=  1 << PIN_RADIO;

#endif
}

void debugpins_radio_set(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTSET =  1 << PIN_RADIO;

#endif
}

// isruarttx
void debugpins_isruarttx_toggle(void) {

#ifndef MONOLETS_TAG
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISRUART_TX)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_ISRUART_TX;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_ISRUART_TX;
    }

#endif
}

void debugpins_isruarttx_clr(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTCLR =  1 << PIN_ISRUART_TX;

#endif
}

void debugpins_isruarttx_set(void) {
    
#ifndef MONOLETS_TAG

    NRF_P0->OUTSET =  1 << PIN_ISRUART_TX;

#endif
}

// isruartrx
void debugpins_isruartrx_toggle(void) {

#ifndef MONOLETS_TAG
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISRUART_RX)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_ISRUART_RX;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_ISRUART_RX;
    }

#endif
}

void debugpins_isruartrx_clr(void) {

#ifndef MONOLETS_TAG
    
    NRF_P0->OUTCLR =  1 << PIN_ISRUART_RX;

#endif
}

void debugpins_isruartrx_set(void) {
    
#ifndef MONOLETS_TAG

    NRF_P0->OUTSET =  1 << PIN_ISRUART_RX;

#endif
}

// intdisabled
void debugpins_intdisabled_toggle(void) {
    
#ifndef MONOLETS_TAG

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_INTDISABLED)) & (NRF_P0->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0->OUTCLR =  1 << PIN_INTDISABLED;
    } else {
        // it is off, turn on led
        NRF_P0->OUTSET =  1 << PIN_INTDISABLED;
    }

#endif
}
void debugpins_intdisabled_clr(void) {

#ifndef MONOLETS_TAG

    NRF_P0->OUTCLR =  1 << PIN_INTDISABLED;

#endif
}
void debugpins_intdisabled_set(void) {
    
#ifndef MONOLETS_TAG

    NRF_P0->OUTSET =  1 << PIN_INTDISABLED;

#endif
}

//=========================== private =========================================
