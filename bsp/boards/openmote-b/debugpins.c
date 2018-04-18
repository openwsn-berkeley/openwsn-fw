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
#define BSP_PINB_BASE           GPIO_B_BASE
#define BSP_PINC_BASE           GPIO_C_BASE

#define BSP_PINA_7              GPIO_PIN_7      // PA7 -- frame
#define BSP_PINC_3              GPIO_PIN_3      // PC3 -- slot

#define BSP_PINB_3              GPIO_PIN_3      // PB3 -- fsm
#define BSP_PINB_2              GPIO_PIN_2      // PB2 -- task
#define BSP_PINB_1              GPIO_PIN_1      // PB1 -- isr
#define BSP_PINB_0              GPIO_PIN_0      // PB0 -- radio

//=========================== variables =======================================

//=========================== prototypes ======================================

void bspDBpinToggle(uint32_t base,uint8_t ui8Pin);

//=========================== public ==========================================

void debugpins_init(void) {
    GPIOPinTypeGPIOOutput(BSP_PINA_BASE, BSP_PINA_7);
    GPIOPinTypeGPIOOutput(BSP_PINC_BASE, BSP_PINC_3);
    GPIOPinTypeGPIOOutput(BSP_PINB_BASE, BSP_PINB_3 | BSP_PINB_2 | BSP_PINB_1 | BSP_PINB_0);

    GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_7, 0x00);
    GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_3, 0x00);
    GPIOPinWrite(BSP_PINB_BASE, (BSP_PINB_3 | BSP_PINB_2 | BSP_PINB_1 | BSP_PINB_0), 0x00);
}

// PA7
void debugpins_frame_toggle(void) {
    bspDBpinToggle(BSP_PINA_BASE, BSP_PINA_7);
}
void debugpins_frame_clr(void) {
    GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_7, 0);
}
void debugpins_frame_set(void) {
    GPIOPinWrite(BSP_PINA_BASE, BSP_PINA_7, BSP_PINA_7);
}

// PC3
void debugpins_slot_toggle(void) {
    bspDBpinToggle(BSP_PINC_BASE, BSP_PINC_3);
}
void debugpins_slot_clr(void) {
    GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_3, 0);
}
void debugpins_slot_set(void) {
    GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_3, BSP_PINC_3);
}

// PB3
void debugpins_fsm_toggle(void) {
    bspDBpinToggle(BSP_PINB_BASE, BSP_PINB_3);
}
void debugpins_fsm_clr(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_3, 0);
}
void debugpins_fsm_set(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_3, BSP_PINB_3);
}

// PB2
void debugpins_task_toggle(void) {
    bspDBpinToggle(BSP_PINB_BASE,BSP_PINB_2);
}
void debugpins_task_clr(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_2, 0);
}
void debugpins_task_set(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_2, BSP_PINB_2);
}

// isruarttx: not provided by openmote-b
void debugpins_isruarttx_toggle(void) {
}
void debugpins_isruarttx_clr(void) {
}
void debugpins_isruarttx_set(void) {
}

// isruartrx: not provided by openmote-b
void debugpins_isruartrx_toggle(void) {
}
void debugpins_isruartrx_clr(void) {
}
void debugpins_isruartrx_set(void) {
}

// PB1
void debugpins_isr_toggle(void) {
    bspDBpinToggle(BSP_PINB_BASE, BSP_PINB_1);
}
void debugpins_isr_clr(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_1, 0);
}
void debugpins_isr_set(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_1, BSP_PINB_1);
}

// PB0
void debugpins_radio_toggle(void) {
    bspDBpinToggle(BSP_PINB_BASE, BSP_PINB_0);
}
void debugpins_radio_clr(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_0, 0);
}
void debugpins_radio_set(void) {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_0, BSP_PINB_0);
}

//------------ private ------------

void bspDBpinToggle(uint32_t base, uint8_t ui8Pin){
    // Get current pin values of selected bits
    uint32_t ui32Toggle = GPIOPinRead(base, ui8Pin);

    // Invert selected bits
    ui32Toggle = (~ui32Toggle) & ui8Pin;

    // Set GPIO
    GPIOPinWrite(base, ui8Pin, ui32Toggle);
}
