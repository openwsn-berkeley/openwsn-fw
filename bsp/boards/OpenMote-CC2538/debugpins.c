/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "debugpins" bsp module.
 */

#include <headers/hw_memmap.h>
#include <headers/hw_types.h>
#include <source/gpio.h>

#include "debugpins.h"
#include "board.h"
#include "gpio.h"

//=========================== defines =========================================

#define BSP_PINA_PORT           GPIO_A_PORT
#define BSP_PIND_PORT           GPIO_D_PORT

#define BSP_PINA_4              GPIO_PIN_4      //!< PA4 -- frame -RF1.5
#define BSP_PINA_5              GPIO_PIN_5      //!< PA5 -- isr   -RF1.11

#define BSP_PIND_3              GPIO_PIN_3      //!< PD3 -- slot  -RF1.6
#define BSP_PIND_2              GPIO_PIN_2      //!< PD2 -- fsm   -RF1.8
#define BSP_PIND_1              GPIO_PIN_1      //!< PD1 -- task  -RF1.10
#define BSP_PIND_0              GPIO_PIN_0      //!< PD0 -- radio -RF1-12

//=========================== variables =======================================

//=========================== prototypes ======================================


//=========================== public ==========================================

void debugpins_init(void) {
  gpio_config_output(BSP_PINA_PORT, (BSP_PINA_4 | BSP_PINA_5));
  gpio_config_output(BSP_PIND_PORT, (BSP_PIND_3 | BSP_PIND_2 | BSP_PIND_1 | BSP_PIND_0));

  gpio_off(BSP_PINA_PORT, (BSP_PINA_4 | BSP_PINA_5));
  gpio_off(BSP_PIND_PORT, (BSP_PIND_3 | BSP_PIND_2 | BSP_PIND_1 | BSP_PIND_0));
}

// PA4
void debugpins_frame_toggle(void) {
  gpio_toggle(BSP_PINA_PORT, BSP_PINA_4);
}

void debugpins_frame_clr(void) {
  gpio_off(BSP_PINA_PORT, BSP_PINA_4);
}

void debugpins_frame_set(void) {
  gpio_on(BSP_PINA_PORT, BSP_PINA_4);
}

void debugpins_slot_toggle(void) {
	gpio_toggle(BSP_PIND_PORT, BSP_PIND_3);
}

void debugpins_slot_clr(void) {
	gpio_off(BSP_PIND_PORT, BSP_PIND_3);
}

void debugpins_slot_set(void) {
	gpio_on(BSP_PIND_PORT, BSP_PIND_3);
}

void debugpins_fsm_toggle(void) {
	gpio_toggle(BSP_PIND_PORT, BSP_PIND_2);
}

void debugpins_fsm_clr(void) {
	gpio_off(BSP_PIND_PORT, BSP_PIND_2);
}

void debugpins_fsm_set(void) {
	gpio_on(BSP_PIND_PORT, BSP_PIND_2);
}

void debugpins_task_toggle(void) {
	gpio_toggle(BSP_PIND_PORT,BSP_PIND_1);
}

void debugpins_task_clr(void) {
	gpio_off(BSP_PIND_PORT, BSP_PIND_1);
}

void debugpins_task_set(void) {
	gpio_on(BSP_PIND_PORT, BSP_PIND_1);
}

void debugpins_isr_toggle(void) {
	gpio_toggle(BSP_PINA_PORT, BSP_PINA_5);
}

void debugpins_isr_clr(void) {
	gpio_off(BSP_PINA_PORT, BSP_PINA_5);
}

void debugpins_isr_set(void) {
	gpio_on(BSP_PINA_PORT, BSP_PINA_5);
}

void debugpins_radio_toggle(void) {
	gpio_toggle(BSP_PIND_PORT, BSP_PIND_0);
}

void debugpins_radio_clr(void) {
	gpio_off(BSP_PIND_PORT, BSP_PIND_0);
}

void debugpins_radio_set(void) {
	gpio_on(BSP_PIND_PORT, BSP_PIND_0);
}
