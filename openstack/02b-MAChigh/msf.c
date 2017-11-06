#include "opendefs.h"
#include "msf.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "idmanager.h"
#include "openapps.h"
#include "openrandom.h"

//=========================== definition =====================================

#define IANA_6TISCH_SFID_MSF    0
#define SF0THRESHOLD            2

//=========================== variables =======================================

msf_vars_t msf_vars;

//=========================== prototypes ======================================

void msf_bandwidthEstimate_task(void);
// sixtop callback 
uint16_t msf_getMetadata(void);
metadata_t msf_translateMetadata(void);
void msf_handleRCError(uint8_t code);

//=========================== public ==========================================

void msf_init(void) {
    memset(&msf_vars,0,sizeof(msf_vars_t));
    msf_vars.numAppPacketsPerSlotFrame = 0;
    sixtop_setSFcallback(msf_getsfid,msf_getMetadata,msf_translateMetadata,msf_handleRCError);
}

// this function is called once per slotframe. 
void msf_notifyNewSlotframe(void) {
   scheduler_push_task(msf_bandwidthEstimate_task,TASKPRIO_MSF);
}

void msf_setBackoff(uint8_t value){
    msf_vars.backoff = value;
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

void msf_bandwidthEstimate_task(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    int8_t         bw_outgoing;
    int8_t         bw_incoming;
    int8_t         bw_self;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    cellInfo_ht    celllist_delete[CELLLIST_MAX_LEN];
    
    // do not reserve cells if I'm a DAGroot
    if (idmanager_getIsDAGroot()){
        return;
    }
    
    if (msf_vars.backoff>0){
        msf_vars.backoff -= 1;
        return;
    }
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }
    
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
    bw_self = msf_vars.numAppPacketsPerSlotFrame;
    
    // In msf, scheduledCells = bw_outgoing
    //         requiredCells  = bw_incoming + bw_self
    // when scheduledCells<requiredCells, add one or more cell
    
    if (bw_outgoing <= bw_incoming+bw_self){
        if (msf_candidateAddCellList(celllist_add,bw_incoming+bw_self-bw_outgoing+1)==FALSE){
            // failed to get cell list to add
            return;
        }
        sixtop_request(
            IANA_6TOP_CMD_ADD,                  // code
            &neighbor,                          // neighbor
            bw_incoming+bw_self-bw_outgoing+1,  // number cells
            LINKOPTIONS_TX,                     // cellOptions
            celllist_add,                       // celllist to add
            NULL,                               // celllist to delete (not used)
            IANA_6TISCH_SFID_MSF,               // sfid
            0,                                  // list command offset (not used)
            0                                   // list command maximum celllist (not used)
        );
    } else {
        // remove cell(s)
        if ( (bw_incoming+bw_self) < (bw_outgoing-SF0THRESHOLD)) {
            if (msf_candidateRemoveCellList(celllist_delete,&neighbor,SF0THRESHOLD)==FALSE){
                // failed to get cell list to delete
                return;
            }
            sixtop_request(
                IANA_6TOP_CMD_DELETE,   // code
                &neighbor,              // neighbor
                SF0THRESHOLD,           // number cells
                LINKOPTIONS_TX,         // cellOptions
                NULL,                   // celllist to add (not used)
                celllist_delete,        // celllist to delete
                IANA_6TISCH_SFID_MSF,   // sfid
                0,                      // list command offset (not used)
                0                       // list command maximum celllist (not used)
            );
        } else {
            // nothing to do
        }
    }
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
            cellList[numCandCells].channeloffset    = openrandom_get16b()%16;
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
