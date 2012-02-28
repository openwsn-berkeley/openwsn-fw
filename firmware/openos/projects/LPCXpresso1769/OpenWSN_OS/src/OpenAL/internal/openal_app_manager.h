/*
 * openal_app_registry.h
 *
 *  Created on: Feb 27, 2012
 *      Author: nerd256
 */

#ifndef OPENAL_APP_REGISTRY_H_
#define OPENAL_APP_REGISTRY_H_

#include "openal_internal_common.h"
#include "openmote_pindefs.h"

#ifndef PINS_H_
#error "Please include pins.h before pin_registry.h"
#endif

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define APP_SLOT_EMPTY 0
#define APP_CURRENT 0xFE
typedef struct app_registry_entry_t {
	U32 appID;
	char appTag[8];
	U32 signature;
	U32 memory_base_address;
	U32 memory_length;
	U32 pin_permissions;
} app_registry_entry_t;

#define OPENAL_NUM_APPS 10
extern const app_registry_entry_t app_registry[OPENAL_NUM_APPS] __attribute__((section(".openAL_registry")));

#define NO_OWNER ((xTaskHandle)0xFFFFFFFF)
typedef struct openal_pin_state_t {
	pin_mode_t mode;
	xTaskHandle owner; // index of app in the app registry
} openal_pin_state_t ;

extern openal_pin_state_t openal_pin_registry[OPENMOTE_NUM_PINS];

// other mutexes used by openAL
extern xSemaphoreHandle pin_configuration_mutex;

/*
 * Claim pin for current task
 */
openal_error_code_t openal_claim_pin(char pin)  OPENAL_LIB;

/*
 * Check if current task is the owner of this pin
 */
openal_error_code_t openal_check_owner(char pin) OPENAL_LIB;

/*
 * Initialize openAL app manager
 */
void openal_app_manager_init() OPENAL_LIB;

#endif /* OPENAL_APP_REGISTRY_H_ */
