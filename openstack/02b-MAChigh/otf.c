#include "opendefs.h"
#include "otf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "cstorm.h"
#include "idmanager.h"

//=========================== variables =======================================

#define OTFTHRESHLOW      0 
#define OTFTHRESHHIGH     0

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
    
    uint8_t bw_outgoing;
    uint8_t bw_incoming;
    uint8_t bw_self;
   
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
    bw_self     = 1515/cstorm_getPeriod();
    
    if (
        idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x06 && \
        idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x07 && \
        idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x08 && \
        idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x09  
    ) {
        // those motes has stopped to generate packets
        bw_self = 0; 
    }    
    
    printf("OTF: Mote %d ",idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    printf("OTF: outgoing = %d, incoming = %d, self = %d\n",
           bw_outgoing,bw_incoming,bw_self);
    
    if (bw_outgoing < bw_incoming+bw_self){
        sixtop_addCells(
            &neighbor,
            bw_incoming+bw_self-bw_outgoing
        );
        printf("OTF: RESEVER\n");
    } else {
        if (bw_outgoing > bw_incoming+bw_self){
            sixtop_removeCell(
                &neighbor,
                bw_outgoing-bw_incoming-bw_self
            );
            printf("OTF: REMOVE\n");
        } else {
            // the bandwidth is able to statisfied the traffic
            // nothing to do
        }
    }
}