/*
 * openal_internal_common.h
 *
 *  Created on: Feb 26, 2012
 *      Author: nerd256
 */

#ifndef OPENAL_INTERNAL_COMMON_H_
#define OPENAL_INTERNAL_COMMON_H_

#include "FreeRTOS.h"

#define OPENAL_LIB __attribute__((section(".openAL")))
#define OPENAL_PUBDATA __attribute__((section(".openAL_pubdata")))

extern unsigned long f_cpu_awake;

typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef unsigned char BOOL;

typedef enum openal_error_code_t {
	SUCCESS = 0,
	INVALID_PIN,
	PIN_NOT_CAPABLE,
	PIN_NOT_OWNER,
	PIN_ACCESS_DENIED,

	UNKNOWN_CALLER
}openal_error_code_t;


/*
 * MPU Privilege mechanisms from FreeRTOS
 */
extern portBASE_TYPE prvRaisePrivilege() __attribute__(( naked ));
#define portRESET_PRIVILEGE( xRunningPrivileged ) if( xRunningPrivileged != pdTRUE ) __asm volatile ( " mrs r0, control \n orr r0, #1 \n msr control, r0" :::"r0" )
#define prvIsPrivileged() (!(__get_CONTROL() & 0x1))


#endif /* OPENAL_INTERNAL_COMMON_H_ */
