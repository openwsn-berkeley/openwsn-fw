/**
\brief OpenOS scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "board.h"
#include "scheduler.h"
#include "radiotimer.h"
#include "IEEE802154E.h"
#include "i2c.h"
#include "res.h"
#include "openserial.h"
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

__monitor void    consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init() {   
   // enable debug pins
   DEBUG_PIN_TASK_INIT();
   DEBUG_PIN_ISR_INIT();
   
   // initialization module variables
   memset(&scheduler_vars,0,sizeof(scheduler_vars_t));
   memset(&scheduler_dbg,0,sizeof(scheduler_dbg_t));
   
   // enable the comparatorA interrupt to SW can wake up the scheduler
   CACTL1 = CAIE;
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
      DEBUG_PIN_TASK_CLR();
      __bis_SR_register(GIE+LPM3_bits);          // sleep, but leave interrupts and ACLK on 
      DEBUG_PIN_TASK_SET();                      // IAR should halt here if nothing to do
   }
}

__monitor void scheduler_push_task(task_cbt cb, task_prio_t prio) {
   taskList_item_t*  taskContainer;
   taskList_item_t** taskListWalker;
   
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
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

//======= interrupt which wakes up the scheduler from SW

#pragma vector = COMPARATORA_VECTOR
__interrupt void COMPARATORA_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   __bic_SR_register_on_exit(CPUOFF);                 // restart CPU
   DEBUG_PIN_ISR_CLR();
}

//======= interrupts which post a task
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
#ifdef ISR_BUTTON
   //interrupt from button connected to P2.7
   if ((P2IFG & 0x80)!=0) {
      P2IFG &= ~0x80;                                 // clear interrupt flag
      scheduler_push_task(ID_ISR_BUTTON);             // post task
      __bic_SR_register_on_exit(CPUOFF);              // restart CPU
   }
#endif
   DEBUG_PIN_ISR_CLR();
}

//======= interrupts handled directly in ISR mode

// TimerA CCR0 interrupt service routine
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   isr_ieee154e_newSlot();
   DEBUG_PIN_ISR_CLR();
}

//======= handled as CPUOFF

// TODO: this is bad practice, should redo, even a busy wait is better

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   ADC12IFG &= ~0x1F;                            // clear interrupt flags
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   DEBUG_PIN_ISR_CLR();
}