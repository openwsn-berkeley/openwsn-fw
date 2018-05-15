/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "debugpins" bsp module.
 */

#include <headers/hw_memmap.h>
#include <headers/hw_types.h>

#include <source/gpio.h>

#include "board.h"
#include "debugpins.h"

//=========================== defines =========================================
// Board dbPINS defines
#define BSP_PINA_BASE           GPIO_A_BASE
#define BSP_PIND_BASE           GPIO_D_BASE

#define BSP_PINA_4              GPIO_PIN_4      //!< PA4 -- frame -RF1.5
#define BSP_PINA_5              GPIO_PIN_5      //!< PA5 -- isr   -RF1.11

#define BSP_PIND_3              GPIO_PIN_3      //!< PD3 -- slot  -RF1.6
#define BSP_PIND_2              GPIO_PIN_2      //!< PD2 -- fsm   -RF1.8
#define BSP_PIND_1              GPIO_PIN_1      //!< PD1 -- task  -RF1.10
#define BSP_PINA_2              GPIO_PIN_2      //!< PA2 -- radio -RF1-12
//=========================== variables =======================================

//=========================== prototypes ======================================

void bspDBpinToggle(uint32_t base,uint8_t ui8Pin);

//=========================== public ==========================================

void debugpins_init(void) {
   GPIOPinTypeGPIOOutput(BSP_PINA_BASE, BSP_PINA_2 | BSP_PINA_4 | BSP_PINA_5);
   GPIOPinTypeGPIOOutput(BSP_PIND_BASE, BSP_PIND_3 | BSP_PIND_2 | BSP_PIND_1);

   GPIOPinWrite(BSP_PINA_BASE, (BSP_PINA_2 | BSP_PINA_4 | BSP_PINA_5), 0x00);
   GPIOPinWrite(BSP_PIND_BASE, (BSP_PIND_3 | BSP_PIND_2 | BSP_PIND_1), 0);
}

// PA4
void debugpins_frame_toggle(void) {
   bspDBpinToggle(BSP_PINA_BASE, BSP_PINA_4);
}
void debugpins_frame_clr(void) {
    GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_4, 0);
}
void debugpins_frame_set(void) {
   GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_4, BSP_PINA_4);
}

// PD3
void debugpins_slot_toggle(void) {
	bspDBpinToggle(BSP_PIND_BASE, BSP_PIND_3);
}
void debugpins_slot_clr(void) {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_3, 0);
}
void debugpins_slot_set(void) {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_3, BSP_PIND_3);
}

// PD2
void debugpins_fsm_toggle(void) {
	bspDBpinToggle(BSP_PIND_BASE, BSP_PIND_2);
}
void debugpins_fsm_clr(void) {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_2, 0);
}
void debugpins_fsm_set(void) {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_2, BSP_PIND_2);
}

// PD1
void debugpins_task_toggle(void) {
	bspDBpinToggle(BSP_PIND_BASE,BSP_PIND_1);
}
void debugpins_task_clr(void) {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_1, 0);
}
void debugpins_task_set(void) {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_1, BSP_PIND_1);
}

// PA5
void debugpins_isr_toggle(void) {
	bspDBpinToggle(BSP_PINA_BASE, BSP_PINA_5);
}
void debugpins_isr_clr(void) {
	GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_5, 0);
}
void debugpins_isr_set(void) {
	GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_5, BSP_PINA_5);
}

// PD0
void debugpins_radio_toggle(void) {
	bspDBpinToggle(BSP_PINA_BASE, BSP_PINA_2);
}
void debugpins_radio_clr(void) {
	GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_2, 0);
}
void debugpins_radio_set(void) {
	GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_2, BSP_PINA_2);
}

//------------ private ------------//

void bspDBpinToggle(uint32_t base, uint8_t ui8Pin)
{
    //
    // Get current pin values of selected bits
    //
    uint32_t ui32Toggle = GPIOPinRead(base, ui8Pin);

    //
    // Invert selected bits
    //
    ui32Toggle = (~ui32Toggle) & ui8Pin;

    //
    // Set GPIO
    //
    GPIOPinWrite(base, ui8Pin, ui32Toggle);
}
