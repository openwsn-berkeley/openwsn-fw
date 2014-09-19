/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

/* === INCLUDES ============================================================ */
#include <sam.h>
#include "samr21_gpio.h"
#include "debugpins.h"

/* === MACROS ============================================================== */
/* Create the Debug pins for debug the stack operation */
#define DEBUG_PIN_FRAME  PIN_PA06
#define DEBUG_PIN_SLOT   PIN_PA13
#define DEBUG_PIN_FSM    PIN_PA18
#define DEBUG_PIN_TASK   PIN_PA16
#define DEBUG_PIN_ISR    PIN_PA17
#define DEBUG_PIN_RADIO  PIN_PB03

#if defined (__SAMR21G18A__)
#define DBG_PIN1         PIN_PB02
#define DBG_PIN2         PIN_PB23
#endif

/*
 * @brief debugpins_init Initialize all the debug pins.
 *        debug pins are used to monitor the stack operation
 *        via DSO
 * 
 * @param None
 *
 */
void debugpins_init(void)
{
	/* Configure Debug Pins, Set it low */
	port_config(DEBUG_PIN_FRAME, PORT_PIN_DIR_OUTPUT);
	port_config(DEBUG_PIN_SLOT, PORT_PIN_DIR_OUTPUT);
	port_config(DEBUG_PIN_FSM, PORT_PIN_DIR_OUTPUT);
	port_config(DEBUG_PIN_TASK, PORT_PIN_DIR_OUTPUT);
	port_config(DEBUG_PIN_ISR, PORT_PIN_DIR_OUTPUT);
	port_config(DEBUG_PIN_RADIO, PORT_PIN_DIR_OUTPUT);

    /* Additional Debug pins used in SAMR21 */
#if defined (__SAMR21G18A__)
    port_config(DBG_PIN1, PORT_PIN_DIR_OUTPUT);
	port_config(DBG_PIN2, PORT_PIN_DIR_OUTPUT);
	port_pin_set_level(DBG_PIN1, SET_LOW);
	port_pin_set_level(DBG_PIN2, SET_LOW);
#endif

	port_pin_set_level(DEBUG_PIN_FRAME, SET_LOW);
	port_pin_set_level(DEBUG_PIN_SLOT, SET_LOW);
	port_pin_set_level(DEBUG_PIN_FSM, SET_LOW);
	port_pin_set_level(DEBUG_PIN_TASK, SET_LOW);
	port_pin_set_level(DEBUG_PIN_ISR, SET_LOW);    
	port_pin_set_level(DEBUG_PIN_RADIO, SET_LOW);
}

/*
 * @brief debugpins_frame_toggle Toggle the Debug Pin Frame. 
 *
 * @param None
 *
 */
void debugpins_frame_toggle(void)
{ 
 port_pin_toggle(DEBUG_PIN_FRAME);
}

/*
 * @brief debugpins_frame_clr Set to Low for
 *        Frame Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_frame_clr(void)
{
 port_pin_set_level(DEBUG_PIN_FRAME, SET_LOW);
}

/*
 * @brief debugpins_frame_set Set to High for
 *        Frame Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_frame_set(void)
{
 port_pin_set_level(DEBUG_PIN_FRAME, SET_HIGH);
}

/*
 * @brief debugpins_slot_toggle Toggle the
 *        Slot Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_slot_toggle(void)
{
 port_pin_toggle(DEBUG_PIN_SLOT);
}

/*
 * @brief debugpins_slot_clr Set to Low for
 *        Slot Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_slot_clr(void)
{
 port_pin_set_level(DEBUG_PIN_SLOT, SET_LOW);
}

/*
 * @brief debugpins_slot_set Set to High for
 *        Slot Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_slot_set(void)
{
 port_pin_set_level(DEBUG_PIN_SLOT, SET_HIGH);
}

/*
 * @brief debugpins_fsm_toggle Toggle the
 *        fsm Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_fsm_toggle(void)
{
 port_pin_toggle(DEBUG_PIN_FSM);
}

/*
 * @brief debugpins_fsm_clr Set to Low for
 *        fsm Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_fsm_clr(void)
{
 port_pin_set_level(DEBUG_PIN_FSM, SET_LOW);
}

/*
 * @brief debugpins_fsm_set Set to High for
 *        FSM Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_fsm_set(void)
{
 port_pin_set_level(DEBUG_PIN_FSM, SET_HIGH);
}

/*
 * @brief debugpins_task_toggle Toggle the
 *        task Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_task_toggle(void)
{
 port_pin_toggle(DEBUG_PIN_TASK);
}

/*
 * @brief debugpins_task_clr Set Low to
 *        task Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_task_clr(void)
{
 port_pin_set_level(DEBUG_PIN_TASK, SET_LOW);
}

/*
 * @brief debugpins_task_set Set High to
 *        task Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_task_set(void)
{
 port_pin_set_level(DEBUG_PIN_TASK, SET_HIGH);
}

/*
 * @brief debugpins_isr_toggle Toggle the
 *        ISR Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_isr_toggle(void)
{
 port_pin_toggle(DEBUG_PIN_ISR);
}

/*
 * @brief debugpins_isr_clr Set Low to
 *        ISR Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_isr_clr(void)
{
 port_pin_set_level(DEBUG_PIN_ISR, SET_LOW);
}

/*
 * @brief debugpins_isr_set Set High to
 *        ISR Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_isr_set(void)
{
 port_pin_set_level(DEBUG_PIN_ISR, SET_HIGH);
}

/*
 * @brief debugpins_radio_toggle Toggle the
 *        radio Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_radio_toggle(void)
{
 port_pin_toggle(DEBUG_PIN_RADIO);
}

/*
 * @brief debugpins_radio_clr Set Low to
 *        radio Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_radio_clr(void)
{
 port_pin_set_level(DEBUG_PIN_RADIO, SET_LOW);
}

/*
 * @brief debugpins_radio_set Set High to
 *        radio Debug Pin. 
 *
 * @param None
 *
 */
void debugpins_radio_set(void)
{
 port_pin_set_level(DEBUG_PIN_RADIO, SET_HIGH);
}


#if defined (__SAMR21G18A__)
/*
 * @brief dbg_pin1_set Set High to
 *        Debug Pin1. 
 *
 * @param None
 *
 */
void dbg_pin1_set(void)
{
 port_pin_set_level(DBG_PIN1, SET_HIGH);
}

/*
 * @brief dbg_pin1_clr Set Low to
 *        Debug Pin1. 
 *
 * @param None
 *
 */
void dbg_pin1_clr(void)
{
	port_pin_set_level(DBG_PIN1, SET_LOW);
}

/*
 * @brief dbg_pin1_toggle Toggle the
 *        Debug Pin1. 
 *
 * @param None
 *
 */
void dbg_pin1_toggle(void)
{
	port_pin_toggle(DBG_PIN1);
}

/*
 * @brief dbg_pin2_set Set High to
 *        Debug Pin2. 
 *
 * @param None
 *
 */
void dbg_pin2_set(void)
{
	port_pin_set_level(DBG_PIN2, SET_HIGH);
}

/*
 * @brief dbg_pin2_clr Set Low to
 *        Debug Pin2. 
 *
 * @param None
 *
 */
void dbg_pin2_clr(void)
{
	port_pin_set_level(DBG_PIN2, SET_LOW);
}

/*
 * @brief dbg_pin2_toggle Toggle the
 *        Debug Pin2. 
 *
 * @param None
 *
 */
void dbg_pin2_toggle(void)
{
	port_pin_toggle(DBG_PIN2);
}
#endif

#ifdef OPENSIM
void debugpins_ka_clr(void)
{

}

void debugpins_ka_set(void)
{

}
void debugpins_syncPacket_clr(void)
{

}

void debugpins_syncPacket_set(void)
{

}

void debugpins_syncAck_clr(void)
{

}

void debugpins_syncAck_set(void)
{

}

void debugpins_debug_clr(void)
{

}

void debugpins_debug_set(void)
{

}

#endif

