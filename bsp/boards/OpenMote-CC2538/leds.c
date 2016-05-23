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

#include "leds.h"
#include "gpio.h"
#include "board.h"

// Board LED defines
#define BSP_LED_PORT            GPIO_C_PORT
#define BSP_LED_1               GPIO_PIN_4      //!< PC4 -- red
#define BSP_LED_2               GPIO_PIN_5      //!< PC5 -- orange
#define BSP_LED_3               GPIO_PIN_6      //!< PC6 -- yellow
#define BSP_LED_4               GPIO_PIN_7      //!< PC7 -- green

#define BSP_LED_ALL             (BSP_LED_1 | \
                                 BSP_LED_2 | \
                                 BSP_LED_3 | \
                                 BSP_LED_4)     //!< Bitmask of all LEDs

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init(void) {
  gpio_config_output(BSP_LED_PORT, BSP_LED_ALL);
  gpio_off(BSP_LED_PORT, BSP_LED_ALL);
}

void leds_error_on(void) {
  gpio_on(BSP_LED_PORT, BSP_LED_1);
}

void leds_error_off(void) {
  gpio_off(BSP_LED_PORT, BSP_LED_1);
}

void leds_error_toggle(void) {
  gpio_toggle(BSP_LED_PORT, BSP_LED_1);
}

uint8_t leds_error_isOn(void) {
  return gpio_read(BSP_LED_PORT, BSP_LED_1);
}

void leds_sync_on(void) {
  gpio_on(BSP_LED_PORT, BSP_LED_2);
}

void leds_sync_off(void) {
  gpio_off(BSP_LED_PORT, BSP_LED_2);
}

void leds_sync_toggle(void) {
  gpio_toggle(BSP_LED_PORT, BSP_LED_2);
}

uint8_t leds_sync_isOn(void) {
  return gpio_read(BSP_LED_PORT, BSP_LED_2);
}

void leds_radio_on(void) {
  gpio_on(BSP_LED_PORT, BSP_LED_4);
}

void leds_radio_off(void) {
  gpio_off(BSP_LED_PORT, BSP_LED_4);
}

void leds_radio_toggle(void) {
  gpio_toggle(BSP_LED_PORT, BSP_LED_4);
}

uint8_t leds_radio_isOn(void) {
  return gpio_read(BSP_LED_PORT, BSP_LED_4);
}

void leds_debug_on(void) {
  gpio_on(BSP_LED_PORT, BSP_LED_3);
}

void leds_debug_off(void) {
  gpio_off(BSP_LED_PORT, BSP_LED_3);
}

void leds_debug_toggle(void) {
  gpio_toggle(BSP_LED_PORT, BSP_LED_3);
}

uint8_t leds_debug_isOn(void) {
  return gpio_read(BSP_LED_PORT, BSP_LED_3);
}

void leds_all_on(void) {
  gpio_on(BSP_LED_PORT, BSP_LED_ALL);
}

void leds_all_off() {
  gpio_off(BSP_LED_PORT, BSP_LED_ALL);
}

void leds_all_toggle() {
  gpio_toggle(BSP_LED_PORT, BSP_LED_ALL);
}

void leds_error_blink(void) {
  uint8_t i;
  volatile uint16_t delay;
   
  // turn all LEDs off
  gpio_off(BSP_LED_PORT, BSP_LED_ALL);
     
  // blink error LED for ~10s
  for (i = 0; i < 80; i++) {
    gpio_toggle(BSP_LED_PORT, BSP_LED_1);
    for (delay = 0xFFFF; delay > 0; delay--);
    for (delay = 0xFFFF; delay > 0; delay--);
  }
}

void leds_circular_shift(void) {
}

void leds_increment(void) {
}

//=========================== private =========================================
