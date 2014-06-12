#include "compiler.h"
#include "system.h"
#include "system_interrupt.h"
#include "port.h"
#include "debugpins.h"

/* Create the Debug pins for debug the stack operation */
#define DEBUG_PIN_FRAME  PIN_PA06
#define DEBUG_PIN_SLOT   PIN_PA13
#define DEBUG_PIN_FSM    PIN_PA18
#define DEBUG_PIN_TASK   PIN_PA16
#define DEBUG_PIN_ISR    PIN_PA17
#define DEBUG_PIN_RADIO  PIN_PB03

#ifdef SAMR21
#define DBG_PIN1         PIN_PB02
#define DBG_PIN2         PIN_PB23
#endif

#define SET_HIGH         true
#define SET_LOW          false

void debugpins_init(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);

	/* Configure Debug Pins, Set it low */
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(DEBUG_PIN_FRAME, &pin_conf);
	port_pin_set_config(DEBUG_PIN_SLOT, &pin_conf);
	port_pin_set_config(DEBUG_PIN_FSM, &pin_conf);
	port_pin_set_config(DEBUG_PIN_TASK, &pin_conf);
	port_pin_set_config(DEBUG_PIN_ISR, &pin_conf);
	port_pin_set_config(DEBUG_PIN_RADIO, &pin_conf);

#ifdef SAMR21
    port_pin_set_config(DBG_PIN1, &pin_conf);
	port_pin_set_config(DBG_PIN2, &pin_conf);
	port_pin_set_output_level(DBG_PIN1, SET_LOW);
	port_pin_set_output_level(DBG_PIN2, SET_LOW);
#endif

	port_pin_set_output_level(DEBUG_PIN_FRAME, SET_LOW);
	port_pin_set_output_level(DEBUG_PIN_SLOT, SET_LOW);
	port_pin_set_output_level(DEBUG_PIN_FSM, SET_LOW);
	port_pin_set_output_level(DEBUG_PIN_TASK, SET_LOW);
	port_pin_set_output_level(DEBUG_PIN_ISR, SET_LOW);    
	port_pin_set_output_level(DEBUG_PIN_RADIO, SET_LOW);
}

void debugpins_frame_toggle(void)
{ 
 port_pin_toggle_output_level(DEBUG_PIN_FRAME);
}

void debugpins_frame_clr(void)
{
 port_pin_set_output_level(DEBUG_PIN_FRAME, SET_LOW);
}

void debugpins_frame_set(void)
{
 port_pin_set_output_level(DEBUG_PIN_FRAME, SET_HIGH);
}

void debugpins_slot_toggle(void)
{
 port_pin_toggle_output_level(DEBUG_PIN_SLOT);
}

void debugpins_slot_clr(void)
{
 port_pin_set_output_level(DEBUG_PIN_SLOT, SET_LOW);
}

void debugpins_slot_set(void)
{
 port_pin_set_output_level(DEBUG_PIN_SLOT, SET_HIGH);
}

void debugpins_fsm_toggle(void)
{
 port_pin_toggle_output_level(DEBUG_PIN_FSM);
}

void debugpins_fsm_clr(void)
{
 port_pin_set_output_level(DEBUG_PIN_FSM, SET_LOW);
}

void debugpins_fsm_set(void)
{
 port_pin_set_output_level(DEBUG_PIN_FSM, SET_HIGH);
}

void debugpins_task_toggle(void)
{
 port_pin_toggle_output_level(DEBUG_PIN_TASK);
}

void debugpins_task_clr(void)
{
 port_pin_set_output_level(DEBUG_PIN_TASK, SET_LOW);
}

void debugpins_task_set(void)
{
 port_pin_set_output_level(DEBUG_PIN_TASK, SET_HIGH);
}

void debugpins_isr_toggle(void)
{
 port_pin_toggle_output_level(DEBUG_PIN_ISR);
}

void debugpins_isr_clr(void)
{
 port_pin_set_output_level(DEBUG_PIN_ISR, SET_LOW);
}

void debugpins_isr_set(void)
{
 port_pin_set_output_level(DEBUG_PIN_ISR, SET_HIGH);
}

void debugpins_radio_toggle(void)
{
 port_pin_toggle_output_level(DEBUG_PIN_RADIO);
}

void debugpins_radio_clr(void)
{
 port_pin_set_output_level(DEBUG_PIN_RADIO, SET_LOW);
}

void debugpins_radio_set(void)
{
 port_pin_set_output_level(DEBUG_PIN_RADIO, SET_HIGH);
}


#ifdef SAMR21
void dbg_pin1_set(void)
{
 port_pin_set_output_level(DBG_PIN1, SET_HIGH);
}

void dbg_pin1_clr(void)
{
	port_pin_set_output_level(DBG_PIN1, SET_LOW);
}

void dbg_pin1_toggle(void)
{
	port_pin_toggle_output_level(DBG_PIN1);
}

void dbg_pin2_set(void)
{
	port_pin_set_output_level(DBG_PIN2, SET_HIGH);
}

void dbg_pin2_clr(void)
{
	port_pin_set_output_level(DBG_PIN2, SET_LOW);
}

void dbg_pin2_toggle(void)
{
	port_pin_toggle_output_level(DBG_PIN2);
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

