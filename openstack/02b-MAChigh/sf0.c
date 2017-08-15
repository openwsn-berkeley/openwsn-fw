#include "opendefs.h"
#include "sf0.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "idmanager.h"
#include "openapps.h"
#include "openrandom.h"
#include "openqueue.h"
#include "IEEE802154E.h"

//=========================== definition =====================================

#define SF0_ID                            0
#define SF0THRESHOLD                      2
#define SF0_QUERY_RANGE                  10 // slots

#define SF0_QUERY_PERIOD              20000 // miliseconds
#define SF0_TRAFFICCONTROL_TIMEOUT    10000 // miliseconds

//=========================== variables =======================================

sf0_vars_t sf0_vars;

//=========================== prototypes ======================================

void sf0_bandwidthEstimate_task(void);
// sixtop callback 
uint16_t sf0_getMetadata(void);
metadata_t sf0_translateMetadata(void);
void sf0_handleRCError(uint8_t code);

void sf0_6pQuery_timer_cb(opentimers_id_t id);
void sf0_trafficControl_timer_cb(opentimers_id_t id);

//=========================== public ==========================================

void sf0_init(void) {
    // sfcontrol
    open_addr_t     temp_neighbor;
    // sfcontrol
    memset(&sf0_vars,0,sizeof(sf0_vars_t));
    sf0_vars.numAppPacketsPerSlotFrame = 0;
    // sfcontrol 
    sf0_vars.sf_control_slotoffset = sf0_hashFunction(idmanager_getMyID(ADDR_64B)->addr_64b[7]);
    
    memset(&temp_neighbor,0,sizeof(temp_neighbor));
    temp_neighbor.type             = ADDR_ANYCAST;
    schedule_addActiveSlot(
        sf0_vars.sf_control_slotoffset,     // slot offset
        CELLTYPE_TXRX,                      // type of slot
        TRUE,                               // shared?
        SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET,    // channel offset
        &temp_neighbor                      // neighbor
    );
    
    // start sf0 6pquery timer
    sf0_vars.query_timer          = opentimers_create();
    sf0_vars.trafficcontrol_timer = opentimers_create();
    opentimers_scheduleIn(
        sf0_vars.query_timer, 
        SF0_QUERY_PERIOD,
        TIME_MS, 
        TIMER_PERIODIC, 
        sf0_6pQuery_timer_cb
    );
    // sfcontrol 
    sixtop_setSFcallback(sf0_getsfid,sf0_getMetadata,sf0_translateMetadata,sf0_handleRCError);
}

// this function is called once per slotframe. 
void sf0_notifyNewSlotframe(void) {
   scheduler_push_task(sf0_bandwidthEstimate_task,TASKPRIO_SF0);
}

void sf0_setBackoff(uint8_t value){
    sf0_vars.backoff = value;
}

//=========================== callback =========================================

uint8_t sf0_getsfid(void){
    return SF0_ID;
}

// sf control
uint16_t  sf0_getControlslotoffset(void){
    return sf0_vars.sf_control_slotoffset;
}

bool sf0_isTrafficControlled(void){
    return sf0_vars.sf_controlSlot_reserved;
}

uint16_t  sf0_hashFunction(uint16_t functionInput){
    return 4 + (functionInput%(SLOTFRAME_LENGTH-NUMSERIALRX-SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS));
}
// sf control

uint16_t sf0_getMetadata(void){
    return SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
}

metadata_t sf0_translateMetadata(void){
    return METADATA_TYPE_FRAMEID;
}

void sf0_handleRCError(uint8_t code){
    if (code==IANA_6TOP_RC_BUSY){
        // disable sf0 for [0...2^4] slotframe long time
        sf0_setBackoff(openrandom_get16b()%(1<<4));
    }
    
    if (code==IANA_6TOP_RC_RESET){
        // TBD: the neighbor can't statisfy the 6p request with given cells, call sf0 to make a decision 
        // (e.g. issue another 6p request with different cell list)
    }
    
    if (code==IANA_6TOP_RC_ERROR){
        // TBD: the neighbor can't statisfy the 6p request, call sf0 to make a decision
    }
    
    if (code==IANA_6TOP_RC_VER_ERR){
        // TBD: the 6p verion does not match
    }
    
    if (code==IANA_6TOP_RC_SFID_ERR){
        // TBD: the sfId does not match
    }
}

// sfcontrol

void sf0_6pQuery_notifyReceived(uint16_t queryOffset, open_addr_t* neighbor){
    if (queryOffset<SLOTFRAME_LENGTH-SF0_QUERY_RANGE){
        if (
            sf0_vars.sf_control_slotoffset < (queryOffset+SF0_QUERY_RANGE)%SLOTFRAME_LENGTH && \
            sf0_vars.sf_control_slotoffset > (queryOffset)%SLOTFRAME_LENGTH
        ){
            sf0_bandwidthEstimate_task();
        }
    } else {
        if (
            sf0_vars.sf_control_slotoffset < (queryOffset+SF0_QUERY_RANGE)%SLOTFRAME_LENGTH || \
            sf0_vars.sf_control_slotoffset > (queryOffset)%SLOTFRAME_LENGTH
        ){
            sf0_bandwidthEstimate_task();
        }
    }
}

void sf0_6pQuery_timer_cb(opentimers_id_t id){
    open_addr_t     temp_neighbor;
    owerror_t       outcome;
    
    // stop if I'm not sync'ed or did not acquire a DAGrank
    if (
        ieee154e_isSynch()==FALSE               || \
        icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK
    ) {
      
        openqueue_removeAllCreatedBy(COMPONENT_SIXTOP_RES);
        sf0_vars.sf_isBusySendingQuery = FALSE;
        // stop here
        return;
    }
   
    // do not send DIO if I have the default DAG rank
    if (sf0_vars.sf_isBusySendingQuery==TRUE){
        
        // stop here
        return;
    }
    
    if (
        idmanager_getIsDAGroot() == FALSE &&
        schedule_getNumOfSlotsByType(CELLTYPE_TX)==0
    ){  
        // do not send query if my schedule is no ready yet
        return;
    }
    
    memset(&temp_neighbor,0,sizeof(temp_neighbor));
    temp_neighbor.type        = ADDR_16B;
    temp_neighbor.addr_16b[0] = 0xff;
    temp_neighbor.addr_16b[1] = 0xff;
    
    outcome = sixtop_request(
        IANA_6TOP_CMD_QUERY,                                            // code
        &temp_neighbor,                                                 // neighbor
        1,                                                              // number cells
        LINKOPTIONS_TX | LINKOPTIONS_RX | LINKOPTIONS_SHARED,           // cellOptions
        NULL,                                                           // celllist to add    (not used)
        NULL,                                                           // celllist to delete (not used)
        SF0_ID,                                                         // sfid
        (sf0_vars.sf_query_offset+SF0_QUERY_RANGE)%SLOTFRAME_LENGTH,    // query offset (reuse list command offset parameter)
        0                                                               // list command maximum celllist (not used)
    );
    if (outcome == E_SUCCESS){
        // update query offset
        sf0_vars.sf_query_offset = (sf0_vars.sf_query_offset+SF0_QUERY_RANGE)%SLOTFRAME_LENGTH;
        
        sf0_vars.sf_isBusySendingQuery = TRUE;
        
        sf0_vars.sf_controlSlot_reserved = TRUE;
        opentimers_scheduleIn(
            sf0_vars.trafficcontrol_timer, 
            SF0_TRAFFICCONTROL_TIMEOUT,
            TIME_MS, 
            TIMER_ONESHOT, 
            sf0_trafficControl_timer_cb
        );
    }
}

void sf0_trafficControl_timer_cb(opentimers_id_t id){
    sf0_vars.sf_controlSlot_reserved = FALSE;
}

void sf0_6pQuery_sendDone(void){
    sf0_vars.sf_isBusySendingQuery = FALSE;
}

// sfcontrol
//=========================== private =========================================

void sf0_bandwidthEstimate_task(void){
    open_addr_t    neighbor;
    bool           foundNeighbor;
    int8_t         bw_outgoing;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    
    // do not reserve cells if I'm a DAGroot
    if (idmanager_getIsDAGroot()){
        return;
    }
    
    if (sf0_vars.backoff>0){
        sf0_vars.backoff -= 1;
        return;
    }
    
    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&neighbor);
    if (foundNeighbor==FALSE) {
        return;
    }

    bw_outgoing = schedule_getNumOfSlotsByType(CELLTYPE_TX);
    
    if (bw_outgoing==0){
        if (sf0_candidateAddCellList(celllist_add,1)==FALSE){
            // failed to get cell list to add
            return;
        }
        sixtop_request(
            IANA_6TOP_CMD_ADD,                  // code
            &neighbor,                          // neighbor
            1,                                  // number cells
            LINKOPTIONS_TX,                     // cellOptions
            celllist_add,                       // celllist to add
            NULL,                               // celllist to delete (not used)
            SF0_ID,                             // sfid
            0,                                  // list command offset (not used)
            0                                   // list command maximum celllist (not used)
        );
    }
}

void sf0_appPktPeriod(uint8_t numAppPacketsPerSlotFrame){
    sf0_vars.numAppPacketsPerSlotFrame = numAppPacketsPerSlotFrame;
}

bool sf0_candidateAddCellList(
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

bool sf0_candidateRemoveCellList(
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
