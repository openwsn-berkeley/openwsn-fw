#ifndef __RADIOTIMER_H
#define __RADIOTIMER_H

/**
\addtogroup BSP
\{
\addtogroup radiotimer
\{

\brief Cross-platform declaration "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
#include "board.h"

//=========================== define ==========================================
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT

enum radiotimer_action_enum {
    // action items
    ACTION_LOAD_PACKET                     = 0x01,
    ACTION_SEND_PACKET                     = 0x02,
    ACTION_RADIORX_ENABLE                  = 0x03,
    ACTION_NORMAL_TIMER                    = 0x04,
    ACTION_TX_SFD_DONE                     = 0x05,
    ACTION_RX_SFD_DONE                     = 0x06,
    ACTION_TX_SEND_DONE                    = 0x07,
    ACTION_RX_DONE                         = 0x08,
    ACTION_ALL_RADIOTIMER_INTERRUPT        = 0x09,
};

#else
#endif

//=========================== typedef =========================================

typedef void (*radiotimer_compare_cbt)(void);
typedef void (*radiotimer_capture_cbt)(PORT_TIMER_WIDTH timestamp);

//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radiotimer_init(void);
void     radiotimer_setOverflowCb(radiotimer_compare_cbt cb);
void     radiotimer_setCompareCb(radiotimer_compare_cbt cb);
void     radiotimer_setStartFrameCb(radiotimer_capture_cbt cb);
void     radiotimer_setEndFrameCb(radiotimer_capture_cbt cb);
void     radiotimer_start(PORT_RADIOTIMER_WIDTH period);
// direct access
PORT_RADIOTIMER_WIDTH radiotimer_getValue(void);
void     radiotimer_setPeriod(PORT_RADIOTIMER_WIDTH period);
PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(void);
// compare
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
void     radiotimer_schedule(uint8_t type,PORT_RADIOTIMER_WIDTH offset);
void     radiotimer_cancel(uint8_t type);
void     radiotimer_setCapture(uint8_t type);
#else
void     radiotimer_schedule(PORT_RADIOTIMER_WIDTH offset);
void     radiotimer_cancel(void);
#endif
// capture
PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(void);

// interrupt handlers
kick_scheduler_t   radiotimer_isr(void);

/**
\}
\}
*/

#endif
