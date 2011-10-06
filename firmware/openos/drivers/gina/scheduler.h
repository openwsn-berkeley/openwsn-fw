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

enum {
   // tasks trigger by radio
   TASKID_RESNOTIF_RX        = 0x00, // scheduled by IEEE802.15.4e
   TASKID_RESNOTIF_TXDONE    = 0x01, // scheduled by IEEE802.15.4e
   // tasks triggered by timers
   TASKID_RES                = 0x02, // scheduled by timerB CCR0 interrupt
   TASKID_RPL                = 0x03, // scheduled by timerB CCR1 interrupt
   TASKID_TCP_TIMEOUT        = 0x04, // scheduled by timerB CCR2 interrupt
   TASKID_UDP_TIMER          = 0x05, // scheduled by timerB CCR3 interrupt
   TASKID_TIMERB4            = 0x06, // scheduled by timerB CCR4 interrupt
   TASKID_TIMERB5            = 0x07, // scheduled by timerB CCR5 interrupt
   TASKID_TIMERB6            = 0x08, // scheduled by timerB CCR6 interrupt
   // tasks trigger by other interrupts
   TASKID_BUTTON             = 0x09, // scheduled by P2.7 interrupt
   MAX_NUM_TASKS             = 0x0a,
};

#define SCHEDULER_WAKEUP()   CACTL1 |= CAIFG

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

// public functions
          void scheduler_init();
          void scheduler_start();
__monitor void scheduler_push_task(int8_t task_id);

// interrupt handlers
#ifdef OPENWSN_STACK
void isr_ieee154e_newSlot();
#endif
#ifdef OPENWSN_STACK
void isr_ieee154e_timer();
#endif
#ifdef ISR_ADC
void isr_adc();
#endif
#ifdef ISR_GYRO
void isr_gyro();
#endif
#ifdef ISR_RADIO
void isr_radio();
#endif
#ifdef ISR_LARGE_RANGE_ACCEL
void isr_large_range_accel();
#endif
#ifdef ISR_BUTTON
void isr_button();
#endif
#ifdef ISR_SPIRX
void isr_spirx();
#endif
#ifdef ISR_SPITX
void isr_spitx();
#endif
#ifdef ISR_I2CRX
void isr_i2crx();
#endif
#ifdef ISR_I2CTX
void isr_i2ctx();
#endif

/**
\}
\}
*/

#endif