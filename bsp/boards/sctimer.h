#ifndef __SCTIMER_H
#define __SCTIMER_H

/**
\addtogroup BSP
\{
\addtogroup sctimer
\{

\brief A timer module with only a single compare value.

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, April 2017.
*/

#include "stdint.h"
#include "board.h"

//=========================== typedef =========================================

typedef kick_scheduler_t (*sctimer_cbt)(void);

//=========================== variables =======================================


//=========================== prototypes ======================================

void     sctimer_init(void);
void     sctimer_setCompare(uint32_t val);
void     sctimer_set_callback(sctimer_cbt cb);
uint32_t sctimer_readCounter(void);
void     sctimer_enable(void);
void     sctimer_disable(void);

/**
\}
\}
*/

#endif
