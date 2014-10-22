/**
\brief Task scheduler.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, October 2014.
\author Pere Tuset <peretuset@openmote.com>, October 2014.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, October 2014.
*/

#include "opendefs.h"
#include "scheduler.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
// freertos includes
#include "projdefs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portable.h"
#include "queue.h"

#define STACK_SIZE                     50

#define tskAPP_PRIORITY                1
#define tskSENDDONE_PRIORITY           2
#define tskRX_PRIORITY                 3

#define SCHEDULER_APP_PRIO_BOUNDARY    TASKPRIO_MAX
#define SCHEDULER_STACK_PRIO_BOUNDARY  4
#define SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY 8

#define INTERRUPT_DECLARATION()        (xStackLock != NULL ? (xStackLock=xStackLock) : ( xStackLock = xSemaphoreCreateMutex()))
#define DISABLE_INTERRUPTS()           xSemaphoreTake( xStackLock, portMAX_DELAY )
#define ENABLE_INTERRUPTS()            xSemaphoreGive( xStackLock )

//=========================== variables =======================================

scheduler_vars_t scheduler_vars;
scheduler_dbg_t  scheduler_dbg;

//typedef struct {
   /// global stack lock
   SemaphoreHandle_t    xStackLock;
   /// application task (takes the packet until it goes into the MAC queue)
   TaskHandle_t         xAppHandle;                   // task
   SemaphoreHandle_t    xAppSem;                      // semaphore to unlock task
   /// stack task which signals sendDone
   TaskHandle_t         xSendDoneHandle;              // task
   SemaphoreHandle_t    xSendDoneSem;                 // semaphore to unlock task
   /// stack task which signals packet reception
   TaskHandle_t         xRxHandle;                    // task
   SemaphoreHandle_t    xRxSem;                       // semaphore to unlock task
//} rtos_sched_v_t;

//rtos_sched_v_t rtos_sched_v;

//=========================== prototypes ======================================

//=== tasks
void vAppTask(void* pvParameters);
void vSendDoneTask(void* pvParameters);
void vRxTask(void* pvParameters);

//=== helpers
void scheduler_createSem(SemaphoreHandle_t* sem);
void scheduler_push_task_internal(task_cbt cb, task_prio_t prio);
bool scheduler_find_next_task(
   task_prio_t          minprio,
   task_prio_t          maxprio,
   taskList_item_t*     pThisTas
);
void scheduler_executeTask(taskList_item_t* pThisTask);

//=========================== public ==========================================

void scheduler_init() {
   
   // clear module variables
   //memset(&rtos_sched_v,0,sizeof(rtos_sched_v_t));
   
   //=== stack lock
   // create
   xStackLock = xSemaphoreCreateMutex();
   if (xStackLock == NULL) {
      //TODO handle failure
      return;
   }
   /*// by default, stack isn't locked
   if (xSemaphoreGive(xStackLock) != pdTRUE) {
      //TODO handle failure
      return;
   }*/

   //=== scheduler lock
   // TODO?
   
   //=== app task
   // task
   // semaphore
   scheduler_createSem(&xAppSem);


   xTaskCreate(
      vAppTask,
      "app",
      STACK_SIZE,
      NULL,
      tskAPP_PRIORITY,
      &xAppHandle
   );
   configASSERT(xAppHandle);

   
   //=== stack task sendDone
   // task
   // semaphore
   scheduler_createSem(&xSendDoneSem);
   xTaskCreate(
      vSendDoneTask,
      "sendDone",
      STACK_SIZE,
      NULL,
      tskSENDDONE_PRIORITY,
      &xSendDoneHandle
   );
   configASSERT(xSendDoneHandle);

   
   //=== stack task rx
   // semaphore
   scheduler_createSem(&xRxSem);
   // task
   xTaskCreate(
      vRxTask,
      "rx",
      STACK_SIZE,
      NULL,
      tskRX_PRIORITY,
      &xRxHandle
   );
   configASSERT(xRxHandle);

}

void scheduler_start() {
   // start scheduling tasks
   vTaskStartScheduler();

   // If all is well we will never reach here as the scheduler will now be
   // running.  If we do reach here then it is likely that there was
   // insufficient heap available for the idle task to be created.
   for (;;) ;
}

void scheduler_push_task(task_cbt cb, task_prio_t prio) {
   
   //=== step 1. insert the task into the task list
   scheduler_push_task_internal(cb, prio);
   
   //=== step 2. toggle the appropriate semaphore so the corresponding handler takes care of it
   if (
         prio <= SCHEDULER_STACK_PRIO_BOUNDARY
      ) {
      
      if (xSemaphoreGive(xRxSem) != pdTRUE) {
         // TODO handle failure
         return;
      }
   } else if (
         prio >  SCHEDULER_STACK_PRIO_BOUNDARY
         &&
         prio <= SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY
      ) {
      
      if (xSemaphoreGive(xSendDoneSem) != pdTRUE) {
         // TODO handle failure
         return;
      }
   } else if (
         prio >  SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY
         &&
         prio <= SCHEDULER_APP_PRIO_BOUNDARY
      ) {
      
      if (xSemaphoreGive(xAppSem) != pdTRUE) {
         // TODO handle failure
         return;
      }
   } else {
      // TODO handle failure
      while (1) ;
   }
}

//=========================== private =========================================

/**
Handle application packets, brinding them down the stack until they are queued,
ready for the lowwe MAC to consume.
*/
void vAppTask(void* pvParameters) {
   bool found = FALSE;
   taskList_item_t* pThisTask = NULL;
   
   while (1) {
      xSemaphoreTake(xAppSem, portMAX_DELAY);
      found = scheduler_find_next_task(
         SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY,
         SCHEDULER_APP_PRIO_BOUNDARY,
         pThisTask
      );
      if (found) {
         // execute the current task
         scheduler_executeTask(pThisTask);
      }
      leds_sync_toggle();
   }
}

/**
Handle sendDone notifications.
*/
void vSendDoneTask(void* pvParameters) {
   bool found;
   taskList_item_t* pThisTask = NULL;
   
   while (1) {
      xSemaphoreTake(xSendDoneSem, portMAX_DELAY);
      found = scheduler_find_next_task(
         SCHEDULER_STACK_PRIO_BOUNDARY,
         SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY,
         pThisTask
      );
      if (found) {
         // execute the current task
         scheduler_executeTask(pThisTask);
      }
      leds_radio_toggle();
   }
}

/**
Handle received packets, bringing them up to the stack.
*/
void vRxTask(void* pvParameters) {
   bool found = FALSE;
   taskList_item_t* pThisTask = NULL;
   
   while (1) {
      xSemaphoreTake(xRxSem, portMAX_DELAY);
      found = scheduler_find_next_task(
         0,
         SCHEDULER_STACK_PRIO_BOUNDARY,
         pThisTask
      );
      if (found) {
         // execute the current task
         scheduler_executeTask(pThisTask);
      }

      leds_debug_toggle();
   }
}

//=========================== helpers =========================================

/**
\brief Create and give a semaphore.
*/
void scheduler_createSem(SemaphoreHandle_t* sem) {
   
   // create semaphore
   *sem = xSemaphoreCreateBinary();
   if (*sem == NULL) {
      // TODO handle failure
      return;
   }
   
   // give semaphore
   if (xSemaphoreGive(*sem) != pdTRUE) {
      // TODO handle failure
      return;
   }
}

/**
\brief Insert task into the task list, according to its priority.
*/
void scheduler_push_task_internal(task_cbt cb, task_prio_t prio) {
   taskList_item_t*     taskContainer;
   taskList_item_t**    taskListWalker;
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();

   // find an empty task container
   taskContainer = &scheduler_vars.taskBuf[0];
   while (
      taskContainer->cb != NULL
      &&
      taskContainer <= &scheduler_vars.taskBuf[TASK_LIST_DEPTH - 1]
      ) {
      taskContainer++;
   }
   if (taskContainer > &scheduler_vars.taskBuf[TASK_LIST_DEPTH - 1]) {
      // task list has overflown. This should never happen!

      // we can not print from within the kernel. Instead:
      // blink the error LED
      leds_error_blink();
      // reset the board
      board_reset();
   }
   // fill that task container with this task
   taskContainer->cb    = cb;
   taskContainer->prio  = prio;

   // find position in queue
   taskListWalker = &scheduler_vars.task_list;
   while (
      *taskListWalker != NULL
      &&
      (*taskListWalker)->prio < taskContainer->prio
      ) {
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

/**
\brief Finds the next task to execute.

\param[out] pThisTask The taskList item to return.
*/
bool scheduler_find_next_task (
      task_prio_t       minprio,
      task_prio_t       maxprio,
      taskList_item_t*  pThisTask
   ) {
   //to shift
   taskList_item_t** prevTask;
   
   if (scheduler_vars.task_list != NULL) {
      // start searching by the task at the head of the queue
      prevTask = &scheduler_vars.task_list;

      // check first element is not the one we want.
      if ((*prevTask)->prio >= minprio && (*prevTask)->prio < maxprio) {
         pThisTask = (*prevTask);
         scheduler_vars.task_list = pThisTask->next;
         return TRUE;
      }
      
      if ((*prevTask)->next != NULL) {
         pThisTask = (*prevTask)->next;
      } else {
         return FALSE;
      }
      
      while (
         !(pThisTask->prio >= minprio && pThisTask->prio < maxprio)
         &&
         pThisTask != NULL
         ) {
         
         //advance both prev and thistask
         prevTask = (taskList_item_t**) &((*prevTask)->next);
         pThisTask = (*prevTask)->next;
      }

      if (pThisTask == NULL) {
         //not found
         return FALSE;
      } else {
         //found
         //link the list again and remove the selected task
         (*prevTask)->next = pThisTask->next;
         return TRUE;
      }
   }
   return FALSE;
}

/**
\brief Execute a task.
*/
void scheduler_executeTask(taskList_item_t* pThisTask) {
   
   // execute the current task
   pThisTask->cb();
   
   // free up this task container
   pThisTask->cb   = NULL;
   pThisTask->prio = TASKPRIO_NONE;
   pThisTask->next = NULL;
   
   // update debug stats
   scheduler_dbg.numTasksCur--;
}
