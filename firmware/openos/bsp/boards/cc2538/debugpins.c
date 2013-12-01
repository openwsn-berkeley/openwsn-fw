/**
\brief cc2538-specific definition of the "debugpins" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, August 2013.
*/


#include "debugpins.h"
#include "gpio.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "board_info.h"

//=========================== defines =========================================
// Board dbPINS defines
#define BSP_PINB_BASE           GPIO_B_BASE
#define BSP_PIND_BASE           GPIO_C_BASE


#define BSP_PINB_1              GPIO_PIN_1      //!< PB1 -- frame -RF1.5
#define BSP_PINB_2              GPIO_PIN_2      //!< PB2 -- isr   -RF1.11

#define BSP_PIND_3              GPIO_PIN_3      //!< PD3 -- slot  -RF1.6
#define BSP_PIND_2              GPIO_PIN_2      //!< PD2 -- fsm   -RF1.8
#define BSP_PIND_1              GPIO_PIN_1      //!< PD1 -- task  -RF1.10
#define BSP_PIND_0              GPIO_PIN_0      //!< PD0 -- radio -RF1-12


//=========================== variables =======================================

//=========================== prototypes ======================================
port_INLINE void bspDBpinToggle(uint32_t base,uint8_t ui8Pin);

//=========================== public ==========================================

void debugpins_init() {


   GPIOPinTypeGPIOOutput(BSP_PINB_BASE, BSP_PINB_1|BSP_PINB_2);
   GPIOPinTypeGPIOOutput(BSP_PIND_BASE, BSP_PIND_3|BSP_PIND_2|BSP_PIND_1|BSP_PIND_0);
   
   debugpins_frame_clr();
   debugpins_slot_clr();
   debugpins_fsm_clr();
   debugpins_task_clr();
   debugpins_isr_clr();
   debugpins_radio_clr();
}

// PB1
void debugpins_frame_toggle() {
	bspDBpinToggle(BSP_PINB_BASE,BSP_PINB_1);
}
void debugpins_frame_clr() {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_1, 0);
}

void debugpins_frame_set() {
    GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_1, BSP_PINB_1);
}

// PD3
void debugpins_slot_toggle() {
	bspDBpinToggle(BSP_PIND_BASE,BSP_PIND_3);
}
void debugpins_slot_clr() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_3, 0);
}
void debugpins_slot_set() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_3, BSP_PIND_3);
}

// PD2
void debugpins_fsm_toggle() {
	bspDBpinToggle(BSP_PIND_BASE,BSP_PIND_2);
}
void debugpins_fsm_clr() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_2, 0);
}
void debugpins_fsm_set() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_2, BSP_PIND_2);
}

// PD1
void debugpins_task_toggle() {
	bspDBpinToggle(BSP_PIND_BASE,BSP_PIND_1);
}
void debugpins_task_clr() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_1, 0);
}
void debugpins_task_set() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_1, BSP_PIND_1);
}

// PB2
void debugpins_isr_toggle() {
	bspDBpinToggle(BSP_PINB_BASE,BSP_PINB_2);
}
void debugpins_isr_clr() {
	GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_2, 0);
}
void debugpins_isr_set() {
	GPIOPinWrite(BSP_PINB_BASE, BSP_PINB_2, BSP_PINB_2);
}

// PD0
void debugpins_radio_toggle() {
	bspDBpinToggle(BSP_PIND_BASE,BSP_PIND_0);
}
void debugpins_radio_clr() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_0, 0);
}
void debugpins_radio_set() {
	GPIOPinWrite(BSP_PIND_BASE, BSP_PIND_0, BSP_PIND_0);
}



//------------ private ------------//



port_INLINE void bspDBpinToggle(uint32_t base,uint8_t ui8Pin)
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


