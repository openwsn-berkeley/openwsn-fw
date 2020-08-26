/**
\brief nRF5340_network-specific definition of the "debugpins" bsp module.

\author: Tengfei Chang <tengfei.chang@inria.fr> August 2020
*/

#include "nRF5340_network.h"
#include "nrf5340_network_bitfields.h"
#include "debugpins.h"

//=========================== defines =========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define PIN_FRAME         NRF_GPIO_PIN_MAP(0,04)    // p0.04
#define PIN_SLOT          NRF_GPIO_PIN_MAP(0,05)    // p0.05
#define PIN_FSM           NRF_GPIO_PIN_MAP(0,06)    // p0.06
#define PIN_TASK          NRF_GPIO_PIN_MAP(0,07)    // p0.07
#define PIN_ISR           NRF_GPIO_PIN_MAP(0,23)    // p0.23
#define PIN_RADIO         NRF_GPIO_PIN_MAP(0,24)    // p0.24
#define PIN_ISRUART_TX    NRF_GPIO_PIN_MAP(0,25)    // p0.25
#define PIN_ISRUART_RX    NRF_GPIO_PIN_MAP(0,26)    // p0.26
#define PIN_INTDISABLED   NRF_GPIO_PIN_MAP(0,27)    // p0.27

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

// init
void debugpins_init(void) {

    nrf_gpio_cfg_output(PIN_FRAME);
    nrf_gpio_cfg_output(PIN_SLOT);
    nrf_gpio_cfg_output(PIN_FSM);
    nrf_gpio_cfg_output(PIN_TASK);
    nrf_gpio_cfg_output(PIN_ISR);
    nrf_gpio_cfg_output(PIN_RADIO);
    nrf_gpio_cfg_output(PIN_ISRUART_TX);
    nrf_gpio_cfg_output(PIN_ISRUART_RX);
    nrf_gpio_cfg_output(PIN_INTDISABLED);

}

// frame
void debugpins_frame_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_FRAME)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_FRAME;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_FRAME;
    }
}

void debugpins_frame_clr(void) {

    NRF_P0_NS->OUTCLR =  1 << PIN_FRAME;
}

void debugpins_frame_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_FRAME;
}

// slot
void debugpins_slot_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_SLOT)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_SLOT;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_SLOT;
    }
}

void debugpins_slot_clr(void) {
    
    NRF_P0_NS->OUTCLR =  1 << PIN_SLOT;
}

void debugpins_slot_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_SLOT;
}

// fsm
void debugpins_fsm_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_FSM)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_FSM;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_FSM;
    }
}

void debugpins_fsm_clr(void) {
    
     NRF_P0_NS->OUTCLR =  1 << PIN_FSM;
}

void debugpins_fsm_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_FSM;
}

// task
void debugpins_task_toggle(void) {
    
        
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_TASK)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_TASK;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_TASK;
    }
}

void debugpins_task_clr(void) {

    NRF_P0_NS->OUTCLR =  1 << PIN_TASK;
}

void debugpins_task_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_TASK;
}

// isr
void debugpins_isr_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISR)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_ISR;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_ISR;
    }
}

void debugpins_isr_clr(void) {
    
    NRF_P0_NS->OUTCLR=  1 << PIN_ISR;
}

void debugpins_isr_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_ISR;
}

// radio
void debugpins_radio_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_RADIO)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_RADIO;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_RADIO;
    }
}

void debugpins_radio_clr(void) {
    
    NRF_P0_NS->OUTCLR=  1 << PIN_RADIO;
}

void debugpins_radio_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_RADIO;
}

// isruarttx
void debugpins_isruarttx_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISRUART_TX)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_ISRUART_TX;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_ISRUART_TX;
    }
}

void debugpins_isruarttx_clr(void) {
    
    NRF_P0_NS->OUTCLR =  1 << PIN_ISRUART_TX;
}

void debugpins_isruarttx_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_ISRUART_TX;
}

// isruartrx
void debugpins_isruartrx_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_ISRUART_RX)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_ISRUART_RX;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_ISRUART_RX;
    }
}

void debugpins_isruartrx_clr(void) {
    
    NRF_P0_NS->OUTCLR =  1 << PIN_ISRUART_RX;
}

void debugpins_isruartrx_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_ISRUART_RX;
}

// intdisabled
void debugpins_intdisabled_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << PIN_INTDISABLED)) & (NRF_P0_NS->OUT);

    if (output_status>0){
        // it is on , turn off led
        NRF_P0_NS->OUTCLR =  1 << PIN_INTDISABLED;
    } else {
        // it is off, turn on led
        NRF_P0_NS->OUTSET =  1 << PIN_INTDISABLED;
    }
}
void debugpins_intdisabled_clr(void) {

    NRF_P0_NS->OUTCLR =  1 << PIN_INTDISABLED;
}
void debugpins_intdisabled_set(void) {
    
    NRF_P0_NS->OUTSET =  1 << PIN_INTDISABLED;
}

//=========================== private =========================================
