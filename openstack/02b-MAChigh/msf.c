#include "opendefs.h"
#include "msf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "openapps.h"
#include "openrandom.h"

//=========================== definition =====================================

//=========================== variables =======================================

msf_vars_t msf_vars;

//=========================== prototypes ======================================

// sixtop callback 
uint16_t msf_getMetadata(void);
metadata_t msf_translateMetadata(void);
void msf_handleRCError(uint8_t code);
// msf private
void msf_trigger6pAdd(void);
void msf_trigger6pDelete(void);

//=========================== public ==========================================

void msf_init(void) {
    memset(&msf_vars,0,sizeof(msf_vars_t));
    msf_vars.numAppPacketsPerSlotFrame = 0;
    sixtop_setSFcallback(msf_getsfid,msf_getMetadata,msf_translateMetadata,msf_handleRCError);
}

void msf_setBackoff(uint8_t value){
    msf_vars.backoff = value;
}

// called by schedule
void    msf_updateCellsPassed(void){
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

void    msf_updateCellsUsed(void){
    msf_vars.numCellsUsed++;
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

void msf_handleRCError(uint8_t code){
    if (code==IANA_6TOP_RC_BUSY){
        // disable msf for [0...2^4] slotframe long time
        msf_setBackoff(openrandom_get16b()%(1<<4));
    }
    
    if (code==IANA_6TOP_RC_RESET){
        // TBD: the neighbor can't statisfy the 6p request with given cells, call msf to make a decision 
        // (e.g. issue another 6p request with different cell list)
    }
    
    if (code==IANA_6TOP_RC_ERROR){
        // TBD: the neighbor can't statisfy the 6p request, call msf to make a decision
    }
    
    if (code==IANA_6TOP_RC_VER_ERR){
        // TBD: the 6p verion does not match
    }
    
    if (code==IANA_6TOP_RC_SFID_ERR){
        // TBD: the sfId does not match
    }
    
    if (code==IANA_6TOP_RC_SEQNUM_ERR){
        // TBD: the seqNum does not match
    }
}

//=========================== private =========================================


void msf_trigger6pAdd(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    
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
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    
    if (msf_candidateRemoveCellList(celllist_delete,&neighbor,1)==FALSE){
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
