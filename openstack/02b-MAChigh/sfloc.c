#include "opendefs.h"
#include "sfloc.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "idmanager.h"
#include "openapps.h"

//=========================== definition =====================================

#define sflocTHRESHOLD      2

//=========================== variables =======================================

sfloc_vars_t sfloc_vars;

//=========================== prototypes ======================================

void sfloc_addCell_task(void);
void sfloc_removeCell_task(void);
void sfloc_bandwidthEstimate_task(void);

//=========================== public ==========================================

void sfloc_init(void) {
    memset(&sfloc_vars,0,sizeof(sfloc_vars_t));
    sfloc_vars.numAppPacketsPerSlotFrame = 0;
}

void sfloc_notif_addedCell(void) {
   scheduler_push_task(sfloc_addCell_task,TASKPRIO_SF0);
}

void sfloc_notif_removedCell(void) {
   scheduler_push_task(sfloc_removeCell_task,TASKPRIO_SF0);
}

// this function is called once per slotframe. 
void sfloc_notifyNewSlotframe(void) {
   scheduler_push_task(sfloc_bandwidthEstimate_task,TASKPRIO_SF0);
}

//=========================== private =========================================

void sfloc_addCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   // get preferred parent
   foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   sixtop_setHandler(SIX_HANDLER_SFLOC);
   // call sixtop
   sixtop_request(
      IANA_6TOP_CMD_ADD,
      &neighbor,
      1,
      sixtop_get_trackbesteffort()           //sfloc > only the best effort track
   );
}

void sfloc_removeCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   // get preferred parent
   foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   sixtop_setHandler(SIX_HANDLER_SFLOC);
   // call sixtop
   sixtop_request(
      IANA_6TOP_CMD_DELETE,
      &neighbor,
      1,
      sixtop_get_trackbesteffort()           //sfloc > only the best effort track
   );
}

void sfloc_bandwidthEstimate_task(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    int8_t         bw_outgoing;
    int8_t         bw_incoming;
    int8_t         bw_self;
    
    // do not reserve cells if I'm a DAGroot
    if (idmanager_getIsDAGroot()){
        return;
    }
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    
    sixtop_setHandler(SIX_HANDLER_SFLOC);
    
    // get bandwidth of outgoing, incoming and self.
    // Here we just calculate the estimated bandwidth for 
    // the application sending on dedicate cells(TX or Rx).
    bw_outgoing = schedule_getNumOfSlotsByType(CELLTYPE_TX);
    bw_incoming = schedule_getNumOfSlotsByType(CELLTYPE_RX);
    
    // get self required bandwith, you can design your
    // application and assign bw_self accordingly. 
    // for example:
    //    bw_self = application_getBandwdith(app_name);
    // By default, it's set to zero.
    // bw_self = openapps_getBandwidth(COMPONENT_UINJECT);
    bw_self = sfloc_vars.numAppPacketsPerSlotFrame;
    
    // In sfloc, scheduledCells = bw_outgoing
    //         requiredCells  = bw_incoming + bw_self
    // when scheduledCells<requiredCells, add one or more cell
    
    if (bw_outgoing <= bw_incoming+bw_self){
        
        // all cell(s)
        sixtop_request(
            IANA_6TOP_CMD_ADD,
            &neighbor,
            bw_incoming+bw_self-bw_outgoing+1,
            sixtop_get_trackbesteffort()           //sfloc > only the best effort track
        );
    } else {
        
        // remove cell(s)
        if ( (bw_incoming+bw_self) < (bw_outgoing-sflocTHRESHOLD)) {
            sixtop_setHandler(SIX_HANDLER_SFLOC);
            
            sixtop_request(
                IANA_6TOP_CMD_DELETE,
                &neighbor,
                sflocTHRESHOLD,
                sixtop_get_trackbesteffort()           //sfloc > only the best effort track
            );
        } else {
            // nothing to do
        }
    }
}

void sfloc_appPktPeriod(uint8_t numAppPacketsPerSlotFrame){
    sfloc_vars.numAppPacketsPerSlotFrame = numAppPacketsPerSlotFrame;
}
