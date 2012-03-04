#ifndef __SCHEDULER_H
#define __SCHEDULER_H

/**
\addtogroup drivers
\{
\addtogroup Scheduler
\{
*/

#include "stdint.h"
#include "radio.h"

//=========================== define ==========================================

typedef enum {
   TASKPRIO_NONE               = 0x00,
   // tasks trigger by radio
   TASKPRIO_RESNOTIF_RX        = 0x01, // scheduled by IEEE802.15.4e
   TASKPRIO_RESNOTIF_TXDONE    = 0x02, // scheduled by IEEE802.15.4e
   // tasks triggered by timers
   TASKPRIO_RES                = 0x03, // scheduled by timerB CCR0 interrupt
   TASKPRIO_RPL                = 0x04, // scheduled by timerB CCR1 interrupt
   TASKPRIO_TCP_TIMEOUT        = 0x05, // scheduled by timerB CCR2 interrupt
   TASKPRIO_COAP               = 0x06, // scheduled by timerB CCR3 interrupt
   // tasks trigger by other interrupts
   TASKPRIO_BUTTON             = 0x07, // scheduled by P2.7 interrupt
   TASKPRIO_MAX                = 0x08,
} task_prio_t;

#define TASK_LIST_DEPTH      10

//=========================== typedef =========================================

typedef void (*task_cbt)(void);

//=========================== variables =======================================

//=========================== prototypes ======================================

// public functions
          void scheduler_init();
          void scheduler_start();
__monitor void scheduler_push_task(task_cbt task_cb, task_prio_t prio);

// interrupt handlers
void isr_ieee154e_newSlot();
void isr_ieee154e_timer();
void isr_adc();
#ifdef ISR_GYRO
void isr_gyro();
#endif
#ifdef ISR_LARGE_RANGE_ACCEL
void isr_large_range_accel();
#endif
#ifdef ISR_BUTTON
void isr_button();
#endif

/**
\}
\}
*/

#endif