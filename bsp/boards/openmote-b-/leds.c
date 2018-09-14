/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "leds" bsp module.
 */

#include <stdint.h>

#include <headers/hw_memmap.h>
#include <headers/hw_types.h>

#include <source/gpio.h>

#include "board.h"
#include "leds.h"

//=========================== defines =========================================

// Board LED defines
#define BSP_LED_BASE            GPIO_C_BASE
#define BSP_LED_1               GPIO_PIN_4      //!< PC4 -- red
#define BSP_LED_2               GPIO_PIN_5      //!< PC5 -- orange
#define BSP_LED_3               GPIO_PIN_6      //!< PC6 -- yellow
#define BSP_LED_4               GPIO_PIN_7      //!< PC7 -- green

#define BSP_LED_ALL             (BSP_LED_1 | \
                                 BSP_LED_2 | \
                                 BSP_LED_3 | \
                                 BSP_LED_4)     //!< Bitmask of all LEDs

//=========================== variables =======================================

//=========================== prototypes ======================================

void bspLedSet(uint8_t ui8Leds);
void bspLedClear(uint8_t ui8Leds);
void bspLedToggle(uint8_t ui8Leds);

//=========================== public ==========================================

void leds_init(void) {
    GPIOPinTypeGPIOOutput(BSP_LED_BASE, BSP_LED_ALL);
    GPIOPinWrite(BSP_LED_BASE, BSP_LED_ALL, BSP_LED_ALL);
}

// red
void    leds_error_on(void) {
    bspLedSet(BSP_LED_1);
}
void    leds_error_off(void) {
    bspLedClear(BSP_LED_1);
}
void    leds_error_toggle(void) {
    bspLedToggle(BSP_LED_1);
}
uint8_t leds_error_isOn(void) {
    uint32_t ui32Toggle = GPIOPinRead(BSP_LED_BASE, BSP_LED_1);
    return (uint8_t)(ui32Toggle & BSP_LED_1)>>4;
}

// green
void    leds_sync_on(void) {
    bspLedSet(BSP_LED_4);
}
void    leds_sync_off(void) {
    bspLedClear(BSP_LED_4);
}
void    leds_sync_toggle(void) {
    bspLedToggle(BSP_LED_4);
}
uint8_t leds_sync_isOn(void) {
    uint32_t ui32Toggle = GPIOPinRead(BSP_LED_BASE, BSP_LED_4);
    return (uint8_t)(ui32Toggle & BSP_LED_4)>>5;
}

// orange
void    leds_radio_on(void) {
    bspLedSet(BSP_LED_2);
}
void    leds_radio_off(void) {
    bspLedClear(BSP_LED_2);
}
void    leds_radio_toggle(void) {
    bspLedToggle(BSP_LED_2);
}
uint8_t leds_radio_isOn(void) {
    uint32_t ui32Toggle = GPIOPinRead(BSP_LED_BASE, BSP_LED_2);
    return (uint8_t)(ui32Toggle & BSP_LED_2)>>7;
}

// yellow
void    leds_debug_on(void) {
    bspLedSet(BSP_LED_3);
}
void    leds_debug_off(void) {
    bspLedClear(BSP_LED_3);
}
void    leds_debug_toggle(void) {
    bspLedToggle(BSP_LED_3);
}
uint8_t leds_debug_isOn(void) {
    uint32_t ui32Toggle = GPIOPinRead(BSP_LED_BASE, BSP_LED_3);
    return (uint8_t)(ui32Toggle & BSP_LED_3)>>6;
}

// all
void leds_all_on(void) {
    bspLedSet(BSP_LED_ALL);
}
void leds_all_off(void) {
    bspLedClear(BSP_LED_ALL);
}
void leds_all_toggle(void) {
    bspLedToggle(BSP_LED_ALL);
}

void leds_error_blink(void) {
    uint8_t i;
    volatile uint16_t delay;

    // turn all LEDs off
    bspLedClear(BSP_LED_ALL);

    // blink error LED for ~10s
    for (i=0;i<80;i++) {
        bspLedToggle(BSP_LED_1);
        for (delay=0xffff;delay>0;delay--);
        for (delay=0xffff;delay>0;delay--);
    }
}

void leds_circular_shift(void) {
    uint8_t i;
    volatile uint16_t delay;

    // turn all LEDs off
    bspLedClear(BSP_LED_ALL);

    // incrementally turn LED on
    for (i=0;i<10;i++) {
        bspLedSet(BSP_LED_1);
        for (delay=0xffff;delay>0;delay--)
        bspLedClear(BSP_LED_1);
        bspLedSet(BSP_LED_2);
        for (delay=0xffff;delay>0;delay--);
        bspLedClear(BSP_LED_2);
        bspLedSet(BSP_LED_3);
        for (delay=0xffff;delay>0;delay--);
        bspLedClear(BSP_LED_3);
        bspLedSet(BSP_LED_4);
        for (delay=0xffff;delay>0;delay--);
        bspLedClear(BSP_LED_ALL);
   }
}

void leds_increment(void) {
    uint8_t i;
    volatile uint16_t delay;

    // turn all LEDs off
    bspLedClear(BSP_LED_ALL);

    // incrementally turn LED on
    for (i=0;i<10;i++) {
        bspLedSet(BSP_LED_1);
        for (delay=0xffff;delay>0;delay--);
        bspLedSet(BSP_LED_2);
        for (delay=0xffff;delay>0;delay--);
        bspLedSet(BSP_LED_3);
        for (delay=0xffff;delay>0;delay--);
        bspLedSet(BSP_LED_4);
        for (delay=0xffff;delay>0;delay--);
        bspLedClear(BSP_LED_ALL);
    }
}

//=========================== private =========================================

port_INLINE void bspLedSet(uint8_t ui8Leds){
    //
    // Turn on specified LEDs
    //
    GPIOPinWrite(BSP_LED_BASE, ui8Leds, 0);
}

port_INLINE void bspLedClear(uint8_t ui8Leds){
    //
    // Turn off specified LEDs
    //
    GPIOPinWrite(BSP_LED_BASE, ui8Leds, ui8Leds);
}

port_INLINE void bspLedToggle(uint8_t ui8Leds){
    //
    // Get current pin values of selected bits
    //
    uint32_t ui32Toggle = GPIOPinRead(BSP_LED_BASE, ui8Leds);

    //
    // Invert selected bits
    //
    ui32Toggle = (~ui32Toggle) & ui8Leds;

    //
    // Set GPIO
    //
    GPIOPinWrite(BSP_LED_BASE, ui8Leds, ui32Toggle);
}