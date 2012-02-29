/*
 * openal_event_manager.h
 *
 *  Created on: Feb 28, 2012
 *      Author: nerd256
 */

#ifndef OPENAL_EVENT_MANAGER_H_
#define OPENAL_EVENT_MANAGER_H_

#include "openal_internal_common.h"
#include "openal_app_manager.h"
#include "openmote_pindefs.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "events.h"

#define OPENAL_EVENT_QUEUE_LEN 5

// All the Listeners we can have. Provides app_no
extern char openal_GPIO_Listeners[OPENMOTE_NUM_PINS];
extern char openal_GPIO_Listener_type[OPENMOTE_NUM_PINS]; // rising or falling

extern char openal_UARTA_Listener;
extern char openal_UARTB_Listener;
extern char openal_UARTC_Listener;
extern char openal_SOCK_Listener;
extern char openal_UDP_Listener;

extern xQueueHandle openal_event_queues[OPENAL_NUM_APPS];

extern xSemaphoreHandle event_listener_config_mutex;
extern xSemaphoreHandle adc_conversion_semaphore;

void openal_event_manager_init() OPENAL_LIB;


#endif /* OPENAL_EVENT_MANAGER_H_ */
