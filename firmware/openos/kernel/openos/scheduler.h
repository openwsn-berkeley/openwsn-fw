#ifndef __SCHEDULER_H
#define __SCHEDULER_H

/**
\addtogroup kernel
\{
\addtogroup Scheduler
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

typedef enum {
   TASKPRIO_NONE               = 0x00,
   // tasks trigger by radio
   TASKPRIO_SIXTOP_NOTIF_RX        = 0x01, // scheduled by IEEE802.15.4e
   TASKPRIO_SIXTOP_NOTIF_TXDONE    = 0x02, // scheduled by IEEE802.15.4e
   // tasks triggered by timers
   TASKPRIO_SIXTOP                = 0x03, // scheduled by timerB CCR0 interrupt
   TASKPRIO_RPL                = 0x04, // scheduled by timerB CCR1 interrupt
   TASKPRIO_TCP_TIMEOUT        = 0x05, // scheduled by timerB CCR2 interrupt
   TASKPRIO_COAP               = 0x06, // scheduled by timerB CCR3 interrupt
   TASKPRIO_ADAPTIVE_SYNC      = 0x07, 
   // tasks trigger by other interrupts
   TASKPRIO_BUTTON             = 0x08, // scheduled by P2.7 interrupt
   TASKPRIO_SIXTOP_DEBUG       = 0x09,
   TASKPRIO_MAX                = 0x0a,
} task_prio_t;

#define TASK_LIST_DEPTH      11

//=========================== typedef =========================================

typedef void (*task_cbt)(void);

typedef struct task_llist_t {
   task_cbt             cb;
   task_prio_t          prio;
   void*                next;
} taskList_item_t;

//=========================== module variables ================================

typedef struct {
   taskList_item_t      taskBuf[TASK_LIST_DEPTH];
   taskList_item_t*     task_list;
   uint8_t              numTasksCur;
   uint8_t              numTasksMax;
} scheduler_vars_t;

typedef struct {
   uint8_t              numTasksCur;
   uint8_t              numTasksMax;
} scheduler_dbg_t;

//=========================== prototypes ======================================

// public functions
void scheduler_init(void);
void scheduler_start(void);
void scheduler_push_task(task_cbt task_cb, task_prio_t prio);

// interrupt handlers
void isr_ieee154e_newSlot(void);
void isr_ieee154e_timer(void);
void isr_adc(void);
#ifdef ISR_GYRO
void isr_gyro(void);
#endif
#ifdef ISR_LARGE_RANGE_ACCEL
void isr_large_range_accel(void);
#endif
#ifdef ISR_BUTTON
void isr_button(void);
#endif

/**
\}
\}
*/

#endif
