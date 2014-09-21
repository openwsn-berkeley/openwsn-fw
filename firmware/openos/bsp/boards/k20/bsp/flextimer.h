/**
\brief Flextimer header module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2013.
*/

#ifndef __FLEXTIMER_H
#define __FLEXTIMER_H

#include "stdint.h"
#include "board.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef kick_scheduler_t (*flextimer_cbt)();

//=========================== variables =======================================

//=========================== prototypes ======================================

void flextimer_init();
void flextimer_reset();
void flextimer_schedule(PORT_TIMER_WIDTH val);
PORT_TIMER_WIDTH flextimer_getValue();
void flextimer_setCb(flextimer_cbt cb);
void flextimer_cancel();
void flextimer_save();
void flextimer_restore();



#endif
