/*
 * openal_app_registry.c
 *
 *  Created on: Feb 27, 2012
 *      Author: nerd256
 */

#include "pins.h"
#include "openmote_pindefs.h"
#include "openal_app_manager.h"
#include "openal_event_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Parameters for starting a task
#define FREERTOS_APP_STACK_SIZE (0x1000/sizeof(size_t)) // 4KB of memory
#define FREERTOS_APP_PRIORITY tskIDLE_PRIORITY

const app_registry_entry_t app_registry[OPENAL_NUM_APPS] ;

xSemaphoreHandle pin_registry_mutex;
openal_pin_state_t openal_pin_registry[OPENMOTE_NUM_PINS];

xSemaphoreHandle app_data_mutex;
volatile_app_data_t openal_app_data[OPENAL_NUM_APPS];

// Some other semaphores used by OpenAL
xSemaphoreHandle pin_configuration_mutex;
xSemaphoreHandle adc_mutex;



extern void * _API_end_of_text;

char openal_current_app() {
	xTaskHandle curTask = xTaskGetCurrentTaskHandle();
	char c;
	for ( c = 0; c < OPENAL_NUM_APPS; c++ ) {
		if (curTask == openal_app_data[c].handle)
			return c + 1; // 1 indexed
	}
	return APP_NOT_FOUND;
}

openal_error_code_t openal_claim_pin(char pin) {
	xTaskHandle curTask;
	openal_pin_state_t * pin_state;
	char c;
	openal_error_code_t error;
	xSemaphoreTake(pin_registry_mutex, portMAX_DELAY); // Going to be modifying the pin registry
	xSemaphoreTake(app_data_mutex, portMAX_DELAY);

	// the pin parameter is 1-indexed
	pin--;

	// Not a proper pin
	if ( pin < 0 || pin >= OPENMOTE_NUM_PINS || openmote_pin_gpio_bank[pin] == NOT_VALID_PIN) {
		error = INVALID_PIN;
		goto fail;
	}

	curTask = xTaskGetCurrentTaskHandle();

	pin_state = &openal_pin_registry[pin];

	// Already claimed by this task
	if ( pin_state->owner == curTask ) {
		goto success;
	}

	for ( c = 0; c < OPENAL_NUM_APPS; c++ ) {
		if (openal_app_data[c].handle == curTask ) {
			goto taskfound;
		} else if ( openal_app_data[c].handle == pin_state->owner ) { // Already claimed by a higher-priority task
			error = PIN_ACCESS_DENIED;
			goto fail;
		}
	}
	error = UNKNOWN_CALLER;
	goto fail;

	taskfound:
		if ( app_registry[c].pin_permissions & (1<<pin) ) {
			pin_state->owner = curTask;
			goto success;
		} else {
			error = PIN_ACCESS_DENIED;
			goto fail;
		}

	success:
		xSemaphoreGive((app_data_mutex));
		xSemaphoreGive(pin_registry_mutex);
		return SUCCESS;

	fail:
		xSemaphoreGive((app_data_mutex));
		xSemaphoreGive(pin_registry_mutex);
		return error;
}

openal_error_code_t openal_check_owner(char pin) {
	openal_error_code_t error;
	xTaskHandle curTask = xTaskGetCurrentTaskHandle();

	// the pin parameter is 1-indexed
	pin--;

	xSemaphoreTake(pin_registry_mutex, portMAX_DELAY);
	if ( pin < 0 || pin >= OPENMOTE_NUM_PINS ) {
		error = INVALID_PIN;
	} else if ( openal_pin_registry[pin].owner == curTask )
		error = SUCCESS;
	else
		error = PIN_NOT_OWNER;
	xSemaphoreGive(pin_registry_mutex);
	return error;
}

void openal_start_app(char app_no) {
	portBASE_TYPE retcode;
	xTaskParameters new_task_params;

	if ( !prvIsPrivileged() ) { // troublemaker!
		openal_kill_app(APP_CURRENT);
		return;
	}

	xSemaphoreTake(app_data_mutex, portMAX_DELAY);

	app_no--;

	if ( app_no < 0 || app_no >= OPENAL_NUM_APPS ||
			app_registry[app_no].appID == APP_SLOT_EMPTY ||
			openal_app_data[app_no].state == RUNNING ) { // invalid app to start
		goto error;
	}


	new_task_params.pvTaskCode = NULL;
	new_task_params.usStackDepth = FREERTOS_APP_STACK_SIZE;
	new_task_params.pvParameters = NULL;
	new_task_params.uxPriority = FREERTOS_APP_PRIORITY;
	new_task_params.puxStackBuffer;
	new_task_params.xRegions;


	retcode = xTaskCreateRestricted(&new_task_params,&(openal_app_data[app_no].handle));
	if ( retcode != pdPASS ) {
		openal_app_data[app_no].handle = NULL;
		goto error;
	}

	openal_event_queues[app_no] = xQueueCreate(OPENAL_EVENT_QUEUE_LEN, sizeof(event_response_t));

	error:
		xSemaphoreGive(app_data_mutex);
		return;
}

void openal_kill_app(char app_no) {
	char c ;
	xTaskHandle curTask = xTaskGetCurrentTaskHandle();

	xSemaphoreTake(app_data_mutex, portMAX_DELAY);

	volatile_app_data_t * curApp;
	if ( app_no == APP_CURRENT ) {

		c = openal_current_app();
		if ( c == APP_NOT_FOUND ) {
			goto error;
		}

		c--; // app numbers are 1-indexed
		curApp = &openal_app_data[c];
		app_no = c ;
	} else {
		app_no--; // app numbers are 1-indexed

		if ( app_no < 0 || app_no >= OPENAL_NUM_APPS ) // invalid
			goto error;

		curApp = &openal_app_data[app_no];

		if ( curApp->handle != curTask && !prvIsPrivileged()) {
			// You are only allowed to kill yourself if not privileged
			goto error;
		}
	}

	curTask = curApp->handle;
	curApp->handle = NULL;
	curApp->state = STOPPED;
	xSemaphoreGive(app_data_mutex);

	// release pins from use
	xSemaphoreTake(pin_registry_mutex, portMAX_DELAY);
	for (c = 0; c < OPENMOTE_NUM_PINS; c++) {
		if ( openal_pin_registry[c].owner == curTask ) {
			openal_pin_registry[c].owner = NO_OWNER;
		}
	}
	xSemaphoreGive(pin_registry_mutex);
	vQueueDelete(openal_event_queues[app_no]);
	vTaskDelete(curTask);

	// should not reach here if this was called on current task
	return;

	error:
		xSemaphoreGive(app_data_mutex);
		return;

}

void openal_app_manager_init() {
	int c;
	for ( c = 0; c < OPENMOTE_NUM_PINS; c++) {
		openal_pin_registry[c].owner = NO_OWNER;
	}

	app_data_mutex = xSemaphoreCreateMutex();
	pin_registry_mutex = xSemaphoreCreateMutex();
	pin_configuration_mutex = xSemaphoreCreateMutex();
	adc_mutex = xSemaphoreCreateMutex();
}


