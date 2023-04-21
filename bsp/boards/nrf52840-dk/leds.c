/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "leds" bsp module.
 */

 #include "stdbool.h"
#include "nrf52840.h"
#include "board_info.h"
#include "leds.h"


//=========================== defines =========================================

// nrf52840-DK
#define LED_1           NRF_GPIO_PIN_MAP(0,13)
#define LED_2           NRF_GPIO_PIN_MAP(0,14)
#define LED_3           NRF_GPIO_PIN_MAP(0,15)
#define LED_4           NRF_GPIO_PIN_MAP(0,16)

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {

    NRF_P0->DIRSET = 1<<LED_1;
    NRF_P0->DIRSET = 1<<LED_2;
    NRF_P0->DIRSET = 1<<LED_3;
    NRF_P0->DIRSET = 1<<LED_4;
}

//==== error led

void leds_error_off(void) {
    NRF_P0->OUTSET = 1<<LED_1;
}

void leds_error_on(void) {
    NRF_P0->OUTCLR = 1<<LED_1;
}

void leds_error_toggle(void) {
    if ((NRF_P0->OUT & (1<<LED_1))!=0) {        
        NRF_P0->OUTCLR = 1<<LED_1;
    } else {
        NRF_P0->OUTSET = 1<<LED_1;
    }
}

uint8_t leds_error_isOn(void) {
    if (NRF_P0->OUT & (1<<LED_1)) {
        return 0;
    } else {
        return 1;
    }
}

//==== sync led

void leds_sync_off(void) {
    NRF_P0->OUTSET = 1<<LED_2;
}

void leds_sync_on(void) {
    NRF_P0->OUTCLR = 1<<LED_2;
}

void leds_sync_toggle(void) {
    if ((NRF_P0->OUT & (1<<LED_2))!=0) {        
        NRF_P0->OUTCLR = 1<<LED_2;
    } else {
        NRF_P0->OUTSET = 1<<LED_2;
    }
}

uint8_t leds_sync_isOn(void) {
    return (uint8_t)(NRF_P0->OUT & (1<<LED_2));
}

//==== radio led


void leds_radio_off(void) {
    NRF_P0->OUTSET = 1<<LED_3;
}

void leds_radio_on(void) {
    NRF_P0->OUTCLR = 1<<LED_3;
}

void leds_radio_toggle(void) {
    if ((NRF_P0->OUT & (1<<LED_3))!=0) {        
        NRF_P0->OUTCLR = 1<<LED_3;
    } else {
        NRF_P0->OUTSET = 1<<LED_3;
    }
}

uint8_t leds_radio_isOn(void) {
    return (uint8_t)(NRF_P0->OUT & (1<<LED_3));
}

//==== debug led


void leds_debug_off(void) {
    NRF_P0->OUTSET = 1<<LED_4;
}

void leds_debug_on(void) {
    NRF_P0->OUTCLR = 1<<LED_4;
}

void leds_debug_toggle(void) {
    if ((NRF_P0->OUT & (1<<LED_4))!=0) {        
        NRF_P0->OUTCLR = 1<<LED_4;
    } else {
        NRF_P0->OUTSET = 1<<LED_4;
    }
}

uint8_t leds_debug_isOn(void) {
    return (uint8_t)(NRF_P0->OUT & (1<<LED_4));
}

//==== all leds

void leds_all_on(void) {
    leds_radio_on();
    leds_sync_on();
    leds_debug_on();
    leds_error_on();
}

void leds_all_off(void) {
    leds_radio_off();
    leds_sync_off();
    leds_debug_off();
    leds_error_off();
}

void leds_all_toggle(void) {
    leds_radio_toggle();
    leds_sync_toggle();
    leds_debug_toggle();
    leds_error_toggle();
}

void leds_error_blink(void) {
    uint8_t i;

    // turn all LEDs off
    leds_all_off();

    // blink error LED for ~10s
    for (i = 0; i < 100; i++) {
        leds_error_toggle();
        nrf_delay_ms(100);
    }
}

void leds_circular_shift(void) {
    bool led3_state= bspLedGet(3);
    (bspLedGet(2))?(bspLedSet(3)):(bspLedClear(3));
    (bspLedGet(1))?(bspLedSet(2)):(bspLedClear(2));
    (bspLedGet(0))?(bspLedSet(1)):(bspLedClear(1));
    (led3_state)?(bspLedSet(0)):(bspLedClear(0));
}

void leds_increment(void) {
    if (bspLedGet(3))     {
        leds_all_off();
    } else {
        if (bspLedGet(2)) {
            bspLedSet(3);
            return;
        }

        if (bspLedGet(1)) {
            bspLedSet(2);
            return;
        }
  
        if (bspLedGet(0)) {
            bspLedSet(1);
            return;
        }

        bspLedSet(0);
    }
}

//=========================== private =========================================