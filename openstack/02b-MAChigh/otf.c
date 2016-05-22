#include "opendefs.h"
#include "otf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "cstorm.h"
#include "idmanager.h"

//=========================== variables =======================================

#define SOURCE_MOTE 0xcb
#define SF0THRESH      3 
//#define SF0_DEBUG

//=========================== prototypes ======================================

void otf_addCell_task(void);
void otf_removeCell_task(void);
void otf_bandwidthEstimate_task(void);

//=========================== public ==========================================

void otf_init(void) {
}

void otf_notif_addedCell(void) {
   scheduler_push_task(otf_addCell_task,TASKPRIO_OTF);
}

void otf_notif_removedCell(void) {
   scheduler_push_task(otf_removeCell_task,TASKPRIO_OTF);
}

void otf_notifyNewSlotframe(void) {
   scheduler_push_task(otf_bandwidthEstimate_task,TASKPRIO_OTF);
}

//=========================== private =========================================

void otf_addCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   sixtop_setHandler(SIX_HANDLER_OTF);
   // call sixtop
   sixtop_addCells(
      &neighbor,
      1
   );
}

void otf_removeCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   sixtop_setHandler(SIX_HANDLER_OTF);
   // call sixtop
   sixtop_removeCell(
      &neighbor,
      1
   );
}

void otf_bandwidthEstimate_task(void){
    open_addr_t neighbor;
    bool    foundNeighbor;
    
    int8_t bw_outgoing;
    int8_t bw_incoming;
    int8_t bw_self;
   
    // do not reserve cell proactively if I was dagroot
    if (idmanager_getIsDAGroot()){
        return;
    }
    
    // get preferred parent
    foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
       return;
    }
   
    sixtop_setHandler(SIX_HANDLER_OTF);

    // get bandwidth of outgoing, incoming and self
    bw_outgoing = schedule_getNumOfSlotsByType(CELLTYPE_TX);
    bw_incoming = schedule_getNumOfSlotsByType(CELLTYPE_RX);
    // number of packet generated per second (slotframe duration 15ms*101=1515ms)
    bw_self     = 15*SLOTFRAME_LENGTH/cstorm_getPeriod();
    
    if (
        idmanager_getMyID(ADDR_64B)->addr_64b[7] != SOURCE_MOTE
    ) {
        // those motes has stopped to generate packets
        bw_self = 0; 
    }
#ifdef SF0_DEBUG
    printf("OTF: Mote %d ",idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    printf("OTF: outgoing = %d, incoming = %d, self = %d\n",
           bw_outgoing,bw_incoming,bw_self);
#endif
    
    if (bw_outgoing < bw_incoming+bw_self){
      if (idmanager_getMyID(ADDR_64B)->addr_64b[7] == SOURCE_MOTE){
        sixtop_addCells(
            &neighbor,
            bw_incoming+bw_self-bw_outgoing+2
        );
      }else {
        sixtop_addCells(
            &neighbor,
            bw_incoming+bw_self-bw_outgoing
        );
      }
#ifdef SF0_DEBUG
        printf("OTF: RESEVER\n");
#endif
    } else {
        if ((bw_outgoing-SF0THRESH) > bw_incoming+bw_self){
            sixtop_removeCell(
                &neighbor,
                bw_outgoing-bw_incoming-bw_self
            );
#ifdef SF0_DEBUG
            printf("OTF: REMOVE\n");
#endif
        } else {
            // the bandwidth is able to statisfied the traffic
            // nothing to do
        }
    }
}