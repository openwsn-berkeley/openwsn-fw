/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific definition of the "leds" bsp module.
 */

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"
#include "sdk/components/libraries/delay/nrf_delay.h"

#include "stdint.h"
#include "leds.h"
#include "board.h"
#include "board_info.h"


//=========================== defines =========================================
#define LED_IDX_ERROR 0
#define LED_IDX_DEBUG 1
#define LED_IDX_SYNC  2
#define LED_IDX_RADIO 3

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init()
{
  // LEDs have probably been already initialized in board.c:board_init(), but we can do that again without problems

  const uint8_t m_board_led_list[LEDS_NUMBER] = LEDS_LIST;

  for (uint8_t l=0; l < LEDS_NUMBER; ++l)
  {
    nrf_gpio_cfg_output(m_board_led_list[l]);
  }

  bsp_board_leds_off();  
}


void leds_error_on(void)
{
	bsp_board_led_on(LED_IDX_ERROR);
}

void leds_error_off(void)
{
	bsp_board_led_off(LED_IDX_ERROR);
}

void leds_error_toggle(void)
{
  bsp_board_led_invert(LED_IDX_ERROR);
}

uint8_t leds_error_isOn(void)
{
  return((uint8_t) bsp_board_led_state_get(LED_IDX_ERROR));
}


void leds_sync_on(void)
{
	bsp_board_led_on(LED_IDX_SYNC);
}

void leds_sync_off(void)
{
	bsp_board_led_off(LED_IDX_SYNC);
}

void leds_sync_toggle(void)
{
  bsp_board_led_invert(LED_IDX_SYNC);
}

uint8_t leds_sync_isOn(void)
{
  return((uint8_t) bsp_board_led_state_get(LED_IDX_SYNC));
}


void leds_radio_on(void)
{
	bsp_board_led_on(LED_IDX_RADIO);
}

void leds_radio_off(void)
{
	bsp_board_led_off(LED_IDX_RADIO);
}

void leds_radio_toggle(void)
{
  bsp_board_led_invert(LED_IDX_RADIO);
}

uint8_t leds_radio_isOn(void)
{
  return((uint8_t) bsp_board_led_state_get(LED_IDX_RADIO));
}


void leds_debug_on(void)
{
	bsp_board_led_on(LED_IDX_DEBUG);
}

void leds_debug_off(void)
{
	bsp_board_led_off(LED_IDX_DEBUG);
}

void leds_debug_toggle(void)
{
  bsp_board_led_invert(LED_IDX_DEBUG);
}

uint8_t leds_debug_isOn(void)
{
  return((uint8_t) bsp_board_led_state_get(LED_IDX_DEBUG));
}


void leds_all_on(void)
{
  leds_radio_on();
  leds_sync_on();
  leds_debug_on();
  leds_error_on();
}

void leds_all_off(void)
{
  leds_radio_off();
  leds_sync_off();
  leds_debug_off();
  leds_error_off();
}

void leds_all_toggle(void)
{
  leds_radio_toggle();
  leds_sync_toggle();
  leds_debug_toggle();
  leds_error_toggle();
}

void leds_error_blink(void)
{
  uint8_t i;

  // turn all LEDs off
  leds_all_off();

  // blink error LED for ~10s
  for (i = 0; i < 100; i++)
  {
    leds_error_toggle();
    nrf_delay_ms(100);
  }
}

void leds_circular_shift(void)
{
  bool led3_state= bsp_board_led_state_get(3);
  (bsp_board_led_state_get(2))?(bsp_board_led_on(3)):(bsp_board_led_off(3));
  (bsp_board_led_state_get(1))?(bsp_board_led_on(2)):(bsp_board_led_off(2));
  (bsp_board_led_state_get(0))?(bsp_board_led_on(1)):(bsp_board_led_off(1));
  (led3_state)?(bsp_board_led_on(0)):(bsp_board_led_off(0));
}

void leds_increment(void)
{
  if (bsp_board_led_state_get(3))
  {
    leds_all_off();
  }
  else
  {
    if (bsp_board_led_state_get(2))
    {
      bsp_board_led_on(3);
      return;
    }

    if (bsp_board_led_state_get(1))
    {
      bsp_board_led_on(2);
      return;
    }
    
    if (bsp_board_led_state_get(0))
    {
      bsp_board_led_on(1);
      return;
    }

    bsp_board_led_on(0);
  }
}

//=========================== private =========================================

