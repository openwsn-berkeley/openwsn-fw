/**
\brief OpenOS scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board.h"
#include "scheduler.h"
#include "openwsn.h"
#include "debugpins.h"

//=========================== variables =======================================

typedef struct task_llist_t {
   task_cbt             cb;
   task_prio_t          prio;
   void*                next;
} taskList_item_t;

typedef struct {
   taskList_item_t      taskBuf[TASK_LIST_DEPTH];
   taskList_item_t*     task_list;
   uint8_t              numTasksCur;
   uint8_t              numTasksMax;
} scheduler_vars_t;

scheduler_vars_t scheduler_vars;

typedef struct {
   uint8_t              numTasksCur;
   uint8_t              numTasksMax;
} scheduler_dbg_t;

scheduler_dbg_t scheduler_dbg;

//=========================== prototypes ======================================

void consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init() {   
   
   // initialization module variables
   memset(&scheduler_vars,0,sizeof(scheduler_vars_t));
   memset(&scheduler_dbg,0,sizeof(scheduler_dbg_t));
   
   // enable the scheduler's interrupt so SW can wake up the scheduler
   SCHEDULER_ENABLE_INTERRUPT();
}

void scheduler_start() {
   taskList_item_t* pThisTask;
   while (1) {
      while(scheduler_vars.task_list!=NULL) {
         // there is still at least one task in the linked-list of tasks
         
         // the task to execute is the one at the head of the queue
         pThisTask                = scheduler_vars.task_list;
         
         // shift the queue by one task
         scheduler_vars.task_list = pThisTask->next;
         
         // execute the current task
         pThisTask->cb();
         
         // free up this task container
         pThisTask->cb            = NULL;
         pThisTask->prio          = TASKPRIO_NONE;
         pThisTask->next          = NULL;
         scheduler_dbg.numTasksCur--;
      }
      debugpins_task_clr();
      board_sleep();
      debugpins_task_set();                      // IAR should halt here if nothing to do
   }
}

 void scheduler_push_task(task_cbt cb, task_prio_t prio) {
   taskList_item_t*  taskContainer;
   taskList_item_t** taskListWalker;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   
   // find an empty task container
   taskContainer = &scheduler_vars.taskBuf[0];
   while (taskContainer->cb!=NULL &&
          taskContainer<=&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
      taskContainer++;
   }
   if (taskContainer>&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
      // task list has overflown
      while(1);
   }
   // fill that task container with this task
   taskContainer->cb              = cb;
   taskContainer->prio            = prio;
   
   // find position in queue
   taskListWalker                 = &scheduler_vars.task_list;
   while (*taskListWalker!=NULL &&
          (*taskListWalker)->prio < taskContainer->prio) {
      taskListWalker              = (taskList_item_t**)&((*taskListWalker)->next);
   }
   // insert at that position
   taskContainer->next            = *taskListWalker;
   *taskListWalker                = taskContainer;
   // maintain debug stats
   scheduler_dbg.numTasksCur++;
   if (scheduler_dbg.numTasksCur>scheduler_dbg.numTasksMax) {
      scheduler_dbg.numTasksMax   = scheduler_dbg.numTasksCur;
   }
   
   ENABLE_INTERRUPTS();
}

//=========================== private =========================================
