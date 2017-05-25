#include "opendefs.h"
#include "sixtop.h"
#include "openserial.h"
#include "openqueue.h"
#include "neighbors.h"
#include "IEEE802154E.h"
#include "iphc.h"
#include "sf0.h"
#include "packetfunctions.h"
#include "openrandom.h"
#include "scheduler.h"
#include "opentimers.h"
#include "debugpins.h"
#include "leds.h"
#include "processIE.h"
#include "IEEE802154.h"
#include "IEEE802154_security.h"
#include "idmanager.h"
#include "schedule.h"

//=========================== define ==========================================

// in seconds: sixtop maintaince is called every 30 seconds
#define MAINTENANCE_PERIOD        30
// in miliseconds: the real EBPERIOD will be randomized chosen between {EBPERIOD-EBPERIOD_RANDOM_RANG, EBPERIOD+EBPERIOD_RANDOM_RANG}
#define EBPERIOD_RANDOM_RANG     500

//=========================== variables =======================================

sixtop_vars_t sixtop_vars;

//=========================== prototypes ======================================

// send internal
owerror_t     sixtop_send_internal(
   OpenQueueEntry_t*    msg,
   bool                 payloadIEPresent
);

// timer interrupt callbacks
void          sixtop_maintenance_timer_cb(opentimer_id_t id);
void          sixtop_timeout_timer_cb(opentimer_id_t id);
void          sixtop_sendingEb_timer_cb(opentimer_id_t id);

//=== EB/KA task

void          timer_sixtop_sendEb_fired(void);
void          timer_sixtop_management_fired(void);
void          sixtop_sendEB(void);
void          sixtop_sendKA(void);

//=== six2six task

void          timer_sixtop_six2six_timeout_fired(void);
void          sixtop_six2six_sendDone(
   OpenQueueEntry_t*    msg,
   owerror_t            error
);
bool          sixtop_processIEs(
   OpenQueueEntry_t*    pkt,
   uint16_t*            lenIE
);
void sixtop_notifyReceiveCommand(
   uint8_t              version, 
   uint8_t              commandId, 
   uint8_t              sfId,
   uint8_t              ptr,
   uint8_t              length,
   OpenQueueEntry_t*    pkt
);

uint8_t sixtop_getCelllist(
   uint8_t             frameID,
   open_addr_t*         neighbor,
   cellInfo_ht*         cellList
);

//=== helper functions

bool          sixtop_candidateAddCellList(
   uint8_t*             frameID,
   cellInfo_ht*         cellList,
   uint8_t              requiredCells
);
bool          sixtop_candidateRemoveCellList(
   uint8_t*             frameID,
   cellInfo_ht*         cellList,
   open_addr_t*         neighbor,
   uint8_t              requiredCells
);
void          sixtop_addCellsByState(
   uint8_t              slotframeID,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop,
   uint8_t              state
);
void          sixtop_removeCellsByState(
   uint8_t              slotframeID,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop
);
bool          sixtop_areAvailableCellsToBeScheduled(
    uint8_t              frameID, 
    uint8_t              numOfCells, 
    cellInfo_ht*         cellList
);
bool sixtop_areAvailableCellsToBeRemoved(      
    uint8_t      frameID, 
    uint8_t      numOfCells, 
    cellInfo_ht* cellList,
    open_addr_t* neighbor
);

//=========================== public ==========================================

void sixtop_init() {
    
    sixtop_vars.periodMaintenance  = 872 +(openrandom_get16b()&0xff);
    sixtop_vars.busySendingKA      = FALSE;
    sixtop_vars.busySendingEB      = FALSE;
    sixtop_vars.dsn                = 0;
    sixtop_vars.mgtTaskCounter     = 0;
    sixtop_vars.kaPeriod           = MAXKAPERIOD;
    sixtop_vars.ebPeriod           = EBPERIOD;
    sixtop_vars.isResponseEnabled  = TRUE;
    sixtop_vars.handler            = SIX_HANDLER_NONE;
    sixtop_vars.six2six_state      = SIX_STATE_IDLE;
    
    sixtop_vars.ebSendingTimerId   = opentimers_start(
        (sixtop_vars.ebPeriod-EBPERIOD_RANDOM_RANG+(openrandom_get16b()%(2*EBPERIOD_RANDOM_RANG))),
        TIMER_PERIODIC,
        TIME_MS,
        sixtop_sendingEb_timer_cb
    );
    
    sixtop_vars.maintenanceTimerId  = opentimers_start(
        sixtop_vars.periodMaintenance,
        TIMER_PERIODIC,
        TIME_MS,
        sixtop_maintenance_timer_cb
    );
    
    sixtop_vars.timeoutTimerId      = opentimers_start(
        SIX2SIX_TIMEOUT_MS,
        TIMER_ONESHOT,
        TIME_MS,
        sixtop_timeout_timer_cb
    );
}

void sixtop_setKaPeriod(uint16_t kaPeriod) {
   if(kaPeriod > MAXKAPERIOD) {
      sixtop_vars.kaPeriod = MAXKAPERIOD;
   } else {
      sixtop_vars.kaPeriod = kaPeriod;
   } 
}

void sixtop_setEBPeriod(uint8_t ebPeriod) {
    if(ebPeriod != 0) {
        // convert parameter to miliseconds
        sixtop_vars.ebPeriod = ebPeriod*1000;
    }
}

bool sixtop_setHandler(six2six_handler_t handler) {
    if (
        sixtop_vars.handler       == SIX_HANDLER_NONE &&
        sixtop_vars.six2six_state == SIX_STATE_IDLE
    ){
        sixtop_vars.handler = handler;
        return TRUE;
    } else {
        // another handler is using sixtop
        return FALSE;
        
    }
}

//======= scheduling

void sixtop_request(uint8_t code, open_addr_t* neighbor, uint8_t numCells){
    OpenQueueEntry_t* pkt;
    uint8_t           len;
    uint8_t           container;
    uint8_t           frameID;
    cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
    memset(cellList,0,sizeof(cellList));
   
    // filter parameters: handler, status and neighbor
    if(
        sixtop_vars.handler       == SIX_HANDLER_NONE ||
        sixtop_vars.six2six_state != SIX_STATE_IDLE   ||
        neighbor                  == NULL
    ){
        // parameters are wrong
        // DONOT change sixtop status for the new transaction
        return;
    }
      
    // new transaction parameter checking passed.
    
    // check whether free entries are enough for reserving more cells
    if (code==IANA_6TOP_CMD_ADD && schedule_getNumberOfFreeEntries() < numCells){
        // no enough free buffer for adding more cells, reset handler 
        sixtop_vars.handler = SIX_HANDLER_NONE;
        // print out error information 
        openserial_printError(
            COMPONENT_SIXTOP,ERR_SCHEDULE_OVERFLOWN,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return ;
    }

    // generate candidate cell list
    if (code == IANA_6TOP_CMD_ADD){
        if (sixtop_candidateAddCellList(&frameID,cellList,numCells)==FALSE){
              sixtop_vars.handler = SIX_HANDLER_NONE;
              return;
        }
    }
    if (code == IANA_6TOP_CMD_DELETE){
        if (sixtop_candidateRemoveCellList(&frameID,cellList,neighbor,numCells)==FALSE){
              sixtop_vars.handler = SIX_HANDLER_NONE;
              return;
        }
    }
    
    // container to be define by SF, currently equals to frameID
    frameID        = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
    container      = frameID;
    
    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
    if (pkt==NULL) {
        sixtop_vars.handler = SIX_HANDLER_NONE;
        openserial_printError(
            COMPONENT_SIXTOP_RES,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
   
    // update state
    sixtop_vars.six2six_state  = SIX_STATE_SENDING_REQUEST;
   
    // take ownership
    pkt->creator = COMPONENT_SIXTOP_RES;
    pkt->owner   = COMPONENT_SIXTOP_RES;
   
    memcpy(&(pkt->l2_nextORpreviousHop),neighbor,sizeof(open_addr_t));
   
    // create packet
    len  = 0;
    if (code == IANA_6TOP_CMD_ADD || code == IANA_6TOP_CMD_DELETE){
        len += processIE_prepend_sixCelllist(pkt,cellList);
        // reserve space for container
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
        // write header
        *((uint8_t*)(pkt->payload)) = container;
        len+=1;
        // reserve space for numCells
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
        // write header
        *((uint8_t*)(pkt->payload)) = numCells;
        len+=1;
    } else {
        // reserve space for container
        packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
        // write header
        *((uint8_t*)(pkt->payload)) = container;
        len+=1;
    }
    
    len += processIE_prepend_sixGeneralMessage(pkt,code);
    len += processIE_prepend_sixSubID(pkt);
    processIE_prepend_sixtopIE(pkt,len);
   
    // indicate IEs present
    pkt->l2_payloadIEpresent = TRUE;
   
    // send packet
    sixtop_send(pkt);
    
    //update states
    switch(code){
    case IANA_6TOP_CMD_ADD:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_ADDREQUEST_SENDDONE;
        break;
    case IANA_6TOP_CMD_DELETE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_DELETEREQUEST_SENDDONE;
        break;
    case IANA_6TOP_CMD_COUNT:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_COUNTREQUEST_SENDDONE;
        break;
    case IANA_6TOP_CMD_LIST:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_LISTREQUEST_SENDDONE;
        break;
    case IANA_6TOP_CMD_CLEAR:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_CLEARREQUEST_SENDDONE;
        break;
    }
}

void sixtop_addORremoveCellByInfo(uint8_t code,open_addr_t* neighbor,cellInfo_ht* cellInfo){
    OpenQueueEntry_t* pkt;
    uint8_t           len;
    uint8_t           frameID;
    uint8_t           container;
    cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
    memset(cellList,0,sizeof(cellList));
   
    // filter parameters: handler, status and neighbor
    if(
        sixtop_vars.handler       == SIX_HANDLER_NONE ||
        sixtop_vars.six2six_state != SIX_STATE_IDLE   ||
        neighbor                  == NULL
    ){
        // parameters are wrong
        // DONOT change sixtop status for the new transaction
        return;
    }
      
    // new transaction parameter checking passed.
   
    // set cell list (only first one is to be removed)
    frameID        = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
    container      = frameID;
    memcpy(&(cellList[0]),cellInfo,sizeof(cellList));
   
   
    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
    if(pkt==NULL) {
        sixtop_vars.handler = SIX_HANDLER_NONE;
        openserial_printError(
            COMPONENT_SIXTOP_RES,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
   
    // update state
    sixtop_vars.six2six_state = SIX_STATE_SENDING_REQUEST;
   
    // declare ownership over that packet
    pkt->creator = COMPONENT_SIXTOP_RES;
    pkt->owner   = COMPONENT_SIXTOP_RES;
      
    memcpy(&(pkt->l2_nextORpreviousHop),neighbor,sizeof(open_addr_t));
    
    len  = 0;
    len += processIE_prepend_sixCelllist(pkt,cellList);
    // reserve space for container
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    // write header
    *((uint8_t*)(pkt->payload)) = container;
    len+=1;
    // reserve space for numCells
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    // write header
    *((uint8_t*)(pkt->payload)) = 1;
    len+=1;
    
    len += processIE_prepend_sixGeneralMessage(pkt,code);
    len += processIE_prepend_sixSubID(pkt);
    processIE_prepend_sixtopIE(pkt,len);
   
    // indicate IEs present
    pkt->l2_payloadIEpresent = TRUE;
   
    // send packet
    sixtop_send(pkt);
   
    //update states
    switch(code){
    case IANA_6TOP_CMD_ADD:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_ADDREQUEST_SENDDONE;
        break;
    case IANA_6TOP_CMD_DELETE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_DELETEREQUEST_SENDDONE;
        break;
    }
}

//======= from upper layer

owerror_t sixtop_send(OpenQueueEntry_t *msg) {
   
   // set metadata
   msg->owner        = COMPONENT_SIXTOP;
   msg->l2_frameType = IEEE154_TYPE_DATA;

   // set l2-security attributes
   msg->l2_securityLevel   = IEEE802154_SECURITY_LEVEL;
   msg->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE; 
   msg->l2_keyIndex        = IEEE802154_SECURITY_K2_KEY_INDEX;

   if (msg->l2_payloadIEpresent == FALSE) {
      return sixtop_send_internal(
         msg,
         FALSE
      );
   } else {
      return sixtop_send_internal(
         msg,
         TRUE
      );
   }
}

//======= from lower layer

void task_sixtopNotifSendDone() {
   OpenQueueEntry_t* msg;
   
   // get recently-sent packet from openqueue
   msg = openqueue_sixtopGetSentPacket();
   if (msg==NULL) {
      openserial_printCritical(
         COMPONENT_SIXTOP,
         ERR_NO_SENT_PACKET,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }
   
   // take ownership
   msg->owner = COMPONENT_SIXTOP;
   
   // update neighbor statistics
   if (msg->l2_sendDoneError==E_SUCCESS) {
      neighbors_indicateTx(
         &(msg->l2_nextORpreviousHop),
         msg->l2_numTxAttempts,
         TRUE,
         &msg->l2_asn
      );
   } else {
      neighbors_indicateTx(
         &(msg->l2_nextORpreviousHop),
         msg->l2_numTxAttempts,
         FALSE,
         &msg->l2_asn
      );
   }
   
   // send the packet to where it belongs
   switch (msg->creator) {
      
      case COMPONENT_SIXTOP:
         if (msg->l2_frameType==IEEE154_TYPE_BEACON) {
            // this is a EB
            
            // not busy sending EB anymore
            sixtop_vars.busySendingEB = FALSE;
            opentimers_setPeriod(
                sixtop_vars.ebSendingTimerId,
                TIME_MS,
                (sixtop_vars.ebPeriod-EBPERIOD_RANDOM_RANG+(openrandom_get16b()%(2*EBPERIOD_RANDOM_RANG)))
            );
         } else {
            // this is a KA
            
            // not busy sending KA anymore
            sixtop_vars.busySendingKA = FALSE;
         }
         // discard packets
         openqueue_freePacketBuffer(msg);
         
         // restart a random timer
         sixtop_vars.periodMaintenance = 872+(openrandom_get16b()&0xff);
         opentimers_setPeriod(
            sixtop_vars.maintenanceTimerId,
            TIME_MS,
            sixtop_vars.periodMaintenance
         );
         break;
      
      case COMPONENT_SIXTOP_RES:
         sixtop_six2six_sendDone(msg,msg->l2_sendDoneError);
         break;
      
      default:
         // send the rest up the stack
         iphc_sendDone(msg,msg->l2_sendDoneError);
         break;
   }
}

void task_sixtopNotifReceive() {
    OpenQueueEntry_t* msg;
    uint16_t          lenIE;
    // get received packet from openqueue
    msg = openqueue_sixtopGetReceivedPacket();
    if (msg==NULL) {
        openserial_printCritical(
            COMPONENT_SIXTOP,
            ERR_NO_RECEIVED_PACKET,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
   
    // take ownership
    msg->owner = COMPONENT_SIXTOP;
   
    // process the header IEs
    lenIE=0;
    if(
        msg->l2_frameType              == IEEE154_TYPE_DATA  &&
        msg->l2_payloadIEpresent       == TRUE               &&
        sixtop_processIEs(msg, &lenIE) == FALSE
    ) {
        // free the packet's RAM memory
        openqueue_freePacketBuffer(msg);
        //log error
        return;
    }
   
    // toss the header IEs
    packetfunctions_tossHeader(msg,lenIE);
   
    // update neighbor statistics
    neighbors_indicateRx(
        &(msg->l2_nextORpreviousHop),
        msg->l1_rssi,
        &msg->l2_asn,
        msg->l2_joinPriorityPresent,
        msg->l2_joinPriority
    );
   
    // reset it to avoid race conditions with this var.
    msg->l2_joinPriorityPresent = FALSE; 
   
    // send the packet up the stack, if it qualifies
    switch (msg->l2_frameType) {
    case IEEE154_TYPE_BEACON:
    case IEEE154_TYPE_DATA:
    case IEEE154_TYPE_CMD:
        if (msg->length>0) {
            // send to upper layer
            iphc_receive(msg);
        } else {
            // free up the RAM
            openqueue_freePacketBuffer(msg);
        }
        break;
    case IEEE154_TYPE_ACK:
    default:
        // free the packet's RAM memory
        openqueue_freePacketBuffer(msg);
        // log the error
        openserial_printError(
            COMPONENT_SIXTOP,
            ERR_MSG_UNKNOWN_TYPE,
            (errorparameter_t)msg->l2_frameType,
            (errorparameter_t)0
        );
        break;
    }
}

//======= debugging

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_myDAGrank() {
   uint16_t output;
   
   output = 0;
   
   output = icmpv6rpl_getMyDAGrank();
   openserial_printStatus(STATUS_DAGRANK,(uint8_t*)&output,sizeof(uint16_t));
   return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_kaPeriod() {
   uint16_t output;
   
   output = sixtop_vars.kaPeriod;
   
   openserial_printStatus(
       STATUS_KAPERIOD,
       (uint8_t*)&output,
       sizeof(output)
   );
   return TRUE;
}

void sixtop_setIsResponseEnabled(bool isEnabled){
    sixtop_vars.isResponseEnabled = isEnabled;
}

//=========================== private =========================================

/**
\brief Transfer packet to MAC.

This function adds a IEEE802.15.4 header to the packet and leaves it the 
OpenQueue buffer. The very last thing it does is assigning this packet to the 
virtual component COMPONENT_SIXTOP_TO_IEEE802154E. Whenever it gets a change,
IEEE802154E will handle the packet.

\param[in] msg The packet to the transmitted
\param[in] payloadIEPresent Indicates wheter an Information Element is present in the
   packet.

\returns E_SUCCESS iff successful.
*/
owerror_t sixtop_send_internal(
   OpenQueueEntry_t* msg, 
   bool    payloadIEPresent) {

   // assign a number of retries
   if (
      packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))==TRUE
      ) {
      msg->l2_retriesLeft = 1;
   } else {
      msg->l2_retriesLeft = TXRETRIES + 1;
   }
   // record this packet's dsn (for matching the ACK)
   msg->l2_dsn = sixtop_vars.dsn++;
   // this is a new packet which I never attempted to send
   msg->l2_numTxAttempts = 0;
   // transmit with the default TX power
   msg->l1_txPower = TX_POWER;
   // add a IEEE802.15.4 header
   ieee802154_prependHeader(msg,
                            msg->l2_frameType,
                            payloadIEPresent,
                            msg->l2_dsn,
                            &(msg->l2_nextORpreviousHop)
                            );
   // change owner to IEEE802154E fetches it from queue
   msg->owner  = COMPONENT_SIXTOP_TO_IEEE802154E;
   return E_SUCCESS;
}

// timer interrupt callbacks
void sixtop_sendingEb_timer_cb(opentimer_id_t id){
   scheduler_push_task(timer_sixtop_sendEb_fired,TASKPRIO_SIXTOP);
}

void sixtop_maintenance_timer_cb(opentimer_id_t id) {
   scheduler_push_task(timer_sixtop_management_fired,TASKPRIO_SIXTOP_TIMEOUT);
}

void sixtop_timeout_timer_cb(opentimer_id_t id) {
   scheduler_push_task(timer_sixtop_six2six_timeout_fired,TASKPRIO_SIXTOP_TIMEOUT);
}

//======= EB/KA task

void timer_sixtop_sendEb_fired(){
    sixtop_sendEB();
}

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timer_sixtop_management_fired(void) {
   
   sixtop_vars.mgtTaskCounter = (sixtop_vars.mgtTaskCounter+1)%MAINTENANCE_PERIOD;
   
   switch (sixtop_vars.mgtTaskCounter) {
      case 0:
         // called every MAINTENANCE_PERIOD seconds
         neighbors_removeOld();
         schedule_housekeeping();
         break;
      default:
         // called every second, except once every MAINTENANCE_PERIOD seconds
         sixtop_sendKA();
         break;
   }
}

/**
\brief Send an EB.

This is one of the MAC management tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sixtop_sendEB() {
   OpenQueueEntry_t* eb;
   uint8_t len;
   
   len = 0;
   
   if ((ieee154e_isSynch()==FALSE) || (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK)){
      // I'm not sync'ed or I did not acquire a DAGrank
      
      // delete packets genereted by this module (EB and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm not busy sending an EB or KA
      sixtop_vars.busySendingEB = FALSE;
      sixtop_vars.busySendingKA = FALSE;
      
      // stop here
      return;
   }
   
   if (sixtop_vars.busySendingEB==TRUE) {
      // don't continue if I'm still sending a previous EB
      return;
   }
   
   // if I get here, I will send an EB
   
   // get a free packet buffer
   eb = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP);
   if (eb==NULL) {
      openserial_printError(COMPONENT_SIXTOP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   eb->creator = COMPONENT_SIXTOP;
   eb->owner   = COMPONENT_SIXTOP;
   
   // reserve space for EB-specific header
   // reserving for IEs.
   len += processIE_prependSlotframeLinkIE(eb);
   len += processIE_prependChannelHoppingIE(eb);
   len += processIE_prependTSCHTimeslotIE(eb);
   len += processIE_prependSyncIE(eb);
   
   //add IE header 
   processIE_prependMLMEIE(eb,len);
  
   // some l2 information about this packet
   eb->l2_frameType                     = IEEE154_TYPE_BEACON;
   eb->l2_nextORpreviousHop.type        = ADDR_16B;
   eb->l2_nextORpreviousHop.addr_16b[0] = 0xff;
   eb->l2_nextORpreviousHop.addr_16b[1] = 0xff;
   
   //I has an IE in my payload
   eb->l2_payloadIEpresent = TRUE;

   // set l2-security attributes
   eb->l2_securityLevel   = IEEE802154_SECURITY_LEVEL_BEACON;
   eb->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
   eb->l2_keyIndex        = IEEE802154_SECURITY_K1_KEY_INDEX;

   // put in queue for MAC to handle
   sixtop_send_internal(eb,eb->l2_payloadIEpresent);
   
   // I'm now busy sending an EB
   sixtop_vars.busySendingEB = TRUE;
}

/**
\brief Send an keep-alive message, if necessary.

This is one of the MAC management tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sixtop_sendKA() {
   OpenQueueEntry_t* kaPkt;
   open_addr_t*      kaNeighAddr;
   
   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (EB and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm not busy sending an EB or KA
      sixtop_vars.busySendingEB = FALSE;
      sixtop_vars.busySendingKA = FALSE;
      
      // stop here
      return;
   }
   
   if (sixtop_vars.busySendingKA==TRUE) {
      // don't proceed if I'm still sending a KA
      return;
   }
   
   kaNeighAddr = neighbors_getKANeighbor(sixtop_vars.kaPeriod);
   if (kaNeighAddr==NULL) {
      // don't proceed if I have no neighbor I need to send a KA to
      return;
   }
   
   // if I get here, I will send a KA
   
   // get a free packet buffer
   kaPkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP);
   if (kaPkt==NULL) {
      openserial_printError(COMPONENT_SIXTOP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)1,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   kaPkt->creator = COMPONENT_SIXTOP;
   kaPkt->owner   = COMPONENT_SIXTOP;
   
   // some l2 information about this packet
   kaPkt->l2_frameType = IEEE154_TYPE_DATA;
   memcpy(&(kaPkt->l2_nextORpreviousHop),kaNeighAddr,sizeof(open_addr_t));
   
   // set l2-security attributes
   kaPkt->l2_securityLevel   = IEEE802154_SECURITY_LEVEL;
   kaPkt->l2_keyIdMode       = IEEE802154_SECURITY_KEYIDMODE;
   kaPkt->l2_keyIndex        = IEEE802154_SECURITY_K2_KEY_INDEX;

   // put in queue for MAC to handle
   sixtop_send_internal(kaPkt,FALSE);
   
   // I'm now busy sending a KA
   sixtop_vars.busySendingKA = TRUE;

#ifdef OPENSIM
   debugpins_ka_set();
   debugpins_ka_clr();
#endif
}

//======= six2six task

void timer_sixtop_six2six_timeout_fired(void) {
   // timeout timer fired, reset the state of sixtop to idle
   sixtop_vars.six2six_state = SIX_STATE_IDLE;
   sixtop_vars.handler = SIX_HANDLER_NONE;
   opentimers_stop(sixtop_vars.timeoutTimerId);
}

void sixtop_six2six_sendDone(OpenQueueEntry_t* msg, owerror_t error){
    uint8_t i,numOfCells;
    uint8_t* ptr;
    cellInfo_ht cellList[SCHEDULEIEMAXNUMCELLS];

    memset(cellList,0,SCHEDULEIEMAXNUMCELLS*sizeof(cellInfo_ht));

    ptr = msg->l2_sixtop_cellObjects;
    numOfCells = msg->l2_sixtop_numOfCells;
    msg->owner = COMPONENT_SIXTOP_RES;
    // if this is a response send done
    if (
        msg->l2_sixtop_returnCode == IANA_6TOP_RC_ERR_VER   ||
        msg->l2_sixtop_returnCode == IANA_6TOP_RC_ERR_SFID  ||
        msg->l2_sixtop_returnCode == IANA_6TOP_RC_ERR_BUSY  ||
        msg->l2_sixtop_returnCode == IANA_6TOP_RC_ERR_NORES ||
        msg->l2_sixtop_returnCode == IANA_6TOP_RC_ERR_RESET ||
        msg->l2_sixtop_returnCode == IANA_6TOP_RC_ERR
    ){
        // no matter successfully being sent out or failed with 
        // 3 times retries, if this is a sixtop response, free 
        // the buffer and end the transcation on my side.
        openqueue_freePacketBuffer(msg);
        return;
    }
    // if this is a request send done
    if(error == E_FAIL) {
        if (
            sixtop_vars.six2six_state == SIX_STATE_WAIT_ADDREQUEST_SENDDONE    ||
            sixtop_vars.six2six_state == SIX_STATE_WAIT_DELETEREQUEST_SENDDONE ||
            sixtop_vars.six2six_state == SIX_STATE_WAIT_LISTREQUEST_SENDDONE   ||
            sixtop_vars.six2six_state == SIX_STATE_WAIT_COUNTREQUEST_SENDDONE  ||
            sixtop_vars.six2six_state == SIX_STATE_WAIT_CLEARREQUEST_SENDDONE
        ){
            // reset handler and state if the request is failed to send out
            sixtop_vars.six2six_state = SIX_STATE_IDLE;
            sixtop_vars.handler       = SIX_HANDLER_NONE;
        }
        openqueue_freePacketBuffer(msg);
        return;
    }
    // the packet has been sent out successfully
    switch (sixtop_vars.six2six_state) {
    case SIX_STATE_WAIT_ADDREQUEST_SENDDONE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_ADDRESPONSE;
        break;
    case SIX_STATE_WAIT_DELETEREQUEST_SENDDONE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_DELETERESPONSE;
        break;
    case SIX_STATE_WAIT_LISTREQUEST_SENDDONE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_LISTRESPONSE;
        break;
    case SIX_STATE_WAIT_COUNTREQUEST_SENDDONE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_COUNTRESPONSE;
        break;
    case SIX_STATE_WAIT_CLEARREQUEST_SENDDONE:
        sixtop_vars.six2six_state = SIX_STATE_WAIT_CLEARRESPONSE;
        break;
    default:
        break;
    }
    
    // in case a response is sent out, check the return code
    if (msg->l2_sixtop_returnCode == IANA_6TOP_RC_SUCCESS){
        if (
            msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_ADD ||
            msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_DELETE
        ){
            if (numOfCells>0){
                for (i=0;i<numOfCells;i++){
                    //TimeSlot 2B
                    cellList[i].tsNum       = *(ptr);
                    cellList[i].tsNum      |= (*(ptr+1))<<8;
                    //Ch.Offset 2B
                    cellList[i].choffset    = *(ptr+2);
                    cellList[i].choffset   |= (*(ptr+3))<<8;
                    ptr += 4;
                    // mark with linkoptions as ocuppied
                    cellList[i].linkoptions = CELLTYPE_RX;
                }
                if (msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_ADD){
                     sixtop_addCellsByState(
                          msg->l2_sixtop_frameID,
                          cellList,
                          &(msg->l2_nextORpreviousHop),
                          sixtop_vars.six2six_state);
                } else {
                      sixtop_removeCellsByState(
                          msg->l2_sixtop_frameID,
                          cellList,
                          &(msg->l2_nextORpreviousHop));
                }
            }
        } else {
            if (msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_CLEAR){
                schedule_removeAllCells(msg->l2_sixtop_frameID,
                                      &(msg->l2_nextORpreviousHop));
            } else {
                // for the request command of List or Count, there
                // is no need to handle on response side.
            }
        }
    }
  
    if (
        sixtop_vars.six2six_state == SIX_STATE_WAIT_ADDRESPONSE        ||
        sixtop_vars.six2six_state == SIX_STATE_WAIT_DELETERESPONSE     ||
        sixtop_vars.six2six_state == SIX_STATE_WAIT_COUNTRESPONSE      ||
        sixtop_vars.six2six_state == SIX_STATE_WAIT_LISTRESPONSE       ||
        sixtop_vars.six2six_state == SIX_STATE_WAIT_CLEARRESPONSE
    ){
        // start timeout timer if I am waiting for a response
        opentimers_setPeriod(
            sixtop_vars.timeoutTimerId,
            TIME_MS,
            SIX2SIX_TIMEOUT_MS
        );
        opentimers_restart(sixtop_vars.timeoutTimerId);
    }
    // discard reservation packets this component has created
    openqueue_freePacketBuffer(msg);
}

port_INLINE bool sixtop_processIEs(OpenQueueEntry_t* pkt, uint16_t * lenIE) {
    uint8_t ptr;
    uint8_t temp_8b,gr_elem_id,subid;
    uint16_t temp_16b,len,sublen;
    uint8_t version,commandIdORcode;
    uint8_t sfId;
   
    ptr=0;  
  
    //candidate IE header  if type ==0 header IE if type==1 payload IE
    temp_8b = *((uint8_t*)(pkt->payload)+ptr);
    ptr++;
    temp_16b = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr))<<8);
    ptr++;
    *lenIE = ptr;
    if(
        (temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) == 
        IEEE802154E_DESC_TYPE_PAYLOAD_IE
    ){
        //payload IE - last bit is 1
        len = temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK;
        gr_elem_id = 
            (temp_16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>
            IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT;
    }else {
        //header IE - last bit is 0
        len = temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK;
        gr_elem_id = (temp_16b & IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK)>>
            IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT; 
    }
  
    *lenIE += len;
    //now determine sub elements if any
    switch(gr_elem_id){
    case SIXTOP_IE_GROUPID:
        subid = *((uint8_t*)(pkt->payload)+ptr);
        ptr += 1;
        sublen = len - 1;
        switch(subid){
            case IANA_6TOP_SUBIE_ID:
                temp_8b = *((uint8_t*)(pkt->payload)+ptr);
                ptr = ptr + 1;
                // get 6P version and command ID
                version   = temp_8b & 0x0f;
                commandIdORcode = (temp_8b >> 4) & 0x0f;
                // get sf_id
                sfId = *((uint8_t*)(pkt->payload)+ptr);
                ptr = ptr + 1;
                sixtop_notifyReceiveCommand(version,commandIdORcode,sfId,ptr,sublen-2,pkt);
                ptr += sublen-2;
                break;
            default:
                return FALSE;
        }
        break;
    default:
        *lenIE = 0;//no header or not recognized.
        return FALSE;
    }
    if (*lenIE>127) {
        // log the error
        openserial_printError(COMPONENT_IEEE802154E,ERR_HEADER_TOO_LONG,
                           (errorparameter_t)*lenIE,
                           (errorparameter_t)1);
    }
    return TRUE;
}

void sixtop_notifyReceiveCommand(
    uint8_t           version, 
    uint8_t           commandIdORcode, 
    uint8_t           sfId,
    uint8_t           ptr,
    uint8_t           length,
    OpenQueueEntry_t* pkt
){
    uint8_t           code;
    uint8_t           frameID;
    uint8_t           numOfCells;
    uint16_t          count;
    cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
    OpenQueueEntry_t* response_pkt;
    uint8_t           len = 0;
    uint8_t           container;
    
    memset(cellList,0,sizeof(cellList));
    
    // get a free packet buffer
    response_pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
    if (response_pkt==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP_RES,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
   
    // take ownership
    response_pkt->creator = COMPONENT_SIXTOP_RES;
    response_pkt->owner   = COMPONENT_SIXTOP_RES;
    
    memcpy(&(response_pkt->l2_nextORpreviousHop),
           &(pkt->l2_nextORpreviousHop),
           sizeof(open_addr_t)
    );
    
    //====== the version and sfID are correct
    //------ if this is a command
    if (
        commandIdORcode == IANA_6TOP_CMD_ADD    ||
        commandIdORcode == IANA_6TOP_CMD_DELETE ||
        commandIdORcode == IANA_6TOP_CMD_COUNT  ||
        commandIdORcode == IANA_6TOP_CMD_LIST   ||
        commandIdORcode == IANA_6TOP_CMD_CLEAR
    ){
        // if the version or sfID is correct
        if (version != IANA_6TOP_6P_VERSION || sfId != SFID_SF0){
            if (version != IANA_6TOP_6P_VERSION){
                code = IANA_6TOP_RC_ERR_VER;
            } else {
                code = IANA_6TOP_RC_ERR_SFID;
            }
        } else {
            // if I am already in a 6top transactions
            if (sixtop_vars.six2six_state != SIX_STATE_IDLE){
                code = IANA_6TOP_RC_ERR_BUSY;
            } else {
                switch(commandIdORcode){
                case IANA_6TOP_CMD_ADD:
                case IANA_6TOP_CMD_DELETE:
                    numOfCells = *((uint8_t*)(pkt->payload)+ptr);
                    container  = *((uint8_t*)(pkt->payload)+ptr+1);
                    frameID = container;
                    processIE_retrieve_sixCelllist(pkt,ptr+2,length-2,cellList);
                    if (
                        commandIdORcode == IANA_6TOP_CMD_ADD &&
                        schedule_getNumberOfFreeEntries() < numOfCells
                    ){
                        // no enough free buffer for adding more cells
                        code = IANA_6TOP_RC_ERR_NORES;
                        break;
                    }
                    if (
                        (
                          commandIdORcode == IANA_6TOP_CMD_ADD &&
                          sixtop_areAvailableCellsToBeScheduled(frameID,numOfCells,cellList)
                        ) ||
                       (
                          commandIdORcode == IANA_6TOP_CMD_DELETE &&
                          sixtop_areAvailableCellsToBeRemoved(frameID,numOfCells,cellList,&(pkt->l2_nextORpreviousHop))
                       )
                    ){
                        code = IANA_6TOP_RC_SUCCESS;
                        len += processIE_prepend_sixCelllist(response_pkt,cellList);
                    } else {
                        code = IANA_6TOP_RC_ERR_RESET;
                    }
                    break;
                case IANA_6TOP_CMD_COUNT:
                    container  = *((uint8_t*)(pkt->payload)+ptr);
                    frameID = container;
                    count = schedule_getCellsCounts(frameID,
                                                    CELLTYPE_RX,
                                                    &(pkt->l2_nextORpreviousHop));
                    code = IANA_6TOP_RC_SUCCESS;
                    packetfunctions_reserveHeaderSize(response_pkt,2);
                    response_pkt->payload[0] = count      & 0xFF;
                    response_pkt->payload[1] = (count>>8) & 0xFF;
                    len = 2;
                    break;
                case IANA_6TOP_CMD_LIST:
                    container  = *((uint8_t*)(pkt->payload)+ptr);
                    frameID = container;
                    numOfCells = sixtop_getCelllist(frameID,
                                         &(pkt->l2_nextORpreviousHop),
                                         cellList);
                    code = IANA_6TOP_RC_SUCCESS;
                    if (numOfCells>0){
                        len += processIE_prepend_sixCelllist(response_pkt,cellList);
                    }
                    break;
                case IANA_6TOP_CMD_CLEAR:
                    container  = *((uint8_t*)(pkt->payload)+ptr);
                    frameID = container;
                    // the cells will be removed when the repsonse sendone successfully
                    // don't clear cells here
                    code = IANA_6TOP_RC_SUCCESS;
                    break;
                default:
                    // don't know this 6top command
                    code = IANA_6TOP_RC_ERR;
                }
            }
        }
        response_pkt->l2_sixtop_requestCommand = commandIdORcode;
        response_pkt->l2_sixtop_frameID        = frameID;
        
        len += processIE_prepend_sixGeneralMessage(response_pkt,code);
        len += processIE_prepend_sixSubID(response_pkt);
        processIE_prepend_sixtopIE(response_pkt,len);
        // indicate IEs present
        response_pkt->l2_payloadIEpresent = TRUE;
        if (sixtop_vars.isResponseEnabled){
            // send packet
            sixtop_send(response_pkt);
        } else {
            openqueue_freePacketBuffer(response_pkt);
        }
    } else {
        //------ if this is a return code
        // The response packet is not required, release it
        openqueue_freePacketBuffer(response_pkt);
      
        // if the code is SUCCESS
        if (commandIdORcode==IANA_6TOP_RC_SUCCESS){
            switch(sixtop_vars.six2six_state){
            case SIX_STATE_WAIT_ADDRESPONSE:
            case SIX_STATE_WAIT_DELETERESPONSE:
                processIE_retrieve_sixCelllist(pkt,ptr,length,cellList);
                // always default frameID
                frameID = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
                if (sixtop_vars.six2six_state == SIX_STATE_WAIT_ADDRESPONSE){
                    sixtop_addCellsByState(frameID,
                                          cellList,
                                          &(pkt->l2_nextORpreviousHop),
                                          sixtop_vars.six2six_state
                    );
                } else {
                    sixtop_removeCellsByState(
                          frameID,
                          cellList,
                          &(pkt->l2_nextORpreviousHop));
                }
                break;
            case SIX_STATE_WAIT_COUNTRESPONSE:
                count  = *((uint8_t*)(pkt->payload)+ptr);
                ptr += 1;
                count |= (*((uint8_t*)(pkt->payload)+ptr))<<8;
                openserial_printInfo(COMPONENT_SIXTOP,ERR_SIXTOP_COUNT,
                       (errorparameter_t)count,
                       (errorparameter_t)sixtop_vars.six2six_state);
                break;
            case SIX_STATE_WAIT_LISTRESPONSE:
                processIE_retrieve_sixCelllist(pkt,ptr,length,cellList);
                // print out first two cells in the list
                openserial_printInfo(COMPONENT_SIXTOP,ERR_SIXTOP_LIST,
                       (errorparameter_t)cellList[0].tsNum,
                       (errorparameter_t)cellList[1].tsNum);
                break;
            case SIX_STATE_WAIT_CLEARRESPONSE:
                frameID = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
                schedule_removeAllCells(frameID,
                                        &(pkt->l2_nextORpreviousHop));
                break;
            default:
                code = IANA_6TOP_RC_ERR;
            }
        } else {
            if (commandIdORcode==IANA_6TOP_RC_ERR_BUSY){
                // disable sf0 for [0...2^4] slotframe long time
                sf0_setBackoff(openrandom_get16b()%(1<<4));
            } else {
                if (commandIdORcode==IANA_6TOP_RC_ERR_NORES){
                    // mark this neighbor as no resource for future processing
                    neighbors_setNeighborNoResource(&(pkt->l2_nextORpreviousHop));
                } else {
                    if (commandIdORcode==IANA_6TOP_RC_ERR_RESET){
                        // TBD: the neighbor can't statisfy the 6p request with given cells, call sf0 to make a decision 
                        // (e.g. issue another 6p request with different cell list)
                    } else {
                        if (commandIdORcode==IANA_6TOP_RC_ERR){
                            // TBD: the neighbor can't statisfy the 6p request, call sf0 to make a decision
                        } else {
                            if (commandIdORcode==IANA_6TOP_RC_ERR_VER){
                                // TBD: the 6p verion does not match
                            } else {
                                if (commandIdORcode==IANA_6TOP_RC_ERR_SFID){
                                    // TBD: the sfId does not match
                                } else {
                                    // TBD: ...
                                }
                            }
                        }
                    }
                }
            }
        }
        openserial_printInfo(COMPONENT_SIXTOP,ERR_SIXTOP_RETURNCODE,
                       (errorparameter_t)commandIdORcode,
                       (errorparameter_t)sixtop_vars.six2six_state);
        sixtop_vars.six2six_state   = SIX_STATE_IDLE;
        sixtop_vars.handler         = SIX_HANDLER_NONE;
        opentimers_stop(sixtop_vars.timeoutTimerId);
    }
}

uint8_t sixtop_getCelllist(
    uint8_t              frameID,
    open_addr_t*         neighbor,
    cellInfo_ht*         cellList
    ){
    uint8_t          i=0;
    scheduleEntry_t* scheduleWalker;
    scheduleEntry_t* currentEntry;
   
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    
    if (frameID != SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE){
        ENABLE_INTERRUPTS();
        return 0;
    }
    
    memset(cellList,0,SCHEDULEIEMAXNUMCELLS*sizeof(cellInfo_ht));
   
    scheduleWalker = schedule_getCurrentScheduleEntry();
    currentEntry   = scheduleWalker;
    do {
       if(packetfunctions_sameAddress(&(scheduleWalker->neighbor),neighbor)){
           cellList[i].tsNum        = scheduleWalker->slotOffset;
           cellList[i].choffset     = scheduleWalker->channelOffset;
           cellList[i].linkoptions  = scheduleWalker->type;
           i++;
       }
       scheduleWalker = scheduleWalker->next;
    }while(scheduleWalker!=currentEntry && i!=SCHEDULEIEMAXNUMCELLS);
   
    ENABLE_INTERRUPTS();
    return i;
}

//======= helper functions

bool sixtop_candidateAddCellList(
      uint8_t*     frameID,
      cellInfo_ht* cellList,
      uint8_t      requiredCells
   ){
   frameLength_t i;
   uint8_t counter;
   uint8_t numCandCells;
   
   *frameID = schedule_getFrameHandle();
   
   numCandCells=0;
   for(counter=0;counter<SCHEDULEIEMAXNUMCELLS;counter++){
      i = openrandom_get16b()%schedule_getFrameLength();
      if(schedule_isSlotOffsetAvailable(i)==TRUE){
         cellList[numCandCells].tsNum       = i;
         cellList[numCandCells].choffset    = openrandom_get16b()%16;
         cellList[numCandCells].linkoptions = CELLTYPE_TX;
         numCandCells++;
      }
   }
   
   if (numCandCells<requiredCells || requiredCells==0) {
      return FALSE;
   } else {
      return TRUE;
   }
}

bool sixtop_candidateRemoveCellList(
      uint8_t*     frameID,
      cellInfo_ht* cellList,
      open_addr_t* neighbor,
      uint8_t      requiredCells
   ){
   uint8_t              i;
   uint8_t              numCandCells;
   slotinfo_element_t   info;
   
   *frameID        = schedule_getFrameHandle();
  
   numCandCells    = 0;
   for(i=0;i<schedule_getFrameLength();i++){
      schedule_getSlotInfo(i,neighbor,&info);
      if(info.link_type == CELLTYPE_TX){
         cellList[numCandCells].tsNum       = i;
         cellList[numCandCells].choffset    = info.channelOffset;
         cellList[numCandCells].linkoptions = CELLTYPE_TX;
         numCandCells++;
         if (numCandCells==SCHEDULEIEMAXNUMCELLS){
            break; // only delete one cell
         }
      }
   }
   
   if(numCandCells<requiredCells){
      return FALSE;
   }else{
      return TRUE;
   }
}

void sixtop_addCellsByState(
      uint8_t      slotframeID,
      cellInfo_ht* cellList,
      open_addr_t* previousHop,
      uint8_t      state
   ){
   uint8_t     i;
   open_addr_t temp_neighbor;
  
   //set schedule according links
   
   for(i = 0;i<SCHEDULEIEMAXNUMCELLS;i++){
      if(cellList[i].linkoptions != CELLTYPE_OFF){
          memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
          //add a RX link
          schedule_addActiveSlot(
              cellList[i].tsNum,
              cellList[i].linkoptions,
              FALSE,
              cellList[i].choffset,
              &temp_neighbor
          );
      }
   }
}

void sixtop_removeCellsByState(
      uint8_t      slotframeID,
      cellInfo_ht* cellList,
      open_addr_t* previousHop
   ){
   uint8_t i;
   
   for(i=0;i<SCHEDULEIEMAXNUMCELLS;i++){   
      if(cellList[i].linkoptions != CELLTYPE_OFF){
         schedule_removeActiveSlot(
            cellList[i].tsNum,
            previousHop
         );
      }
   }
}

bool sixtop_areAvailableCellsToBeScheduled(
      uint8_t      frameID, 
      uint8_t      numOfCells, 
      cellInfo_ht* cellList
   ){
   uint8_t i;
   uint8_t bw;
   bool    available;
   
   i          = 0;
   bw         = numOfCells;
   available  = FALSE;
  
   if(bw == 0 || bw>SCHEDULEIEMAXNUMCELLS){
      // log wrong parameter error TODO
    
      available = FALSE;
   } else {
      do {
         if(schedule_isSlotOffsetAvailable(cellList[i].tsNum) == TRUE){
            bw--;
         } else {
            cellList[i].linkoptions = CELLTYPE_OFF;
         }
         i++;
      }while(i<SCHEDULEIEMAXNUMCELLS && bw>0);
      
      if(bw==0){
         //the rest link will not be scheduled, mark them as off type
         while(i<SCHEDULEIEMAXNUMCELLS){
            cellList[i].linkoptions = CELLTYPE_OFF;
            i++;
         }
         // local schedule can statisfy the bandwidth of cell request. 
         available = TRUE;
      } else {
         // local schedule can't statisfy the bandwidth of cell request
         available = FALSE;
      }
   }
   
   return available;
}

bool sixtop_areAvailableCellsToBeRemoved(      
        uint8_t      frameID, 
        uint8_t      numOfCells, 
        cellInfo_ht* cellList,
        open_addr_t* neighbor
    ){
   uint8_t              i;
   uint8_t              bw;
   bool                 available;
   slotinfo_element_t   info;
   
   i          = 0;
   bw         = numOfCells;
   available  = FALSE;
  
   if(bw == 0 || bw>SCHEDULEIEMAXNUMCELLS){
      // log wrong parameter error TODO
      available = FALSE;
   } else {
      do {
          schedule_getSlotInfo(cellList[i].tsNum,neighbor,&info);
          if(info.link_type == CELLTYPE_RX){
              bw--;
          } else {
              cellList[i].linkoptions = CELLTYPE_OFF;
          }
          i++;
        }while(i<SCHEDULEIEMAXNUMCELLS && bw>0);
      
        if(bw==0){
            //the rest link will not be scheduled, mark them as off type
            while(i<SCHEDULEIEMAXNUMCELLS){
                cellList[i].linkoptions = CELLTYPE_OFF;
                i++;
            }
            // local schedule can statisfy the bandwidth of cell request. 
            available = TRUE;
        } else {
            // local schedule can't statisfy the bandwidth of cell request
            available = FALSE;
        }
   }
   
   return available;
}
