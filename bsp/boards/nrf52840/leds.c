/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "leds" bsp module.
 */

#include "sdk/components/boards/boards.h"
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "stdint.h"
#include "leds.h"
#include "board.h"
#include "board_info.h"


//=========================== defines =========================================
#if BOARD_PCA10056
// nrf52840-DK
#define LED_IDX_ERROR 0
#define LED_IDX_DEBUG 1
#define LED_IDX_SYNC  2
#define LED_IDX_RADIO 3
#endif
#if BOARD_PCA10059
// nrf52840-DONGLE
#define LED_IDX_ERROR 0 // LED1
#define LED_IDX_DEBUG 2	// LED2_G
#define LED_IDX_RADIO 3 // LED2_B
#endif


//=========================== variables =======================================

//=========================== prototypes ======================================

void bspLedSet(uint8_t ui8Led);
void bspLedClear(uint8_t ui8Led);
void bspLedToggle(uint8_t ui8Led);
bool bspLedGet(uint8_t ui8Led);


//=========================== public ==========================================

void leds_init() {
#ifndef NO_LEDS
    // LEDs have probably been already initialized in board.c:board_init(), but we can do that again without problems

    const uint8_t m_board_led_list[LEDS_NUMBER] = LEDS_LIST;

    for (uint8_t l=0; l < LEDS_NUMBER; ++l) {
        nrf_gpio_cfg_output(m_board_led_list[l]);
    }

    bsp_board_leds_off();
#endif // NO_LEDS
}


void leds_error_on(void) {
    bspLedSet(LED_IDX_ERROR);
}

void leds_error_off(void) {
    bspLedClear(LED_IDX_ERROR);
}

void leds_error_toggle(void) {
    bspLedToggle(LED_IDX_ERROR);
}

uint8_t leds_error_isOn(void) {
    return((uint8_t) bspLedGet(LED_IDX_ERROR));
}


void leds_sync_on(void) {
#if BOARD_PCA10056
    bspLedSet(LED_IDX_SYNC);
#endif
}

void leds_sync_off(void) {
#if BOARD_PCA10056
    bspLedClear(LED_IDX_SYNC);
#endif
}

void leds_sync_toggle(void) {
#if BOARD_PCA10056
    bspLedToggle(LED_IDX_SYNC);
#endif
}

uint8_t leds_sync_isOn(void) {
#if BOARD_PCA10056
    return((uint8_t) bspLedGet(LED_IDX_SYNC));
#else
    return FALSE;
#endif
}


void leds_radio_on(void) {
    bspLedSet(LED_IDX_RADIO);
}

void leds_radio_off(void) {
    bspLedClear(LED_IDX_RADIO);
}

void leds_radio_toggle(void) {
    bspLedToggle(LED_IDX_RADIO);
}

uint8_t leds_radio_isOn(void) {
    return((uint8_t) bspLedGet(LED_IDX_RADIO));
}


void leds_debug_on(void) {
    bspLedSet(LED_IDX_DEBUG);
}

void leds_debug_off(void) {
    bspLedClear(LED_IDX_DEBUG);
}

void leds_debug_toggle(void) {
    bspLedToggle(LED_IDX_DEBUG);
}

uint8_t leds_debug_isOn(void) {
    return((uint8_t) bspLedGet(LED_IDX_DEBUG));
}


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

void bspLedSet(uint8_t ui8Led) {
#ifndef NO_LEDS
    bsp_board_led_on(ui8Led);
#else
    (void)ui8Led;
#endif
}

void bspLedClear(uint8_t ui8Led) {
#ifndef NO_LEDS
    bsp_board_led_off(ui8Led);
#else
    (void)ui8Led;
#endif
}

void bspLedToggle(uint8_t ui8Led) {
#ifndef NO_LEDS
    bsp_board_led_invert(ui8Led);
#else
    (void)ui8Led;
#endif
}

bool bspLedGet(uint8_t ui8Led) {
#ifndef NO_LEDS
    return bsp_board_led_state_get(ui8Led);
#else
    (void)ui8Led;
    return false;
#endif
}