#include "opendefs.h"
#include "neighbors_control.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "packetfunctions.h"

//=========================== defination ======================================

#define BLOCKEDNEIGHBOR_CLEANUP_TIMER_PERIOD 60000

//=========================== variables =======================================

neighbors_control_vars_t neighbors_control_vars;

//=========================== prototypes ======================================

void neighbors_control_timer_monitor_cb(opentimer_id_t id);
void neighbors_control_timer_cleanBlockedNeighbor_cb(opentimer_id_t id);

//=========================== public ==========================================

void neighbors_control_init(void) {
    // clear module variables
    memset(&neighbors_control_vars,0,sizeof(neighbors_control_vars_t));
    neighbors_control_vars.periodMaintenance = NEIGHBORSCONTROL_TIMERPERIOD;
    
    neighbors_control_vars.periodCleanBlockedNeighbor = BLOCKEDNEIGHBOR_CLEANUP_TIMER_PERIOD;
    neighbors_control_vars.cleanTimerId = opentimers_start(
                                    neighbors_control_vars.periodCleanBlockedNeighbor,
                                    TIMER_PERIODIC,
                                    TIME_MS,
                                    neighbors_control_timer_cleanBlockedNeighbor_cb
                                 );
}

void neighbors_control_startTimer(open_addr_t* neighbor){
    // get a time for the neighbor
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (neighbors_control_vars.timers[i].used==FALSE){
            neighbors_control_vars.timers[i].used = TRUE;
            neighbors_control_vars.timers[i].neighbor = neighbor;
            neighbors_control_vars.timers[i].id       = opentimers_start(
                                      neighbors_control_vars.periodMaintenance,
                                      TIMER_ONESHOT,
                                      TIME_MS,
                                      neighbors_control_timer_monitor_cb
                                   );
            break;
        }
    }
    if (i==MAXNUMNEIGHBORS){
        printf("too many neighbors to monitor!\n");
    }
}

void neighbors_control_cancelTimer(open_addr_t* neighbor){
    // get a time for the neighbor
    uint8_t i;
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (
          neighbors_control_vars.timers[i].used==TRUE &&
          packetfunctions_sameAddress(neighbor,neighbors_control_vars.timers[i].neighbor)
        ){
            neighbors_control_vars.timers[i].used     = FALSE;
            neighbors_control_vars.timers[i].neighbor = NULL;
            neighbors_control_vars.timers[i].id       = 0;
            break;
        }
    }
    if (i==MAXNUMNEIGHBORS){
        printf("No such timer activated in NEIGHBORS_CONTROL!\n");
    }
}

//=========================== private =========================================

void neighbors_control_timer_monitor_cb(opentimer_id_t id){
    uint8_t i;
    uint8_t index;
    // 1. find the timer according to the id
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (
            neighbors_control_vars.timers[i].used==TRUE &&
            neighbors_control_vars.timers[i].id  == id
        ){
            index = neighbors_getNeighborIndex(neighbors_control_vars.timers[i].neighbor);
            neighbors_blockNeighbor(index);
            break;
        }
    }
}

void neighbors_control_timer_cleanBlockedNeighbor_cb(opentimer_id_t id){
    neighbors_removeBlockedNeighbors();
}