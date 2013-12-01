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
#define BSP_PINC_BASE           GPIO_C_BASE


#define BSP_PINB_1              GPIO_PIN_1      //!< PB1 -- frame -RF1.5
#define BSP_PINB_2              GPIO_PIN_2      //!< PB2 -- isr   -RF1.11

#define BSP_PINC_4              GPIO_PIN_4      //!< PC4 -- slot  -RF1.6
#define BSP_PINC_5              GPIO_PIN_5      //!< PC5 -- fsm   -RF1.8
#define BSP_PINC_6              GPIO_PIN_6      //!< PC6 -- task  -RF1.10
#define BSP_PINC_7              GPIO_PIN_7      //!< PC7 -- radio -RF1-12


//=========================== variables =======================================

//=========================== prototypes ======================================
port_INLINE void bspDBpinToggle(uint32_t base,uint8_t ui8Pin);

//=========================== public ==========================================

void debugpins_init() {


   GPIOPinTypeGPIOOutput(BSP_PINB_BASE, BSP_PINB_1|BSP_PINB_2);
   GPIOPinTypeGPIOOutput(BSP_PINC_BASE, BSP_PINC_4|BSP_PINC_5|BSP_PINC_6|BSP_PINC_7);
   
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

// PC4
void debugpins_slot_toggle() {
	//bspDBpinToggle(BSP_PINC_BASE,BSP_PINC_4);
}
void debugpins_slot_clr() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_4, 0);
}
void debugpins_slot_set() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_4, BSP_PINC_4);
}

// PC5
void debugpins_fsm_toggle() {
	//bspDBpinToggle(BSP_PINC_BASE,BSP_PINC_5);
}
void debugpins_fsm_clr() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_5, 0);
}
void debugpins_fsm_set() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_5, BSP_PINC_5);
}

// PC6
void debugpins_task_toggle() {
	//bspDBpinToggle(BSP_PINC_BASE,BSP_PINC_6);
}
void debugpins_task_clr() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_6, 0);
}
void debugpins_task_set() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_6, BSP_PINC_6);
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

// PC7
void debugpins_radio_toggle() {
	//bspDBpinToggle(BSP_PINC_BASE,BSP_PINC_7);
}
void debugpins_radio_clr() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_7, 0);
}
void debugpins_radio_set() {
	//GPIOPinWrite(BSP_PINC_BASE, BSP_PINC_7, BSP_PINC_7);
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


