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
#include "IEEE802154_security.h"
#include "openserial.h"

//=========================== definition =====================================

#define SF0_ID                            0
#define SF0THRESHOLD                      2
#define SF0_QUERY_MAX_FACTOR              6 // 10 => 1/10
#define SF0_QUERY_MIN_FACTOR              2 // 3 =>  1/3

#define SF0_QUERY_PERIOD              20000 // miliseconds
#define SF0_TRAFFICCONTROL_TIMEOUT    10000 // miliseconds
#define SF0_BANDWIDTHESTIMATE_TIMEOUT 60000 // miliseconds
#define SF0_PROBEPARENT_PERIOD        60000 // miliseconds
#define SF0_HOUSEKEEPING_PERIOD       60000 // miliseconds

//#define SF0_QUERY_ACTION_KA
#define SF0_QUERY_ACTION_6PQUERY

//=========================== variables =======================================

sf0_vars_t sf0_vars;

//=========================== prototypes ======================================

void sf0_bandwidthEstimate_task(open_addr_t* neighbor);
void sf0_probeParentBySendingKA(void);
// sixtop callback 
uint16_t sf0_getMetadata(void);
metadata_t sf0_translateMetadata(void);
void sf0_handleRCError(uint8_t code, open_addr_t* neighbor);

void sf0_6pQuery_timer_cb(opentimers_id_t id);
void sf0_trafficControl_timer_cb(opentimers_id_t id);
void sf0_housekeeping_timer_cb(opentimers_id_t id);

void task_sf0_6pQuery_timer_fired(void);
void task_sf0_housekeeping_timer_fired(void);

//=========================== public ==========================================

void sf0_init(void) {
    // sfcontrol
    open_addr_t     temp_neighbor;
    // sfcontrol
    memset(&sf0_vars,0,sizeof(sf0_vars_t));
    sf0_vars.numAppPacketsPerSlotFrame = 0;
    // sfcontrol 
    sf0_vars.sf_control_slotoffset = sf0_hashFunction(256*idmanager_getMyID(ADDR_64B)->addr_64b[6]+idmanager_getMyID(ADDR_64B)->addr_64b[7]);
    sf0_vars.sf_query_factor       = SF0_QUERY_MAX_FACTOR;
    
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
    
    sf0_vars.housekeeping_timer   = opentimers_create();
    opentimers_scheduleIn(
        sf0_vars.housekeeping_timer, 
        SF0_HOUSEKEEPING_PERIOD,
        TIME_MS, 
        TIMER_PERIODIC, 
        sf0_housekeeping_timer_cb
    ); 
    // sfcontrol 
    sixtop_setSFcallback(sf0_getsfid,sf0_getMetadata,sf0_translateMetadata,sf0_handleRCError);
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
    return sf0_hashFunction(256*idmanager_getMyID(ADDR_64B)->addr_64b[6]+idmanager_getMyID(ADDR_64B)->addr_64b[7]);
}

bool sf0_isTrafficControlled(void){
    return sf0_vars.sf_controlSlot_reserved;
}

uint16_t  sf0_hashFunction(uint16_t functionInput){
#ifdef FASTSIM
    return 4 + ((functionInput*10)%(SLOTFRAME_LENGTH-NUMSERIALRX-SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS));
#else
    return 4 + (functionInput%(SLOTFRAME_LENGTH-NUMSERIALRX-SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS));
#endif
}
// sf control

uint16_t sf0_getMetadata(void){
    return SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
}

metadata_t sf0_translateMetadata(void){
    return METADATA_TYPE_FRAMEID;
}

void sf0_handleRCError(uint8_t code, open_addr_t* neighbor){
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
    
    if (code==IANA_6TOP_RC_NORES){
        // mark the neighbor as no resource
        neighbors_setNeighborNoResource(neighbor);
    }
}

// sfcontrol

void sf0_6pQuery_notifyReceived(uint16_t query_factor, open_addr_t* neighbor){
    if (openrandom_get16b() < 0xffff/query_factor){
#ifdef SF0_QUERY_ACTION_6PQUERY
        sf0_bandwidthEstimate_task(neighbor);
#endif
#ifdef SF0_QUERY_ACTION_KA
        sf0_probeParentBySendingKA();
#endif
    }
}

bool sf0_getControlslotConflictWithParent(void){
    return sf0_vars.controlCellConflictWithParent;
}

void sf0_setControlslotConflictWithParent(bool isConflicted){
    sf0_vars.controlCellConflictWithParent = isConflicted;
}

void sf0_6pQuery_timer_cb(opentimers_id_t id){
    scheduler_push_task(task_sf0_6pQuery_timer_fired,TASKPRIO_SF0);
}

void task_sf0_6pQuery_timer_fired(void){
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
    
    
    if (schedule_getNumberOfFreeEntries()==0){
        // stop send 6pQuery if I have no free entries for slot
        return;
    }
    
   
    // do not send 6p query if I have the default DAG rank
    if (sf0_vars.sf_isBusySendingQuery==TRUE){
        
        // stop here
        return;
    }
    
    if (
        idmanager_getIsDAGroot()                  == FALSE &&
        schedule_getNumOfSlotsByType(CELLTYPE_TX) == 0  
    ) {
        // don't send 6p query until getting a tx cell to parent
        return;
    }
    
    memset(&temp_neighbor,0,sizeof(temp_neighbor));
    temp_neighbor.type        = ADDR_ANYCAST;
    
    if (openqueue_macGetPacketCreatedBy(COMPONENT_SIXTOP_RES,&temp_neighbor)!=NULL){
        // previous sixtop transcation is still going on
        // skip 6p query this time
      
        // turn on traffic control helping sixtop transcation
        sf0_vars.sf_controlSlot_reserved = TRUE;
        opentimers_scheduleIn(
            sf0_vars.trafficcontrol_timer, 
            SF0_TRAFFICCONTROL_TIMEOUT,
            TIME_MS, 
            TIMER_ONESHOT, 
            sf0_trafficControl_timer_cb
        );
        
        return;
    }
    
    memset(&temp_neighbor,0,sizeof(temp_neighbor));
    temp_neighbor.type        = ADDR_16B;
    temp_neighbor.addr_16b[0] = 0xff;
    temp_neighbor.addr_16b[1] = 0xff;
    
    if (sf0_vars.received6Ppreviously==FALSE){
        if (sf0_vars.sf_query_factor>SF0_QUERY_MIN_FACTOR && (schedule_getNumOfSlotsByType(CELLTYPE_RX)>0)){
            // change query factor when I have a rx cell in my schedule first
            sf0_vars.sf_query_factor--;
        }
    } else {
        if (sf0_vars.sf_query_factor<SF0_QUERY_MAX_FACTOR){
            sf0_vars.sf_query_factor++;
        }
    }

    outcome = sixtop_request(
        IANA_6TOP_CMD_QUERY,                                            // code
        &temp_neighbor,                                                 // neighbor
        1,                                                              // number cells
        LINKOPTIONS_TX | LINKOPTIONS_RX | LINKOPTIONS_SHARED,           // cellOptions
        NULL,                                                           // celllist to add    (not used)
        NULL,                                                           // celllist to delete (not used)
        SF0_ID,                                                         // sfid
        sf0_vars.sf_query_factor,                                       // query offset (reuse list command offset parameter)
        0                                                               // list command maximum celllist (not used)
    );
    if (outcome == E_SUCCESS){
        
        sf0_vars.received6Ppreviously = FALSE;
        
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

void sf0_setReceived6Ppreviously(bool received){
    sf0_vars.received6Ppreviously = received;
}

void sf0_housekeeping_timer_cb(opentimers_id_t id){
    scheduler_push_task(task_sf0_housekeeping_timer_fired,TASKPRIO_SF0);
}

void task_sf0_housekeeping_timer_fired(void){
    cellInfo_ht    celllist_delete[CELLLIST_MAX_LEN];
    open_addr_t    neighbor;
    // do not reserve cells if I'm a DAGroot
    if (idmanager_getIsDAGroot()){
        return;
    }
    
    if (sf0_vars.backoff>0){
        sf0_vars.backoff -= 1;
        return;
    }
    
    if (schedule_getNonParentNeighborWithTxCell(&neighbor)==FALSE){
        return;
    }

    if (sf0_candidateRemoveCellList(celllist_delete,&neighbor,1)==FALSE){
        // failed to get cell list to add
        return;
    }
    
    /**  do nothing for debugging, so I can see the wrong scheduled tx cell
    */
    // remove the tx cell directly
//    schedule_removeActiveSlot(celllist_delete[0].slotoffset,&neighbor);
    
}

// sfcontrol
//=========================== private =========================================

void sf0_bandwidthEstimate_task(open_addr_t* neighbor){
    uint8_t        bw_outgoing;
    cellInfo_ht    celllist_add[CELLLIST_MAX_LEN];
    
    // do not reserve cells if I'm a DAGroot
    if (idmanager_getIsDAGroot()){
        return;
    }
    
    if (sf0_vars.backoff>0){
        sf0_vars.backoff -= 1;
        return;
    }

    if (icmpv6rpl_isPreferredParent(neighbor)==FALSE){
        return;
    }

    bw_outgoing = schedule_getCellsCounts(SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE,CELLTYPE_TX,neighbor);
    
    if (bw_outgoing==0){
        if (sf0_candidateAddCellList(celllist_add,1)==FALSE){
            // failed to get cell list to add
            return;
        }
        sixtop_request(
            IANA_6TOP_CMD_ADD,                  // code
            neighbor,                           // neighbor
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

void sf0_probeParentBySendingKA(void){
    open_addr_t    kaNeighAddr;
    bool           foundNeighbor;
    OpenQueueEntry_t* kaPkt;
    
    if (ieee154e_isSynch()==FALSE) {
        // I'm not sync'ed
      
        // delete packets genereted by this module (EB and KA) from openqueue
        openqueue_removeAllCreatedBy(COMPONENT_SIXTOP_RES);
      
        // I'm not busy sending a KA
        sf0_vars.busySendingKA = FALSE;
      
        // stop here
        return;
    }
   
    if (sf0_vars.busySendingKA==TRUE) {
        // don't proceed if I'm still sending a KA
        return;
    }
    
    // do not reserve cells if I'm a DAGroot
    if (idmanager_getIsDAGroot()){
        return;
    }

    // get preferred parent
    foundNeighbor = icmpv6rpl_getPreferredParentEui64(&kaNeighAddr);
    if (foundNeighbor==FALSE) {
        return;
    }
    
    // if I get here, I will send a KA
   
    // get a free packet buffer
    kaPkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP);
    if (kaPkt==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)1,
            (errorparameter_t)0
        );
        return;
    }
   
    // declare ownership over that packet
    kaPkt->creator = COMPONENT_SIXTOP_RES;
    kaPkt->owner   = COMPONENT_SIXTOP_RES;
   
    // some l2 information about this packet
    memcpy(&(kaPkt->l2_nextORpreviousHop),&kaNeighAddr,sizeof(open_addr_t));
   
    // set l2-security attributes
    kaPkt->l2_securityLevel   = IEEE802154_SECURITY_LEVEL; // do not exchange KAs with 
    kaPkt->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
    kaPkt->l2_keyIndex        = IEEE802154_security_getDataKeyIndex();
    kaPkt->l2_payloadIEpresent= FALSE;

    // put in queue for MAC to handle
    sixtop_send(kaPkt);
    
    // I'm now busy sending a KA
    sf0_vars.busySendingKA = TRUE;
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
