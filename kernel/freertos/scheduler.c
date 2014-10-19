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

#define STACK_SIZE 50
#define tskIDLE_PRIORITY 2

#define SCHEDULER_APP_PRIO_BOUNDARY TASKPRIO_MAX //TODO REVISE THIS PRIOS
#define SCHEDULER_STACK_PRIO_BOUNDARY 4
#define SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY 8

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

//semaphores to block tasks until there is something to be done.
SemaphoreHandle_t xSchedulerAppStackSemaphore;
SemaphoreHandle_t xSchedulerStackRxSemaphore;
SemaphoreHandle_t xSchedulerSendDoneAndTimerSemaphore;

SemaphoreHandle_t xSchedulerMutex; //for mutual exclusion

//=========================== prototypes ======================================
void vAppStackTask(void * pvParameters);
void vSendDoneAndTimerTask(void * pvParameters);
void vStackRxTask(void * pvParameters);

void initializeTaskSemaphores(SemaphoreHandle_t * sem);
void internal_scheduler_push_task(task_cbt cb, task_prio_t prio);
bool internal_find_next_task(task_prio_t minprio, task_prio_t maxprio,
		taskList_item_t * pThisTask);
void executeTask(taskList_item_t* pThisTask);
//=========================== public ==========================================

void scheduler_init() {

	xGlobalMutex = xSemaphoreCreateMutex();
	if (xGlobalMutex == NULL) {
		//fail
		return;
	}
	//give it
	if (xSemaphoreGive(xGlobalMutex) != pdTRUE) {
		//TODO handle failure
		return;
	}

	initializeTaskSemaphores(&xSchedulerAppStackSemaphore);
	initializeTaskSemaphores(&xSchedulerStackRxSemaphore);
	initializeTaskSemaphores(&xSchedulerSendDoneAndTimerSemaphore);

	//create the tasks
	xTaskCreate(vStackRxTask, "StackRX", STACK_SIZE, NULL, tskIDLE_PRIORITY,
			&xStackRxHandle);configASSERT(xStackRxHandle);

	xTaskCreate(vStackRxTask, "StackApp", STACK_SIZE, NULL, tskIDLE_PRIORITY,
			&xAppStackHandle);configASSERT(xAppStackHandle);

	xTaskCreate(vSendDoneAndTimerTask, "StackTimer", STACK_SIZE, NULL,
			tskIDLE_PRIORITY, &xSendDoneAndTimerHandle);configASSERT(xSendDoneAndTimerHandle);

}

void scheduler_start() {
	/* Start the tasks running. */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
	 running.  If we do reach here then it is likely that there was insufficient
	 heap available for the idle task to be created. */
	for (;;)
		;
}

void scheduler_push_task(task_cbt cb, task_prio_t prio) {
	//1-insert the task into the task list
	internal_scheduler_push_task(cb, prio);
	//2-toggle the appropiate sempahore so the corresponding handler takes care of it.
	if (prio <= SCHEDULER_STACK_PRIO_BOUNDARY) {
		if (xSemaphoreGive(xSchedulerStackRxSemaphore) != pdTRUE) {
			//TODO handle failure
			return;
		}

	} else if (prio > SCHEDULER_STACK_PRIO_BOUNDARY
			&& prio <= SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY) {
		if (xSemaphoreGive(xSchedulerSendDoneAndTimerSemaphore) != pdTRUE) {
			//TODO handle failure
			return;
		}
	} else if (prio > SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY
			&& prio <= SCHEDULER_APP_PRIO_BOUNDARY) {
		if (xSemaphoreGive(xSchedulerAppStackSemaphore) != pdTRUE) {
			//TODO handle failure
			return;
		}
	} else {
		//THIS SHOULD NEVER HAPPEN
		while (1)
			;
	}
	/*We will need 3 different queues to split tasks into the 3 different tasks.*/
}

//=========================== private =========================================

/* Task to handle reception of packets up to the stack. */
void vStackRxTask(void * pvParameters) {
	bool found = false;
	taskList_item_t * pThisTask = NULL;
	while (1) {
		xSemaphoreTake(xSchedulerStackRxSemaphore, portMAX_DELAY);
		found = internal_find_next_task(0,
				SCHEDULER_STACK_PRIO_BOUNDARY, pThisTask);
		if (found) {
			// execute the current task
			executeTask(pThisTask);
		}

		leds_debug_toggle();
	}
}

/* Task to handle app packets downstream the stack up to be queued at MAC. */
void vAppStackTask(void * pvParameters) {
	bool found = false;
	taskList_item_t * pThisTask = NULL;
	while (1) {
		xSemaphoreTake(xSchedulerAppStackSemaphore, portMAX_DELAY);
		found = internal_find_next_task(SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY,
				SCHEDULER_APP_PRIO_BOUNDARY, pThisTask);
		if (found) {
			// execute the current task
			executeTask(pThisTask);
		}
		leds_sync_toggle();

	}
}

/* Task to handle senddone notification and app level pushed tasks (from timers). */
void vSendDoneAndTimerTask(void * pvParameters) {
	bool found;
	taskList_item_t * pThisTask = NULL;
	while (1) {
		xSemaphoreTake(xSchedulerSendDoneAndTimerSemaphore, portMAX_DELAY);
		found = internal_find_next_task(SCHEDULER_STACK_PRIO_BOUNDARY,
				SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY, pThisTask);
		if (found) {
			// execute the current task
			executeTask(pThisTask);
		}
		leds_radio_toggle();
	}
}

/* helper functions */
void initializeTaskSemaphores(SemaphoreHandle_t * sem) {

	*sem = xSemaphoreCreateBinary();
	if (*sem == NULL) {
		//fail
		return;
	}
	//give it
	if (xSemaphoreGive(*sem) != pdTRUE) {
		//TODO handle failure
		return;
	}
}

/* insert task into the list according to prio */
void internal_scheduler_push_task(task_cbt cb, task_prio_t prio) {
	taskList_item_t* taskContainer;
	taskList_item_t** taskListWalker;
	INTERRUPT_DECLARATION();

	DISABLE_INTERRUPTS();

	// find an empty task container
	taskContainer = &scheduler_vars.taskBuf[0];
	while (taskContainer->cb != NULL
			&& taskContainer <= &scheduler_vars.taskBuf[TASK_LIST_DEPTH - 1]) {
		taskContainer++;
	}
	if (taskContainer > &scheduler_vars.taskBuf[TASK_LIST_DEPTH - 1]) {
		// task list has overflown. This should never happpen!

		// we can not print from within the kernel. Instead:
		// blink the error LED
		leds_error_blink();
		// reset the board
		board_reset();
	}
	// fill that task container with this task
	taskContainer->cb = cb;
	taskContainer->prio = prio;

	// find position in queue
	taskListWalker = &scheduler_vars.task_list;
	while (*taskListWalker != NULL
			&& (*taskListWalker)->prio < taskContainer->prio) {
		taskListWalker = (taskList_item_t**) &((*taskListWalker)->next);
	}
	// insert at that position
	taskContainer->next = *taskListWalker;
	*taskListWalker = taskContainer;
	// maintain debug stats
	scheduler_dbg.numTasksCur++;
	if (scheduler_dbg.numTasksCur > scheduler_dbg.numTasksMax) {
		scheduler_dbg.numTasksMax = scheduler_dbg.numTasksCur;
	}

	ENABLE_INTERRUPTS();
}

/*
 * finds the next task to execute. out param the taskList item to return.
 *
 * */
bool internal_find_next_task(task_prio_t minprio, task_prio_t maxprio,
		taskList_item_t * pThisTask) {
	//to shift
	taskList_item_t ** prevTask;
	if (scheduler_vars.task_list != NULL) {
		// start searching by the task at the head of the queue
		prevTask = &scheduler_vars.task_list;

		//check first element is not the one we want.
		if ((*prevTask)->prio >= minprio && (*prevTask)->prio < maxprio) {
			pThisTask = (*prevTask);
			scheduler_vars.task_list = pThisTask->next;
			return true;
		}

		if ((*prevTask)->next != NULL) {
			pThisTask = (*prevTask)->next;
		} else {
			return false;
		}

		while (!(pThisTask->prio >= minprio && pThisTask->prio < maxprio)
				&& pThisTask != NULL) {
			//advance both prev and thistask
			prevTask = (taskList_item_t**) &((*prevTask)->next);
			pThisTask = (*prevTask)->next;
		}

		if (pThisTask == NULL) {
			//not found
			return false;
		} else {
			//found
			//link the list again and remove the selected task
			(*prevTask)->next = pThisTask->next;
			return true;
		}
	}
	return false;

}

/* Executes a task */
void executeTask(taskList_item_t* pThisTask) {
	// execute the current task
	pThisTask->cb();
	// free up this task container
	pThisTask->cb = NULL;
	pThisTask->prio = TASKPRIO_NONE;
	pThisTask->next = NULL;
	scheduler_dbg.numTasksCur--;
}
