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
   task_cbt      task_cb;
   task_prio_t   prio;
   void*         next;
} task_llist_t;

typedef struct {
   task_llist_t    taskBuf[TASK_LIST_DEPTH];
   task_llist_t*   task_list;
} scheduler_vars_t;

scheduler_vars_t scheduler_vars;

//=========================== prototypes ======================================

__monitor void    consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init() {   
   // enable debug pins
   DEBUG_PIN_TASK_INIT();
   DEBUG_PIN_ISR_INIT();
   
   // initialization module variables
   memset(&scheduler_vars,0,sizeof(scheduler_vars_t));
   
   // enable the comparatorA interrupt to SW can wake up the scheduler
   CACTL1 = CAIE;
}

void scheduler_start() {
   task_llist_t* thisTask;
   while (1) {
      while(scheduler_vars.task_list!=NULL) {
         // there is still at least one task in the linked-list of tasks
         
         // the task to execute is the one at the head of the queue
         thisTask                 = scheduler_vars.task_list;
         
         // shift the queue by one task
         scheduler_vars.task_list = thisTask->next;
         
         // execute the current task
         thisTask->task_cb();
         
         // free up this task container
         thisTask->task_cb        = NULL;
      }
      DEBUG_PIN_TASK_CLR();
      __bis_SR_register(GIE+LPM3_bits);          // sleep, but leave interrupts and ACLK on 
      DEBUG_PIN_TASK_SET();                      // IAR should halt here if nothing to do
   }
}

__monitor void scheduler_push_task(task_cbt task_cb, task_prio_t prio) {
   task_llist_t* taskContainer;
   task_llist_t* taskListWalker;
   // find an empty task container
   taskContainer = &scheduler_vars.taskBuf[0];
   while (taskContainer->task_cb!=NULL &&
          taskContainer<=&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
      taskContainer++;
   }
   if (taskContainer>&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
      // task list has overflown
      while(1);
   }
   // fill that task container with this task
   taskContainer->task_cb    = task_cb;
   taskContainer->prio       = prio;
   // find position in queue
   taskListWalker = &scheduler_vars.taskBuf[0];
   while (taskListWalker->prio<taskContainer->prio &&
         taskListWalker->next!=NULL ) {
      taskListWalker++;
   }
   // insert at that position
   taskContainer->next       = taskListWalker->next;
   taskListWalker->next      = taskContainer;
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