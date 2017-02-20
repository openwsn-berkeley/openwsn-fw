#ifndef __STATS_H
#define __STATS_H

/**
\addtogroup cross-layers
\{
\addtogroup Stats
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

// types of info
enum {
   STAT_QUEUE_USE                      = 0,
   STAT_SCHED_USE_TX                   ,
   STAT_SCHED_USE_RX                   ,
   STAT_SCHED_USE_SHARED_TXRX          ,
   STAT_SCHED_EMPTY                    ,
   STAT_DUTY_CYCLE                     ,
   STAT_PREF_PARENT_CHANGE             ,
};

//=========================== typedef =========================================



//=========================== prototypes ======================================

void     stats_init(void);
uint16_t stats_getCounter(uint8_t type);
/**
\}
\}
*/

#endif
