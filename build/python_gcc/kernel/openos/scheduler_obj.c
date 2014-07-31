/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:23.506510.
*/
/**
\brief OpenOS scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "openwsn_obj.h"
#include "scheduler_obj.h"
#include "board_obj.h"
#include "debugpins_obj.h"
#include "leds_obj.h"

//=========================== variables =======================================

// declaration of global variable _scheduler_vars_ removed during objectification.
// declaration of global variable _scheduler_dbg_ removed during objectification.

//=========================== prototypes ======================================

void consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init(OpenMote* self) {   
   
   // initialization module variables
   memset(&(self->scheduler_vars),0,sizeof(scheduler_vars_t));
   memset(&(self->scheduler_dbg),0,sizeof(scheduler_dbg_t));
   
   // enable the scheduler's interrupt so SW can wake up the scheduler
   SCHEDULER_ENABLE_INTERRUPT();
}

void scheduler_start(OpenMote* self) {
   taskList_item_t* pThisTask;
   while (1) {
      while((self->scheduler_vars).task_list!=NULL) {
         // there is still at least one task in the linked-list of tasks
         
         // the task to execute is the one at the head of the queue
         pThisTask                = (self->scheduler_vars).task_list;
         
         // shift the queue by one task
         (self->scheduler_vars).task_list = pThisTask->next;
         
         // execute the current task
         pThisTask->cb(self);
         
         // free up this task container
         pThisTask->cb            = NULL;
         pThisTask->prio          = TASKPRIO_NONE;
         pThisTask->next          = NULL;
         (self->scheduler_dbg).numTasksCur--;
      }
 debugpins_task_clr(self);
 board_sleep(self);
 debugpins_task_set(self);                      // IAR should halt here if nothing to do
   }
}

 void scheduler_push_task(OpenMote* self, task_cbt cb, task_prio_t prio) {
   taskList_item_t*  taskContainer;
   taskList_item_t** taskListWalker;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   
   // find an empty task container
   taskContainer = &(self->scheduler_vars).taskBuf[0];
   while (taskContainer->cb!=NULL &&
          taskContainer<=&(self->scheduler_vars).taskBuf[TASK_LIST_DEPTH-1]) {
      taskContainer++;
   }
   if (taskContainer>&(self->scheduler_vars).taskBuf[TASK_LIST_DEPTH-1]) {
      // task list has overflown. This should never happpen!
   
      // we can not print from within the kernel. Instead:
      // blink the error LED
 leds_error_blink(self);
      // reset the board
 board_reset(self);
   }
   // fill that task container with this task
   taskContainer->cb              = cb;
   taskContainer->prio            = prio;
   
   // find position in queue
   taskListWalker                 = &(self->scheduler_vars).task_list;
   while (*taskListWalker!=NULL &&
          (*taskListWalker)->prio < taskContainer->prio) {
      taskListWalker              = (taskList_item_t**)&((*taskListWalker)->next);
   }
   // insert at that position
   taskContainer->next            = *taskListWalker;
   *taskListWalker                = taskContainer;
   // maintain debug stats
   (self->scheduler_dbg).numTasksCur++;
   if ((self->scheduler_dbg).numTasksCur>(self->scheduler_dbg).numTasksMax) {
      (self->scheduler_dbg).numTasksMax   = (self->scheduler_dbg).numTasksCur;
   }
   
   ENABLE_INTERRUPTS();
}

//=========================== private =========================================
