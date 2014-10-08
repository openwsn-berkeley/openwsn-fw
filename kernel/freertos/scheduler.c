/**
 \brief Task scheduler.

 \author Thomas Watteyne <watteyne@eecs.berkeley.edu>,
 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>,
 \author Pere Tuset <peretuset@openmote.com>, October 2014.
 */

#include "opendefs.h"
#include "scheduler.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
//freertos includes
#include "projdefs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portable.h"
#include "queue.h"

#define STACK_SIZE 256
#define tskIDLE_PRIORITY 2

#define INTERRUPT_DECLARATION()  (xGlobalMutex != NULL ?: ( xGlobalMutex = xSemaphoreCreateMutex()))
#define DISABLE_INTERRUPTS() xSemaphoreTake(xGlobalMutex, portMAX_DELAY )
#define ENABLE_INTERRUPTS() xSemaphoreGive( xGlobalMutex )

//=========================== variables =======================================

scheduler_vars_t scheduler_vars;
scheduler_dbg_t scheduler_dbg;

//an application tasks which takes the packet until it goes into the MAC queue
TaskHandle_t xAppStackHandle = NULL;
TaskHandle_t xSendDoneAndTimerHandle = NULL;
TaskHandle_t xStackRxHandle = NULL;

//GLOBAL variables to be moved to some rtos.h file
SemaphoreHandle_t xGlobalMutex;

//=========================== prototypes ======================================
void vAppStackTask(void * pvParameters);
void vSendDoneAndTimerTask(void * pvParameters);
void vStackRxTask(void * pvParameters);
//=========================== public ==========================================

void scheduler_init() {

	xGlobalMutex = xSemaphoreCreateMutex();
	if (xGlobalMutex == NULL) {
		//fail
	}
	//give it
	if (xSemaphoreGive(xGlobalMutex) != pdTRUE) {
		//TODO handle failure
	}

	//create the tasks
	xTaskCreate(vStackRxTask, "StackRX", STACK_SIZE, NULL, tskIDLE_PRIORITY,
			&xStackRxHandle);
	configASSERT(xStackRxHandle);


	xTaskCreate(vStackRxTask, "StackApp", STACK_SIZE, NULL, tskIDLE_PRIORITY,
			&xAppStackHandle);
	configASSERT(xAppStackHandle);


	xTaskCreate(vSendDoneAndTimerTask, "StackTimer", STACK_SIZE, NULL,
			tskIDLE_PRIORITY, &xSendDoneAndTimerHandle);
	configASSERT(xSendDoneAndTimerHandle);


}

void scheduler_start() {
	/* Start the tasks running. */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
		running.  If we do reach here then it is likely that there was insufficient
		heap available for the idle task to be created. */
	for( ;; );
}

void scheduler_push_task(task_cbt cb, task_prio_t prio) {
	// TODO: fill in as part of FW-16.
}

//=========================== private =========================================

/* Task to handle reception of packets up to the stack. */
void vStackRxTask(void * pvParameters) {
	while (1) {

		leds_debug_toggle();
		for (uint16_t i = 0; i < 0xffff; i++)
			;	      //delay

	}
}

/* Task to handle app packets downstream the stack up to be queued at MAC. */
void vAppStackTask(void * pvParameters) {
	while (1) {

		leds_sync_toggle();
		for (uint16_t i = 0; i < 0x3fff; i++)
			;	      //delay

	}
}

/* Task to handle senddone notification and app level pushed tasks (from timers). */
void vSendDoneAndTimerTask(void * pvParameters) {
	while (1) {

		leds_radio_toggle();
		for (uint16_t i = 0; i < 0x0fff; i++)
			;	      //delay

	}
}

