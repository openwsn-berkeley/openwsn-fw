#include "opendefs.h"
#include "msf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "openapps.h"
#include "openrandom.h"
#include "idmanager.h"
#include "icmpv6rpl.h"
#include "IEEE802154E.h"

//=========================== definition =====================================

//=========================== variables =======================================

msf_vars_t msf_vars;

//=========================== prototypes ======================================

// sixtop callback 
uint16_t msf_getMetadata(void);
metadata_t msf_translateMetadata(void);
void msf_handleRCError(uint8_t code, open_addr_t* address);

void msf_timer_housekeeping_cb(opentimers_id_t id);
void msf_timer_housekeeping_task(void);

void msf_timer_waitretry_cb(opentimers_id_t id);

void msf_timer_clear_task(void);
// msf private
void msf_trigger6pAdd(void);
void msf_trigger6pDelete(void);
void msf_housekeeping(void);

//=========================== public ==========================================

void msf_init(void) {
    memset(&msf_vars,0,sizeof(msf_vars_t));
    msf_vars.numAppPacketsPerSlotFrame = 0;
    sixtop_setSFcallback(
        (sixtop_sf_getsfid)msf_getsfid,
        (sixtop_sf_getmetadata)msf_getMetadata,
        (sixtop_sf_translatemetadata)msf_translateMetadata,
        (sixtop_sf_handle_callback)msf_handleRCError
    );
    msf_vars.housekeepingTimerId = opentimers_create();
    opentimers_scheduleIn(
        msf_vars.housekeepingTimerId,
        872 +(openrandom_get16b()&0xff),
        TIME_MS,
        TIMER_ONESHOT,
        msf_timer_housekeeping_cb
    );
    msf_vars.waitretryTimerId    = opentimers_create();
}

// called by schedule
void    msf_updateCellsPassed(open_addr_t* neighbor){
  
    if (icmpv6rpl_isPreferredParent(neighbor)==FALSE){
        return;
    }
  
    msf_vars.numCellsPassed++;
    if (msf_vars.numCellsPassed == MAX_NUMCELLS){
        if (msf_vars.numCellsUsed > LIM_NUMCELLSUSED_HIGH){
            msf_trigger6pAdd();
        }
        if (msf_vars.numCellsUsed < LIM_NUMCELLSUSED_LOW){
            msf_trigger6pDelete();
        }
        msf_vars.numCellsPassed = 0;
        msf_vars.numCellsUsed   = 0;
    }
}

void    msf_updateCellsUsed(open_addr_t* neighbor){
  
    if (icmpv6rpl_isPreferredParent(neighbor)==FALSE){
        return;
    }
    
    msf_vars.numCellsUsed++;
}

void    msf_trigger6pClear(open_addr_t* neighbor){
        
    if (schedule_hasDedicatedCellToNeighbor(neighbor)>0){
        sixtop_request(
            IANA_6TOP_CMD_CLEAR,                // code
            neighbor,                           // neighbor
            NUMCELLS_MSF,                       // number cells
            CELLOPTIONS_MSF,                    // cellOptions
            NULL,                               // celllist to add (not used)
            NULL,                               // celllist to delete (not used)
            IANA_6TISCH_SFID_MSF,               // sfid
            0,                                  // list command offset (not used)
            0                                   // list command maximum celllist (not used)
        );
    }
}
//=========================== callback =========================================

uint8_t msf_getsfid(void){
    return IANA_6TISCH_SFID_MSF;
}

uint16_t msf_getMetadata(void){
    return SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
}

metadata_t msf_translateMetadata(void){
    return METADATA_TYPE_FRAMEID;
}

void msf_handleRCError(uint8_t code, open_addr_t* address){
    uint16_t waitDuration;
    
    if (
        code==IANA_6TOP_RC_RESET        ||
        code==IANA_6TOP_RC_LOCKED
    ){
        // waitretry
        msf_vars.waitretry = TRUE;
        waitDuration = WAITDURATION_MIN+openrandom_get16b()%WAITDURATION_RANDOM_RANGE;
        opentimers_scheduleIn(
            msf_vars.waitretryTimerId,
            waitDuration,
            TIME_MS,
            TIMER_ONESHOT,
            msf_timer_waitretry_cb
        );
    }
    
    if (
        code==IANA_6TOP_RC_ERROR        ||
        code==IANA_6TOP_RC_VER_ERR      ||
        code==IANA_6TOP_RC_SFID_ERR
    ){
        // quarantine
    }
    
    if (
        code==IANA_6TOP_RC_SEQNUM_ERR   ||
        code==IANA_6TOP_RC_CELLLIST_ERR
    ){
        // clear
        scheduler_push_task(msf_timer_clear_task,TASKPRIO_MSF);
    }
    
    if (code==IANA_6TOP_RC_BUSY){
        // mark neighbor f6NORES
        neighbors_setNeighborNoResource(address);
    }
}

void msf_timer_waitretry_cb(opentimers_id_t id){
    msf_vars.waitretry = FALSE;
}

void msf_timer_housekeeping_cb(opentimers_id_t id){
    scheduler_push_task(msf_timer_housekeeping_task,TASKPRIO_MSF);
    // update the period
    opentimers_scheduleIn(
        msf_vars.housekeepingTimerId,
        872 +(openrandom_get16b()&0xff),
        TIME_MS,
        TIMER_ONESHOT,
        msf_timer_housekeeping_cb
    );
}
                        
void msf_timer_housekeeping_task(void){
    msf_vars.housekeepingTimerCounter = (msf_vars.housekeepingTimerCounter+1)%HOUSEKEEPING_PERIOD;
    switch (msf_vars.housekeepingTimerCounter) {
    case 0:
        msf_housekeeping();
        break;
    default:
        break;
    }
}

void msf_timer_clear_task(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    
    sixtop_request(
        IANA_6TOP_CMD_CLEAR,                // code
        &neighbor,                          // neighbor
        NUMCELLS_MSF,                       // number cells
        CELLOPTIONS_MSF,                    // cellOptions
        NULL,                               // celllist to add (not used)
        NULL,                               // celllist to delete (not used)
        IANA_6TISCH_SFID_MSF,               // sfid
        0,                                  // list command offset (not used)
        0                                   // list command maximum celllist (not used)
    );
}

//=========================== private =========================================

void msf_trigger6pAdd(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    
    if (ieee154e_isSynch()==FALSE) {
        return;
    }
    
    if (msf_vars.waitretry){
        return;
    }
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    
    if (msf_candidateAddCellList(celllist_add,NUMCELLS_MSF)==FALSE){
        // failed to get cell list to add
        return;
    }
    
    sixtop_request(
        IANA_6TOP_CMD_ADD,                  // code
        &neighbor,                          // neighbor
        NUMCELLS_MSF,                       // number cells
        CELLOPTIONS_MSF,                    // cellOptions
        celllist_add,                       // celllist to add
        NULL,                               // celllist to delete (not used)
        IANA_6TISCH_SFID_MSF,               // sfid
        0,                                  // list command offset (not used)
        0                                   // list command maximum celllist (not used)
    );
}

void msf_trigger6pDelete(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_delete[CELLLIST_MAX_LEN];
    
    if (ieee154e_isSynch()==FALSE) {
        return;
    }
    
    if (msf_vars.waitretry){
        return;
    }
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    
    if (schedule_getNumberOfDedicatedCells(&neighbor)<=1){
        // at least one dedicated cell presents
        return;
    }
    
    if (msf_candidateRemoveCellList(celllist_delete,&neighbor,NUMCELLS_MSF)==FALSE){
        // failed to get cell list to delete
        return;
    }
    sixtop_request(
        IANA_6TOP_CMD_DELETE,   // code
        &neighbor,              // neighbor
        NUMCELLS_MSF,           // number cells
        CELLOPTIONS_MSF,        // cellOptions
        NULL,                   // celllist to add (not used)
        celllist_delete,        // celllist to delete
        IANA_6TISCH_SFID_MSF,   // sfid
        0,                      // list command offset (not used)
        0                       // list command maximum celllist (not used)
    );
}

void msf_appPktPeriod(uint8_t numAppPacketsPerSlotFrame){
    msf_vars.numAppPacketsPerSlotFrame = numAppPacketsPerSlotFrame;
}

bool msf_candidateAddCellList(
      cellInfo_ht* cellList,
      uint8_t      requiredCells
   ){
    uint8_t i;
    frameLength_t slotoffset;
    uint8_t numCandCells;
    
    memset(cellList,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    numCandCells=0;
    for(i=0;i<CELLLIST_MAX_LEN;i++){
        slotoffset = openrandom_get16b()%schedule_getFrameLength();
        if(schedule_isSlotOffsetAvailable(slotoffset)==TRUE){
            cellList[numCandCells].slotoffset       = slotoffset;
            cellList[numCandCells].channeloffset    = openrandom_get16b()&0x0F;
            cellList[numCandCells].isUsed           = TRUE;
            numCandCells++;
        }
    }
   
    if (numCandCells<requiredCells || requiredCells==0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

bool msf_candidateRemoveCellList(
      cellInfo_ht* cellList,
      open_addr_t* neighbor,
      uint8_t      requiredCells
   ){
   uint8_t              i;
   uint8_t              numCandCells;
   slotinfo_element_t   info;
   
   memset(cellList,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
   numCandCells    = 0;
   for(i=0;i<schedule_getFrameLength();i++){
      schedule_getSlotInfo(i,neighbor,&info);
      if(info.link_type == CELLTYPE_TX){
         cellList[numCandCells].slotoffset       = i;
         cellList[numCandCells].channeloffset    = info.channelOffset;
         cellList[numCandCells].isUsed           = TRUE;
         numCandCells++;
         if (numCandCells==CELLLIST_MAX_LEN){
            break;
         }
      }
   }
   
   if(numCandCells<requiredCells){
      return FALSE;
   }else{
      return TRUE;
   }
}

void msf_housekeeping(void){
    
    open_addr_t    parentNeighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    cellInfo_ht    celllist_delete[CELLLIST_MAX_LEN];
    
    if (ieee154e_isSynch()==FALSE) {
        return;
    }
    
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&parentNeighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    if (schedule_getNumberOfDedicatedCells(&parentNeighbor)==0){
        msf_trigger6pAdd();
        return;
    }
    
    if (msf_vars.waitretry){
        return;
    }
    
    if (schedule_isNumTxWrapped(&parentNeighbor)==FALSE){
        return;
    }
    
    memset(celllist_delete, 0, CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    if (schedule_getCellsToBeRelocated(&parentNeighbor, celllist_delete)){
        if (msf_candidateAddCellList(celllist_add,NUMCELLS_MSF)==FALSE){
            // failed to get cell list to add
            return;
        }
        sixtop_request(
            IANA_6TOP_CMD_RELOCATE,   // code
            &parentNeighbor,          // neighbor
            NUMCELLS_MSF,             // number cells
            CELLOPTIONS_MSF,          // cellOptions
            celllist_add,             // celllist to add
            celllist_delete,          // celllist to delete
            IANA_6TISCH_SFID_MSF,     // sfid
            0,                        // list command offset (not used)
            0                         // list command maximum celllist (not used)
        );
    }
}