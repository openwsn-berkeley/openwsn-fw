#ifndef OPENWSN_SCHEDULER_H
#define OPENWSN_SCHEDULER_H

/**
\addtogroup kernel
\{
\addtogroup Scheduler
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

typedef enum {
    TASKPRIO_NONE                  = 0x00,
    // tasks trigger by radio
    TASKPRIO_SIXTOP_NOTIF_RX       = 0x01,
    TASKPRIO_SIXTOP_NOTIF_TXDONE   = 0x02,
    // tasks triggered by timers
    TASKPRIO_OPENTIMERS            = 0x03,
    TASKPRIO_SIXTOP                = 0x04,
    TASKPRIO_FRAG                  = 0x05,
    TASKPRIO_IPHC                  = 0x06,
    TASKPRIO_RPL                   = 0x07,
    TASKPRIO_UDP                   = 0x08,
    TASKPRIO_COAP                  = 0x09,
    TASKPRIO_ADAPTIVE_SYNC         = 0x0a,
    TASKPRIO_MSF                   = 0x0b,
    // tasks trigger by other interrupts
    TASKPRIO_BUTTON                = 0x0c,
    TASKPRIO_SIXTOP_TIMEOUT        = 0x0d,
    TASKPRIO_SNIFFER               = 0x0e,
    TASKPRIO_OPENSERIAL            = 0X0f,
    TASKPRIO_MAX                   = 0x10,
} task_prio_t;

#define TASK_LIST_DEPTH           10

//=========================== typedef =========================================

typedef void (*task_cbt)(void);

//=========================== module variables ================================


//=========================== prototypes ======================================

void scheduler_init(void);
void scheduler_start(void);
void scheduler_push_task(task_cbt task_cb, task_prio_t prio);

#if SCHEDULER_DEBUG_ENABLE
uint8_t scheduler_debug_get_TasksCur(void);
uint8_t scheduler_debug_get_TasksMax(void);
#endif

#include "scheduler_types.h"

/**
\}
\}
*/

#endif /* OPENWSN_SCHEDULER_H */
