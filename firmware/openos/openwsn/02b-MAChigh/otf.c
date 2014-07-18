#include "openwsn.h"
#include "otf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"

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

//=========================== private =========================================

void otf_addCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   printf("[%d] otf_addCell_task\r\n",idmanager_getMyID(ADDR_64B)->addr_64b[7]);
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   printf(
      "[%d] otf_addCell_task to %d\r\n",
      idmanager_getMyID(ADDR_64B)->addr_64b[7],
      neighbor.addr_64b[7]
   );
   
   // call sixtop
   sixtop_addCells(
      &neighbor,
      1
   );
}

void otf_removeCell_task(void) {
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   printf("[%d] otf_removeCell_task\r\n",idmanager_getMyID(ADDR_64B)->addr_64b[7]);
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   printf(
      "[%d] otf_removeCell_task to %d\r\n",
      idmanager_getMyID(ADDR_64B)->addr_64b[7],
      neighbor.addr_64b[7]
   );
   
   // call sixtop
   sixtop_removeCell(
      &neighbor
   );
}