#ifndef __SCTIMER_H
#define __SCTIMER_H

/**
\addtogroup BSP
\{
\addtogroup sctimer
\{

\brief A timer module with only a single compare value.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
*/

#include "stdint.h"
#include "board.h"

//=========================== typedef =========================================

typedef void  (*sctimer_cbt)(void);
typedef void  (*sctimer_capture_cbt)(PORT_TIMER_WIDTH timestamp);

#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
enum radiotimer_action_enum {
    // action items
    ACTION_LOAD_PACKET                     = 0x01,
    ACTION_SEND_PACKET                     = 0x02,
    ACTION_RADIORX_ENABLE                  = 0x03,
    ACTION_SET_TIMEOUT                     = 0x04,
    ACTION_TX_SFD_DONE                     = 0x05,
    ACTION_RX_SFD_DONE                     = 0x06,
    ACTION_TX_SEND_DONE                    = 0x07,
    ACTION_RX_DONE                         = 0x08,
    ACTION_ALL_RADIOTIMER_INTERRUPT        = 0x09,
};
#endif

//=========================== variables =======================================


//=========================== prototypes ======================================

void     sctimer_init(void);
void     sctimer_setCompare(PORT_TIMER_WIDTH val);
void     sctimer_set_callback(sctimer_cbt cb);
void     sctimer_setStartFrameCb(sctimer_capture_cbt cb);
void     sctimer_setEndFrameCb(sctimer_capture_cbt cb);
PORT_TIMER_WIDTH sctimer_readCounter(void);
void     sctimer_enable(void);
void     sctimer_disable(void);

#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
void     sctimer_set_actionCallback(sctimer_cbt cb);
void     sctimer_scheduleActionIn(uint8_t type,PORT_RADIOTIMER_WIDTH offset);
void     sctimer_actionCancel(uint8_t type);
void     sctimer_setCapture(uint8_t type);
#endif

kick_scheduler_t sctimer_isr(void);

/**
\}
\}
*/

#endif
