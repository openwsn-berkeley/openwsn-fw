/**
\brief nRF52840-specific definition of the "leds" bsp module.

\author Tengfei Chang <tengfei.chang@gmail.com>, July 2020.
*/

#include "nrf52833.h"
#include "nrf52833_bitfields.h"
#include "leds.h"

//=========================== defines =========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#ifdef MONOLETS_TAG

#define LED_ERROR          NRF_GPIO_PIN_MAP(0,26) // p0.13
#define LED_RADIO          NRF_GPIO_PIN_MAP(0,19) // p0.14
#define LED_SYNC           NRF_GPIO_PIN_MAP(0,4)  // p0.15
#define LED_DEBUG          NRF_GPIO_PIN_MAP(0,13) // p0.16

#else

#define LED_ERROR          NRF_GPIO_PIN_MAP(0,13) // p0.13
#define LED_RADIO          NRF_GPIO_PIN_MAP(0,14) // p0.14
#define LED_SYNC           NRF_GPIO_PIN_MAP(0,15) // p0.15
#define LED_DEBUG          NRF_GPIO_PIN_MAP(0,16) // p0.16

#endif

//=========================== variables =======================================

//=========================== prototypes ======================================

void nrf_gpio_cfg_output(uint32_t pin_number);

//=========================== public ==========================================

void    leds_init(void) {

    nrf_gpio_cfg_output(LED_ERROR);
    nrf_gpio_cfg_output(LED_RADIO);
    nrf_gpio_cfg_output(LED_SYNC);
    nrf_gpio_cfg_output(LED_DEBUG);

    leds_all_off();

}


// p0.13
void    leds_error_on(void) {

    NRF_P0->OUTCLR =  1 << LED_ERROR;
}


void    leds_error_off(void) {
    
     NRF_P0->OUTSET =  1 << LED_ERROR;
}


void    leds_error_toggle(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_ERROR)) & (NRF_P0->OUT);

    if (output_status==0){
        // it is on , turn off led
        NRF_P0->OUTSET =  1 << LED_ERROR;
    } else {
        // it is off, turn on led
        NRF_P0->OUTCLR =  1 << LED_ERROR;
    }
}


uint8_t leds_error_isOn(void) {

    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_ERROR)) & (NRF_P0->OUT);

    if (output_status>0){
        return (uint8_t)1;
    } else {
        return (uint8_t)0;
    }
}


void leds_error_blink(void) {

    uint8_t i;
    uint32_t delay;

    // turn all LEDs off
    leds_all_off();

    // blink error LED for ~10s
    for (i=0;i<80;i++) {
        leds_error_toggle();
        for (delay=0x7ffff;delay>0;delay--);
        for (delay=0x7ffff;delay>0;delay--);
    }
}


void    leds_radio_on(void) {

    NRF_P0->OUTCLR =  1 << LED_RADIO;
}


void    leds_radio_off(void) {
    
    NRF_P0->OUTSET =  1 << LED_RADIO;
}


void    leds_radio_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_RADIO)) & (NRF_P0->OUT);

    if (output_status==0){
        // it is on , turn off led
        NRF_P0->OUTSET =  1 << LED_RADIO;
    } else {
        // it is off, turn on led
        NRF_P0->OUTCLR =  1 << LED_RADIO;
    }
}


uint8_t leds_radio_isOn(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_RADIO)) & (NRF_P0->OUT);

    if (output_status>0){
        return (uint8_t)1;
    } else {
        return (uint8_t)0;
    }
}

void    leds_sync_on(void) {

    NRF_P0->OUTCLR =  1 << LED_SYNC;
}


void    leds_sync_off(void) {
    
    NRF_P0->OUTSET =  1 << LED_SYNC;
}


void    leds_sync_toggle(void) {
   
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_SYNC)) & (NRF_P0->OUT);

    if (output_status==0){
        // it is on , turn off led
        NRF_P0->OUTSET =  1 << LED_SYNC;
    } else {
        // it is off, turn on led
        NRF_P0->OUTCLR =  1 << LED_SYNC;
    }
}


uint8_t leds_sync_isOn(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_SYNC)) & (NRF_P0->OUT);

    if (output_status>0){
        return (uint8_t)1;
    } else {
        return (uint8_t)0;
    }
}

void    leds_debug_on(void) {

    NRF_P0->OUTCLR =  1 << LED_DEBUG;
}


void    leds_debug_off(void) {
    
    NRF_P0->OUTSET =  1 << LED_DEBUG;
}


void    leds_debug_toggle(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_DEBUG)) & (NRF_P0->OUT);

    if (output_status==0){
        // it is on , turn off led
        NRF_P0->OUTSET =  1 << LED_DEBUG;
    } else {
        // it is off, turn on led
        NRF_P0->OUTCLR =  1 << LED_DEBUG;
    }
}


uint8_t leds_debug_isOn(void) {
    
    volatile uint32_t output_status;

    output_status = ((uint32_t)(1 << LED_DEBUG)) & (NRF_P0->OUT);

    if (output_status>0){
        return (uint8_t)1;
    } else {
        return (uint8_t)0;
    }
}

void    leds_all_on(void) {

    NRF_P0->OUTCLR =  1 << LED_ERROR;
    NRF_P0->OUTCLR =  1 << LED_RADIO;
    NRF_P0->OUTCLR =  1 << LED_SYNC;
    NRF_P0->OUTCLR =  1 << LED_DEBUG;
}


void    leds_all_off(void) {

    NRF_P0->OUTSET =  1 << LED_ERROR;
    NRF_P0->OUTSET =  1 << LED_RADIO;
    NRF_P0->OUTSET =  1 << LED_SYNC;
    NRF_P0->OUTSET =  1 << LED_DEBUG;
}

void    leds_all_toggle(void) {
    
    leds_error_toggle();
    leds_radio_toggle(); 
    leds_sync_toggle();
    leds_debug_toggle();
}


void    leds_circular_shift(void) {
    
    uint8_t i;
    uint32_t delay;

    // turn all LEDs off
    leds_all_off();

    // incrementally turn LED on
    for (i=0;i<10;i++) {
        leds_error_on();
        for (delay=0x7ffff;delay>0;delay--)
        leds_error_off();
        leds_radio_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_radio_off();
        leds_sync_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_sync_off();
        leds_debug_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_debug_off();
   }
}

void    leds_increment(void) {
    
    uint8_t i;
    uint32_t delay;

    // turn all LEDs off
    leds_all_off();

    // incrementally turn LED on
    for (i=0;i<10;i++) {
        leds_error_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_radio_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_sync_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_debug_on();
        for (delay=0x7ffff;delay>0;delay--);
        leds_all_off();
    }
}

//=========================== private =========================================

void nrf_gpio_cfg_output(uint32_t pin_number){
 

    NRF_P0->PIN_CNF[pin_number] =   \
           ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
         | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
         | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
         | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
         | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

}