#include "opendefs.h"
#include "neighbors_control.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "idmanager.h"

//=========================== defination ======================================

//#define NEIGHBORS_CONTROL_DEBUG
#define BLOCKEDNEIGHBOR_CLEANUP_TIMER_PERIOD 60000

//=========================== variables =======================================

neighbors_control_vars_t neighbors_control_vars;

//=========================== prototypes ======================================

void neighbors_control_timer_monitor_cb(opentimer_id_t id);
void neighbors_control_timer_cleanBlockedNeighbor_cb(opentimer_id_t id);

void debug_printAllTimers();

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
            memcpy(&(neighbors_control_vars.timers[i].neighbor),neighbor,sizeof(open_addr_t));
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
#ifdef NEIGHBORS_CONTROL_DEBUG
        printf("this is mote %d too many neighbors to monitor!\n",idmanager_getMyID(ADDR_16B)->addr_16b[1]);
#endif
    }
}

void neighbors_control_cancelTimer(open_addr_t* neighbor){
    // get a time for the neighbor
    uint8_t i;
#ifdef NEIGHBORS_CONTROL_DEBUG
    printf("this is mote %d cancel hitted, release!\n", idmanager_getMyID(ADDR_16B)->addr_16b[1]);
#endif
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (
          neighbors_control_vars.timers[i].used==TRUE &&
          packetfunctions_sameAddress(neighbor,&(neighbors_control_vars.timers[i].neighbor))
        ){
            neighbors_control_vars.timers[i].used     = FALSE;
            memset(&(neighbors_control_vars.timers[i].neighbor),0,sizeof(open_addr_t));
#ifdef NEIGHBORS_CONTROL_DEBUG
            printf("timer ID = %d released\n",neighbors_control_vars.timers[i].id);
#endif
            opentimers_stop(neighbors_control_vars.timers[i].id);
            neighbors_control_vars.timers[i].id       = 0;
            break;
        }
    }
    if (i==MAXNUMNEIGHBORS){
#ifdef NEIGHBORS_CONTROL_DEBUG
        printf("No such timer activated in NEIGHBORS_CONTROL!\n");
#endif
    }
}

void neighbors_control_removeTimer(open_addr_t*   neighbor){
    uint8_t i;

    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (
            neighbors_control_vars.timers[i].used==TRUE &&
            packetfunctions_sameAddress(neighbor,&(neighbors_control_vars.timers[i].neighbor))
        ){
            opentimers_stop(neighbors_control_vars.timers[i].id);
            neighbors_control_vars.timers[i].used     = FALSE;
            memset(&(neighbors_control_vars.timers[i].neighbor),0,sizeof(open_addr_t));
            neighbors_control_vars.timers[i].id       = 0;
        }
    }
}

//=========================== private =========================================

void neighbors_control_timer_monitor_cb(opentimer_id_t id){
    uint8_t i,index;
    bool found;
    open_addr_t   parent;
    
    for (i=0;i<MAXNUMNEIGHBORS;i++){
        if (
            neighbors_control_vars.timers[i].used==TRUE &&
            neighbors_control_vars.timers[i].id  == id
        ){
                // donot block my parent
            if (neighbors_getPreferredParentEui64(&parent)==TRUE){
                if (
                    packetfunctions_sameAddress(&parent,&(neighbors_control_vars.timers[i].neighbor))
                ){
                    neighbors_control_vars.timers[i].used     = FALSE;
                    memset(&(neighbors_control_vars.timers[i].neighbor),0,sizeof(open_addr_t));
                    opentimers_stop(neighbors_control_vars.timers[i].id);
                    neighbors_control_vars.timers[i].id       = 0;
#ifdef NEIGHBORS_CONTROL_DEBUG
                    printf("this is my parent %d, release!\n",parent.addr_64b[7]);
#endif
                    return;
                }
                
            }
            found = neighbors_getNeighborIndex(&(neighbors_control_vars.timers[i].neighbor), &index);
            if (found==TRUE){
#ifdef NEIGHBORS_CONTROL_DEBUG
                printf("Block neighbor %d !\n",neighbors_control_vars.timers[i].neighbor.addr_64b[7]);
#endif
                neighbors_blockNeighbor(index);
                neighbors_updateMyDAGrankAndNeighborPreference();
            }
            break;
        }
    }
}

void neighbors_control_timer_cleanBlockedNeighbor_cb(opentimer_id_t id){
    neighbors_removeBlockedNeighbors();
}

void debug_printAllTimers(){
    uint8_t i;
#ifdef NEIGHBORS_CONTROL_DEBUG
    for(i=0;i<MAXNUMNEIGHBORS;i++){
        printf(
           "used %d neighbor %d\n",
           neighbors_control_vars.timers[i].used,
           neighbors_control_vars.timers[i].neighbor.addr_64b[7]
        );
    }
#endif
}