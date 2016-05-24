#include "opendefs.h"
#include "otf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"

//=========================== variables =======================================

#define SF0THRESH      3 

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

// this function is called once per slotframe. 
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
   sixtop_request(
      IANA_6TOP_CMD_ADD,
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
   sixtop_request(
      IANA_6TOP_CMD_DELETE,
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

    // get bandwidth of outgoing, incoming and self.
    // Here we just calcuate the estimated bandwidth for 
    // the application sending on dedicate cells(TX or Rx).
    bw_outgoing = schedule_getNumOfSlotsByType(CELLTYPE_TX);
    bw_incoming = schedule_getNumOfSlotsByType(CELLTYPE_RX);
    /* 
      get self required bandwith, you can design your
      application and assign bw_swlf accordingly. 
      for example:
          bw_self = application_getBandwdith(app_name);
      By default, it's set to zero.
    */
    bw_self = 0;
    
    // In SF0, scheduledCells = bw_outgoing
    //         requiredCells  = bw_incoming + bw_self
    
    // when scheduledCells<requiredCells, add one or more cell
    if (bw_outgoing < bw_incoming+bw_self){
         if (bw_incoming+bw_self-bw_outgoing)
         sixtop_setHandler(SIX_HANDLER_OTF);
         // call sixtop
         sixtop_request(
            IANA_6TOP_CMD_ADD,
            &neighbor,
            bw_incoming+bw_self-bw_outgoing
         );
    } else {
        // when requiredCells<(scheduledCells-SF0THRESH), remove one or more cell
        if ( (bw_incoming+bw_self) < (bw_outgoing-SF0THRESH)) {
           sixtop_setHandler(SIX_HANDLER_OTF);
           // call sixtop
           sixtop_request(
              IANA_6TOP_CMD_DELETE,
              &neighbor,
              1
           );
        } else {
            // the bandwidth is able to statisfied the traffic
            // nothing to do
        }
    }
}