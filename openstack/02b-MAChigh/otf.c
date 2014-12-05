#include "opendefs.h"
#include "otf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "openqueue.h"
#include "openrandom.h"
#include "scheduler.h"
#include "opentimers.h"
#include "IEEE802154E.h"
#include "schedule.h"
#include "openserial.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "sixtop.h"


//=========================== variables =======================================

//=========================== prototypes ======================================

void otf_addCell_task(void);
void otf_removeCell_task(void);

//=========================== public ==========================================

void otf_init(void) {
}

void otf_notif_addedCell(void) {
   scheduler_push_task(otf_addCell_task,TASKPRIO_OTF);
}

void otf_notif_removedCell(void) {
   scheduler_push_task(otf_removeCell_task,TASKPRIO_OTF);
}

/**
 \brief a packet is pushed to the lower layer, updates the otf statistics
\param[in]     msg             The packet pushed.
\param[in]     flow_label      The flowlabel to add in the 6LoWPAN header.
\param[in]     fw_SendOrfw_Rcv The packet is originating from this mote
*/
owerror_t otf_stat_tx(
                        OpenQueueEntry_t*      msg,
                        uint32_t*              flow_label,
                        uint8_t                fw_SendOrfw_Rcv
){


   //debug (with the inserted slot/channel offset)
   openserial_printInfo(COMPONENT_OTF,
                        ERR_OTF_NEWPK,
                        (errorparameter_t)0,
                        (owerror_t)0);

   return E_SUCCESS;

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
   
   // call sixtop
   sixtop_removeCell(
      &neighbor
   );
}