#include "opendefs.h"
#include "sfx.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "openqueue.h"
#include "idmanager.h"

//=========================== definition ======================================

#define SFX_DEBUG

// the threshold here is a relative ratio to CELL_USAGE_CALCULATION_WINDOWS (schedule.c)
// those value must be less than CELL_USAGE_CALCULATION_WINDOWS. If the cell usage is 
// less than 
//          SFX_DELETE_THRESHOLD/CELL_USAGE_CALCULATION_WINDOWS
// remove a slot. If cell usage is more than 
//          SFX_ADD_THRESHOLD/CELL_USAGE_CALCULATION_WINDOWS
// add a lost. Else, nothing happens.
#define SFX_ADD_THRESHOLD          3 
#define SFX_TARGET                 2
#define SFX_DELETE_THRESHOLD       1

//=========================== variables =======================================

sfx_vars_t sfx_vars;

//=========================== prototypes ======================================

void sfx_addCell_task(void);
void sfx_removeCell_task(void);

//=========================== public ==========================================

void sfx_init(void) {
    sfx_vars.periodMaintenance = 0;
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
   
   sixtop_setHandler(SIX_HANDLER_SFX);
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
   
   sixtop_setHandler(SIX_HANDLER_SFX);
   // call sixtop
   sixtop_request(
      IANA_6TOP_CMD_DELETE,
      &neighbor,
      1
   );
}

void sfx_notifyNewSlotframe(void){
   open_addr_t          neighbor; 
   bool                 foundNeighbor;
   uint16_t             numberOfCells;
   uint16_t             cellUsage;
   OpenQueueEntry_t*    entry;
   
   if (idmanager_getIsDAGroot()){
      return;
   }
   
   if (sfx_vars.periodMaintenance>0){
      sfx_vars.periodMaintenance -= 1;
      return;
   }
   
   // get preferred parent
   foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
   if (foundNeighbor==FALSE) {
      return;
   }
   
   numberOfCells = schedule_getCellsCounts(
            SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE,
            CELLTYPE_TX,
            &neighbor);
   cellUsage = schedule_getTotalCellUsageStatus(CELLTYPE_TX,&neighbor);
   
   if (numberOfCells==0){
       entry = openqueue_getIpPacket();
       if (entry!=NULL ){
#ifdef SFX_DEBUG
           printf("mote %d no cell and I have packet\n",,idmanager_getMyID(ADDR_16B)->addr_16b[1]);
#endif
           sixtop_setHandler(SIX_HANDLER_SFX);
           // call sixtop
           sixtop_request(
              IANA_6TOP_CMD_ADD,
              &neighbor,
              1
           );
       }
       sfx_vars.periodMaintenance = CELL_USAGE_CALCULATION_WINDOWS;
       return;
   }
   
   
   // cell usage scheduling, bandwith estimation algorithm
   if (cellUsage/numberOfCells>=SFX_ADD_THRESHOLD){
#ifdef SFX_DEBUG
       printf("mote %d reserve one\n",idmanager_getMyID(ADDR_16B)->addr_16b[1]);
#endif
       sixtop_setHandler(SIX_HANDLER_SFX);
       // call sixtop
       sixtop_request(
          IANA_6TOP_CMD_ADD,
          &neighbor,
          cellUsage/SFX_TARGET-numberOfCells+1
       );
   } else {
     if (cellUsage/numberOfCells<SFX_DELETE_THRESHOLD){
         if (numberOfCells>1){
#ifdef SFX_DEBUG
             printf("mote %d remove one\n",idmanager_getMyID(ADDR_16B)->addr_16b[1]);
#endif
             sixtop_setHandler(SIX_HANDLER_SFX);
             // call sixtop
             sixtop_request(
                IANA_6TOP_CMD_DELETE,
                &neighbor,
                1
             );
         } else {
           // at least one dedicate slot for sixtop
         }
     } else {
        // nothing happens
     }
   }
}