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
#include "portmacro.h"
#include "queue.h"

#define STACK_SIZE                     50

#define tskAPP_PRIORITY                1
#define tskSENDDONE_PRIORITY           2
#define tskRX_PRIORITY                 3

#define SCHEDULER_APP_PRIO_BOUNDARY    TASKPRIO_MAX
#define SCHEDULER_STACK_PRIO_BOUNDARY  4
#define SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY 8

#define INTERRUPT_DECLARATION()       // (xStackLock != NULL ? (xStackLock=xStackLock) : ( xStackLock = xSemaphoreCreateMutex()))
#define DISABLE_INTERRUPTS()         //  xSemaphoreTake( xStackLock, portMAX_DELAY )
#define ENABLE_INTERRUPTS()           // xSemaphoreGive( xStackLock )

//=========================== variables =======================================

scheduler_vars_t scheduler_vars;
scheduler_dbg_t  scheduler_dbg;

//typedef struct {
   /// global stack lock
 static  SemaphoreHandle_t    xStackLock;
   /// application task (takes the packet until it goes into the MAC queue)
 static   TaskHandle_t         xAppHandle;                   // task
 static   SemaphoreHandle_t    xAppSem;                      // semaphore to unlock task
   /// stack task which signals sendDone
 static  TaskHandle_t         xSendDoneHandle;              // task
 static SemaphoreHandle_t    xSendDoneSem;                 // semaphore to unlock task
   /// stack task which signals packet reception
 static TaskHandle_t         xRxHandle;                    // task
 static   SemaphoreHandle_t    xRxSem;                       // semaphore to unlock task
//} rtos_sched_v_t;

//rtos_sched_v_t rtos_sched_v;

//=========================== prototypes ======================================

//=== tasks
static void vAppTask(void* pvParameters);
static void vSendDoneTask(void* pvParameters);
static void vRxTask(void* pvParameters);

//=== helpers
static inline void scheduler_createSem(SemaphoreHandle_t* sem);
static inline void scheduler_push_task_internal(task_cbt cb, task_prio_t prio);
static inline bool scheduler_find_next_task_and_execute(
   task_prio_t          minprio,
   task_prio_t          maxprio,
   taskList_item_t*     pThisTas
);
static inline void scheduler_executeTask(taskList_item_t* pThisTask);
void vApplicationIdleHook( void);

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
    BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
   //=== step 1. insert the task into the task list
   scheduler_push_task_internal(cb, prio);
   
   //=== step 2. toggle the appropriate semaphore so the corresponding handler takes care of it
   if (
         prio <= SCHEDULER_STACK_PRIO_BOUNDARY
      ) {
	   xSemaphoreGiveFromISR(xRxSem,&xHigherPriorityTaskWoken );

   } else if (
         prio >  SCHEDULER_STACK_PRIO_BOUNDARY
         &&
         prio <= SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY
      ) {
	   xSemaphoreGiveFromISR(xSendDoneSem,&xHigherPriorityTaskWoken );

   } else if (
         prio >  SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY
         &&
         prio <= SCHEDULER_APP_PRIO_BOUNDARY
      ) {
      xSemaphoreGiveFromISR(xAppSem,&xHigherPriorityTaskWoken );
   } else {
      // TODO handle failure
      while (1) ;
   }

   portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//=========================== private =========================================

/**
Handle application packets, brinding them down the stack until they are queued,
ready for the lowwe MAC to consume.
*/
static void vAppTask(void* pvParameters) {
   bool found = FALSE;
   taskList_item_t* pThisTask = NULL;
   xSemaphoreTake(xAppSem, portMAX_DELAY);//take it for the first time so it blocks right after.
   while (1) {
      xSemaphoreTake(xAppSem, portMAX_DELAY);
      found = scheduler_find_next_task_and_execute(
         SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY,
         SCHEDULER_APP_PRIO_BOUNDARY,
         pThisTask
      );

      //leds_sync_toggle();
   }
}

/**
Handle sendDone notifications.
*/
static void vSendDoneTask(void* pvParameters) {
   bool found;
   taskList_item_t* pThisTask = NULL;
   xSemaphoreTake(xSendDoneSem, portMAX_DELAY);//take it for the first time so it blocks right after.
   while (1) {
      xSemaphoreTake(xSendDoneSem, portMAX_DELAY);
      found = scheduler_find_next_task_and_execute(
         SCHEDULER_STACK_PRIO_BOUNDARY,
         SCHEDULER_SENDDONETIMER_PRIO_BOUNDARY,
         pThisTask
      );

     // leds_radio_toggle();
   }
}

/**
Handle received packets, bringing them up to the stack.
*/
static void vRxTask(void* pvParameters) {
   bool found = FALSE;
   taskList_item_t* pThisTask = NULL;
   xSemaphoreTake(xRxSem, portMAX_DELAY); //take it for the first time so it blocks right after.
   while (1) {
      xSemaphoreTake(xRxSem, portMAX_DELAY);
      found = scheduler_find_next_task_and_execute(
         0,
         SCHEDULER_STACK_PRIO_BOUNDARY,
         pThisTask
      );


     // leds_debug_toggle();
   }
}

//=========================== helpers =========================================

/**
\brief Create and give a semaphore.
*/
static inline void scheduler_createSem(SemaphoreHandle_t* sem) {
   
   // create semaphore
   *sem = xSemaphoreCreateBinary();
   if (*sem == NULL) {
      // TODO handle failure
      return;
   }
}

/**
\brief Insert task into the task list, according to its priority.
*/
static void inline scheduler_push_task_internal(task_cbt cb, task_prio_t prio) {
   taskList_item_t*     taskContainer;
   taskList_item_t**    taskListWalker;
   //INTERRUPT_DECLARATION();

   //DISABLE_INTERRUPTS();

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
  // leds_sync_toggle();
  // ENABLE_INTERRUPTS();
}

/**
\brief Finds the next task to execute.

\param[out] pThisTask The taskList item to return.
*/
static inline bool scheduler_find_next_task_and_execute (
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
         scheduler_executeTask(pThisTask);
         return TRUE;
      }
      //it is not the first so let's look at the next one if nothing return
      if ((*prevTask)->next != NULL) {
         pThisTask = (*prevTask)->next;
      } else {
         return FALSE;
      }
      //move throught the list until we find the first element in the priority group
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
         scheduler_executeTask(pThisTask);

         return TRUE;
      }
   }
   return FALSE;
}

/**
\brief Execute a task.
*/
static inline void scheduler_executeTask(taskList_item_t* pThisTask) {
   
   // execute the current task
   pThisTask->cb();
   
   // free up this task container
   pThisTask->cb   = NULL;
   pThisTask->prio = TASKPRIO_NONE;
   pThisTask->next = NULL;
   
   //leds_radio_toggle();

   // update debug stats
   scheduler_dbg.numTasksCur--;
}

/*void vApplicationIdleHook( void ){
	leds_debug_toggle();
}
*/
