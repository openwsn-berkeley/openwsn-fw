#include "opendefs.h"
#include "sfx.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void sfx_addCell_task(void);
void sfx_removeCell_task(void);

//=========================== public ==========================================

void sfx_init(void) {
}

void sfx_notif_addedCell(void) {
   scheduler_push_task(sfx_addCell_task,TASKPRIO_OTF);
}

void sfx_notif_removedCell(void) {
   scheduler_push_task(sfx_removeCell_task,TASKPRIO_OTF);
}

//=========================== private =========================================

void sfx_addCell_task(void) {
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

void sfx_removeCell_task(void) {
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