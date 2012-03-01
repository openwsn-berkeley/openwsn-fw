/*
 * events.h
 *
 *  Created on: Feb 28, 2012
 *      Author: nerd256
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#include "internal/openal_internal_common.h"

#define OPENAL_NUM_EVENTS 5
typedef enum {
	PIN_RISING,
	PIN_FALLING,
	UART_RX,
	SOCK_RX,
	UDP_RX
} event_code_t;

typedef struct {
	event_code_t type;
	U32 param1;
	U32 param2;
	U32 param3;
} event_response_t ;

event_response_t events_wait_for_event() OPENAL_LIB;


#endif /* EVENTS_H_ */
