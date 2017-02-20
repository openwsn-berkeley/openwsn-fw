#include "opendefs.h"
#include "stats.h"
#include "openqueue.h"
#include "schedule.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void stats_init() {
   // reset local variables
}

uint16_t stats_getCounter(uint8_t type){
    uint16_t stats = 0;
    switch (type) {
    case STAT_SCHED_USE_TX:
        stats = schedule_getNumOfSlotsByType(CELLTYPE_TX);
        break;
    case STAT_SCHED_USE_SHARED_TXRX:
        stats = schedule_getNumOfSlotsByType(CELLTYPE_TXRX);
        break;
    case STAT_SCHED_USE_RX:
        stats = schedule_getNumOfSlotsByType(CELLTYPE_RX);
        break;
    case STAT_SCHED_EMPTY:
        stats = schedule_getNumberOfFreeEntries();
        break;
    case STAT_QUEUE_USE:
        stats = (uint16_t) openqueue_getQueueUsage();
        break;
    default:
        break;
    }
    return stats;
}



//=========================== private =========================================
