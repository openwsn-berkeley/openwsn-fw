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



#define _DEBUG_SIXTOP_
//#define _DEBUG_SIXTOP_TIMEOUT_
//#define _DEBUG_EB_
//#define _DEBUG_KA_


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

// state change
void           sixtop_setState(six2six_state_t state);

//=== EB/KA task

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
//   uint8_t*             flag,
   cellInfo_ht*         cellList,
   track_t              track,
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
//   uint8_t              numOfLinks,
   cellInfo_ht*         cellList,
   track_t              track,
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
   sixtop_vars.timeoutTimerId     = TOO_MANY_TIMERS_ERROR;
   sixtop_vars.isResponseEnabled  = TRUE;
   sixtop_vars.handler            = SIX_HANDLER_NONE;

   
   sixtop_vars.maintenanceTimerId = opentimers_start(
      sixtop_vars.periodMaintenance,
      TIMER_PERIODIC,
      TIME_MS,
      sixtop_maintenance_timer_cb
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
   if(ebPeriod < SIXTOP_MINIMAL_EBPERIOD) {
      sixtop_vars.ebPeriod = SIXTOP_MINIMAL_EBPERIOD;
   } else {
      sixtop_vars.ebPeriod = ebPeriod;
   } 
}

void sixtop_setHandler(six2six_handler_t handler) {
    sixtop_vars.handler = handler;
}

//======= scheduling


//is sixtop idle (i.e. no on-going negotiation)
bool sixtop_isIdle(){
   return(sixtop_vars.six2six_state == SIX_IDLE);
}

void sixtop_request(uint8_t code, open_addr_t* neighbor, uint8_t numCells, track_t track){
    OpenQueueEntry_t* pkt;
    uint8_t           len;
    uint8_t           container;
    uint8_t           frameID;
    cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
    memset(cellList,0,sizeof(cellList));
   
    // filter parameters
    if(sixtop_vars.six2six_state!=SIX_IDLE){
        return;
    }
    if (neighbor==NULL){
        return;
    }
   
    if (sixtop_vars.handler == SIX_HANDLER_NONE) {
        // sxitop handler must not be NONE
        return;
    }

   
    // generate candidate cell list
    if (code == IANA_6TOP_CMD_ADD){
        if (sixtop_candidateAddCellList(&frameID,cellList,track,numCells)==FALSE){
              return;
        } else{
            // container to be define by SF, currently equals to frameID
            container  = frameID;
        }
    }
    if (code == IANA_6TOP_CMD_DELETE){
        if (sixtop_candidateRemoveCellList(&frameID,cellList,neighbor,numCells)==FALSE){
              return;
        } else{
            // container to be define by SF, currently equals to frameID
            container  = frameID;
        }
    }


    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
    if (pkt==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP_RES,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }

    // update state
    sixtop_setState(SIX_SENDING_REQUEST);

    // take ownership
    pkt->creator = COMPONENT_SIXTOP_RES;
    pkt->owner   = COMPONENT_SIXTOP_RES;
   
    memcpy(&(pkt->l2_nextORpreviousHop),neighbor,sizeof(open_addr_t));

#ifdef _DEBUG_SIXTOP_
   uint8_t  i;
   char str[150];

   sprintf(str, "LinkReq/Rem enqueued: to ");
   openserial_ncat_uint8_t_hex(str, neighbor->addr_64b[6], 150);
   openserial_ncat_uint8_t_hex(str, neighbor->addr_64b[7], 150);
   strncat(str, ", bw=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)numCells, 150);
   strncat(str, ", nbcells ", 150);
   openserial_ncat_uint32_t(str, (uint32_t)SCHEDULEIEMAXNUMCELLS, 150);
   strncat(str, ", track ", 150);
   openserial_ncat_uint32_t(str, (uint32_t)track.instance, 150);

   for(i=0; i<SCHEDULEIEMAXNUMCELLS; i++){
      strncat(str, ", slot ", 150);
      openserial_ncat_uint32_t(str, (uint32_t)cellList[i].tsNum, 150);
      strncat(str, ", ch ", 150);
      openserial_ncat_uint32_t(str, (uint32_t)cellList[i].choffset, 150);
   }
   openserial_printf(COMPONENT_SIXTOP, str, strlen(str));

#endif
   
    // create packet
    len  = 0;
    if (code == IANA_6TOP_CMD_ADD || IANA_6TOP_CMD_DELETE){
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
       sixtop_setState(SIX_WAIT_ADDREQUEST_SENDDONE);
        break;
    case IANA_6TOP_CMD_DELETE:
       sixtop_setState(SIX_WAIT_DELETEREQUEST_SENDDONE);
        break;
    case IANA_6TOP_CMD_COUNT:
       sixtop_setState(SIX_WAIT_COUNTREQUEST_SENDDONE);
        break;
    case IANA_6TOP_CMD_LIST:
       sixtop_setState(SIX_WAIT_LISTREQUEST_SENDDONE);
        break;
    case IANA_6TOP_CMD_CLEAR:
       sixtop_setState(SIX_WAIT_CLEARREQUEST_SENDDONE);
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

   // filter parameters
   if (sixtop_vars.six2six_state!=SIX_IDLE){
      return;
   }
   if (neighbor==NULL){
      return;
   }
   if (sixtop_vars.handler == SIX_HANDLER_NONE) {
      openserial_printCritical(
            COMPONENT_SIXTOP, ERR_SIXTOP_WRONG_HANDLER,
            (errorparameter_t)0,
            (errorparameter_t)0
      );

      // sixtop handler must not be NONE
      return;
   }

    // filter parameters
    if (sixtop_vars.six2six_state!=SIX_IDLE){
        return;
    }
    if (neighbor==NULL){
        return;
    }
    if (sixtop_vars.handler == SIX_HANDLER_NONE) {
        // sixtop handler must not be NONE
        return;
    }

    // set cell list (only first one is to be removed)
    frameID        = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
    container      = frameID;
    memcpy(&(cellList[0]),cellInfo,sizeof(cellList));
   
   
    // get a free packet buffer
    pkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
    if(pkt==NULL) {
        openserial_printError(
            COMPONENT_SIXTOP_RES,
            ERR_NO_FREE_PACKET_BUFFER,
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        return;
    }
   
    // update state
    sixtop_setState(SIX_SENDING_REQUEST);
   
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
 

#ifdef _DEBUG_SIXTOP_
   uint8_t  i;
   char str[150];

   sprintf(str, "LinkRem txed - case2: to ");
   openserial_ncat_uint8_t_hex(str, neighbor->addr_64b[6], 150);
   openserial_ncat_uint8_t_hex(str, neighbor->addr_64b[7], 150);
   strncat(str, ", bw=", 150);
   openserial_ncat_uint32_t(str, (uint32_t)1, 150);
   strncat(str, ", nbcells ", 150);
   openserial_ncat_uint32_t(str, (uint32_t)1, 150);

   for(i=0; i<1; i++){
      strncat(str, ", slot ", 150);
      openserial_ncat_uint32_t(str, (uint32_t)cellList[i].tsNum, 150);
   }
   openserial_printf(COMPONENT_SIXTOP, str, strlen(str));

#endif

    // indicate IEs present
    pkt->l2_payloadIEpresent = TRUE;
   
    // send packet
    sixtop_send(pkt);
   
    //update states
    switch(code){
    case IANA_6TOP_CMD_ADD:
        sixtop_setState(SIX_WAIT_ADDREQUEST_SENDDONE);
        break;
    case IANA_6TOP_CMD_DELETE:
        sixtop_setState(SIX_WAIT_DELETEREQUEST_SENDDONE);
        break;
    }

}

//======= maintaning 
void sixtop_maintaining(uint16_t slotOffset,open_addr_t* neighbor){
    slotinfo_element_t info;
    cellInfo_ht linkInfo;
    schedule_getSlotInfo(slotOffset,neighbor,&info);
    if(info.link_type != CELLTYPE_OFF){
        linkInfo.tsNum       = slotOffset;
        linkInfo.choffset    = info.channelOffset;
        linkInfo.linkoptions = info.link_type;
        sixtop_vars.handler  = SIX_HANDLER_MAINTAIN;
        sixtop_addORremoveCellByInfo(IANA_6TOP_CMD_DELETE,neighbor, &linkInfo);
    } else {
        //should log this error
        
        return;
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
   
#if defined(_DEBUG_EB_) || defined (_DEBUG_KA_)
   char str[150];
#endif


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

#ifdef _DEBUG_EB_
            sprintf(str, "EB transmitted");
            openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

         } else {
            // this is a KA
            
#ifdef _DEBUG_KA_
            sprintf(str, "KA transmitted");
            openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

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

#if defined(_DEBUG_EB_) || defined (_DEBUG_KA_)
         sprintf(str, "KA/EB maintenance period=");
         openserial_ncat_uint32_t(str, (uint32_t)sixtop_vars.periodMaintenance, 150);
         openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

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
\param[in] iePresent Indicates wheter an Information Element is present in the
   packet.
\param[in] frameVersion The frame version to write in the packet.

\returns E_SUCCESS iff successful.
*/
owerror_t sixtop_send_internal(
   OpenQueueEntry_t* msg, 
   bool    payloadIEPresent) {

   // assign a number of retries
   if (
         packetfunctions_isBroadcastMulticast_debug(&(msg->l2_nextORpreviousHop), 61)==TRUE
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

   //otf notification
//   otf_notif_pktTx(msg);

   return E_SUCCESS;
}

// timer interrupt callbacks

void sixtop_maintenance_timer_cb(opentimer_id_t id) {
   scheduler_push_task(timer_sixtop_management_fired,TASKPRIO_SIXTOP);
}

void sixtop_timeout_timer_cb(opentimer_id_t id) {
   scheduler_push_task(timer_sixtop_six2six_timeout_fired,TASKPRIO_SIXTOP_TIMEOUT);
}

//======= EB/KA task

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timer_sixtop_management_fired(void) {
   scheduleEntry_t* entry;
   sixtop_vars.mgtTaskCounter = (sixtop_vars.mgtTaskCounter+1)%sixtop_vars.ebPeriod;

   switch (sixtop_vars.mgtTaskCounter) {
      case 0:
         // called every EBPERIOD seconds
         sixtop_sendEB();
         break;
      case 1:
         // called every EBPERIOD seconds
         neighbors_removeOld();
         break;
      case 2:
         // called every EBPERIOD seconds
         entry = schedule_statistic_poorLinkQuality();
         if (
             entry       != NULL                        && \
             entry->type != CELLTYPE_OFF                && \
             entry->type != CELLTYPE_TXRX               
         ){
             sixtop_maintaining(entry->slotOffset,&(entry->neighbor));
         }
      default:
         // called every second, except third times every EBPERIOD seconds
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
#if defined(_DEBUG_EB_) || defined(_DEBUG_KA_)
   char str[150];
#endif

   len = 0;
   
   if ((ieee154e_isSynch()==FALSE) || (icmpv6rpl_getMyDAGrank()==DEFAULTDAGRANK)){
      // I'm not sync'ed or I did not acquire a DAGrank
      
      // delete packets genereted by this module (EB and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm now busy sending an EB
      sixtop_vars.busySendingEB = FALSE;

#ifdef _DEBUG_EB_
      sprintf(str, "EB not sync'ed");
      openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

      // stop here
      return;
   }
   
   if (sixtop_vars.busySendingEB==TRUE) {
      // don't continue if I'm still sending a previous EB
#ifdef _DEBUG_EB_
      sprintf(str, "EB BUSY");
      openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif


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

#ifdef _DEBUG_EB_
   sprintf(str, "EB enqueued");
   openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

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
#ifdef _DEBUG_KA_
   char str[150];
#endif

   
   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (EB and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm now busy sending a KA
      sixtop_vars.busySendingKA = FALSE;
      
#ifdef _DEBUG_KA_
      sprintf(str, "KA not sync'ed");
      openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif


      // stop here
      return;
   }
   
   if (sixtop_vars.busySendingKA==TRUE) {
#ifdef _DEBUG_KA_
      sprintf(str, "KA busy");
      openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

      // don't proceed if I'm still sending a KA
      return;
   }
   
   kaNeighAddr = neighbors_getKANeighbor(sixtop_vars.kaPeriod);
   if (kaNeighAddr==NULL) {
#ifdef _DEBUG_KA_
      sprintf(str, "KA nothing to do");
      openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif


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

#ifdef _DEBUG_KA_
   sprintf(str, "KA generated for parent ");
   openserial_ncat_uint8_t_hex(str, (uint8_t)kaNeighAddr->addr_64b[6], 150);
   strncat(str, "-", 150);
   openserial_ncat_uint8_t_hex(str, (uint8_t)kaNeighAddr->addr_64b[7], 150);
   openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif

#ifdef OPENSIM
   debugpins_ka_set();
   debugpins_ka_clr();
#endif
}

//======= six2six task


//changes the current sixtop state
void sixtop_setState(six2six_state_t state){
   sixtop_vars.six2six_state = state;
   uint16_t  timeout_sixtop_value;    // to change the timeout value (jitter)


   //schedule a timer: back to the idle state after a timeout
   if (state != SIX_IDLE){

      //stops the previous timer
      if (sixtop_vars.timeoutTimerId != TOO_MANY_TIMERS_ERROR){
         opentimers_stop(sixtop_vars.timeoutTimerId);
         sixtop_vars.timeoutTimerId = TOO_MANY_TIMERS_ERROR;
      }

      //and starts a new one (randomized to avoid all the nodes regenerate one request simultaneously)
      // timeout in [SIX2SIX_TIMEOUT_MS, SIX2SIX_TIMEOUT_MS*1.5]
      timeout_sixtop_value = SIX2SIX_TIMEOUT_MS + openrandom_get16b();    //65536 at most
      while (timeout_sixtop_value > SIX2SIX_TIMEOUT_MS * 1.5){
         timeout_sixtop_value -= SIX2SIX_TIMEOUT_MS / 2;
      }
      sixtop_vars.timeoutTimerId     = opentimers_start(
            timeout_sixtop_value,
            TIMER_ONESHOT,
            TIME_MS,
            sixtop_timeout_timer_cb
      );
   }

   //otf callback when we come back to the idle state
   if (state == SIX_IDLE){
      opentimers_stop(sixtop_vars.timeoutTimerId);
      sixtop_vars.timeoutTimerId = TOO_MANY_TIMERS_ERROR;
      sixtop_vars.handler = SIX_HANDLER_NONE;
  //    otf_verifSchedule();
   }

}

/*
 * brief: the timer has been triggered. 6top has to come back to
 * the idle state.
 */
void timer_sixtop_six2six_timeout_fired(void) {
#ifdef _DEBUG_SIXTOP_TIMEOUT_
   openserial_printInfo(
      COMPONENT_SIXTOP,
      ERR_SIXTOP_TIMEOUT,
      (errorparameter_t)sixtop_vars.six2six_state,
      (errorparameter_t)opentimers_getPeriod(sixtop_vars.timeoutTimerId)
   );
#endif


   // timeout timer fired, reset the state of sixtop to idle (only if we don't have an on-going transmission for this component)
   // no need to call notif_sendDone() since the LinkReq/LinkRem did not modify a priori the schedule (only LlinkRep do)
   if (!ieee154e_is_ongoing(COMPONENT_SIXTOP_RES)) {
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP_RES);
      sixtop_setState(SIX_IDLE);
   }
   //starts a new timer (duration = 2 cells) so that the on-going transmission will be terminated
   else{
      sixtop_vars.timeoutTimerId     = opentimers_start(
            2* TsSlotDuration,
            TIMER_ONESHOT,
            TIME_MS,
            sixtop_timeout_timer_cb
      );
   }
}

void sixtop_six2six_sendDone(OpenQueueEntry_t* msg, owerror_t error){
   uint8_t i,numOfCells;
   uint8_t* ptr;
   cellInfo_ht cellList[SCHEDULEIEMAXNUMCELLS];

#ifdef _DEBUG_SIXTOP_
   char  str[150];
#endif

   memset(cellList,0,SCHEDULEIEMAXNUMCELLS*sizeof(cellInfo_ht));
   ptr = msg->l2_sixtop_cellObjects;
   numOfCells = msg->l2_sixtop_numOfCells;
   msg->owner = COMPONENT_SIXTOP_RES;

   switch (sixtop_vars.six2six_state) {
   case SIX_WAIT_ADDREQUEST_SENDDONE:
      if (error != E_FAIL){
#ifdef _DEBUG_SIXTOP_
         sprintf(str, "LinkReq/Rem txed: to ");
         openserial_ncat_uint8_t_hex(str, msg->l2_nextORpreviousHop.addr_64b[6], 150);
         openserial_ncat_uint8_t_hex(str, msg->l2_nextORpreviousHop.addr_64b[7], 150);
         openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif
         sixtop_setState(SIX_WAIT_ADDRESPONSE);
      }
      else{
#ifdef _DEBUG_SIXTOP_
         sprintf(str, "LinkReq tx failed: to ");
         openserial_ncat_uint8_t_hex(str, msg->l2_nextORpreviousHop.addr_64b[6], 150);
         openserial_ncat_uint8_t_hex(str, msg->l2_nextORpreviousHop.addr_64b[7], 150);
         openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif
         sixtop_setState(SIX_IDLE);
      }

      break;
   case SIX_WAIT_DELETEREQUEST_SENDDONE:
      sixtop_setState(SIX_WAIT_DELETERESPONSE);
      break;
   case SIX_WAIT_LISTREQUEST_SENDDONE:
      sixtop_setState(SIX_WAIT_LISTRESPONSE);
      break;
   case SIX_WAIT_COUNTREQUEST_SENDDONE:
      sixtop_setState(SIX_WAIT_COUNTRESPONSE);
      break;
   case SIX_WAIT_CLEARREQUEST_SENDDONE:
      sixtop_setState(SIX_WAIT_CLEARRESPONSE);
      break;
   case SIX_WAIT_RESPONSE_SENDDONE:

#ifdef _DEBUG_SIXTOP_
         sprintf(str, "LinkRep txed: to ");
         openserial_ncat_uint8_t_hex(str, msg->l2_nextORpreviousHop.addr_64b[6], 150);
         openserial_ncat_uint8_t_hex(str, msg->l2_nextORpreviousHop.addr_64b[7], 150);
         strncat(str, ", bw=", 150);
  /*       openserial_ncat_uint32_t(str, (uint32_t)numOfCells, 150);
         strncat(str, ", track=", 150);
         openserial_ncat_uint32_t(str, (uint32_t)msg->l2_bandwidthIE_track.instance, 150);
         strncat(str, ", owner=", 150);
         openserial_ncat_uint8_t_hex(str, (uint32_t)msg->l2_bandwidthIE_track.owner.addr_64b[6], 150);
         openserial_ncat_uint8_t_hex(str, (uint32_t)msg->l2_bandwidthIE_track.owner.addr_64b[7], 150);
  */
         strncat(str, ", nbcells ", 150);
         openserial_ncat_uint32_t(str, (uint32_t)numOfCells, 150);
         strncat(str, ", state ", 150);
         openserial_ncat_uint32_t(str, (uint32_t)sixtop_vars.six2six_state, 150);
         for(i=0; i<numOfCells; i++){
            strncat(str, ", slot ", 150);
            openserial_ncat_uint32_t(str, (uint32_t)cellList[i].tsNum, 150);
         }
         openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
   #endif

        if (msg->l2_sixtop_returnCode == IANA_6TOP_RC_SUCCESS && error == E_SUCCESS){
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
                        cellList[i].linkoptions = CELLTYPE_TX;
                    }
                    if (msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_ADD){
                         sixtop_addCellsByState(
                              msg->l2_sixtop_frameID,
                              cellList,
                              sixtop_get_trackbesteffort(),
                              &(msg->l2_nextORpreviousHop),
                              sixtop_vars.six2six_state);
                    } else {
                          sixtop_removeCellsByState(
                              msg->l2_sixtop_frameID,
                              cellList,
                              &(msg->l2_nextORpreviousHop));
                    }
                }
                  
            } else{
                if (msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_CLEAR){
                    schedule_removeAllCells(msg->l2_sixtop_frameID,
                                          &(msg->l2_nextORpreviousHop));
                }
            }
        }
        else{

           //if failed - TODO!!!!
        }
        
        sixtop_setState(SIX_IDLE);


        if (
            msg->l2_sixtop_returnCode     == IANA_6TOP_RC_SUCCESS && 
            msg->l2_sixtop_requestCommand == IANA_6TOP_CMD_ADD
        ){

            if (sixtop_vars.handler == SIX_HANDLER_MAINTAIN){

               openserial_printCritical(COMPONENT_SIXTOP,ERR_GENERIC,
                                             (errorparameter_t)122,
                                             (errorparameter_t)100 + sixtop_vars.handler);




               sixtop_request(
                    IANA_6TOP_CMD_DELETE,
                    &(msg->l2_nextORpreviousHop),
                    1,
                    sixtop_get_trackbesteffort()      //we don't care about the trackid for the suppression
                );
                sixtop_request(
                      IANA_6TOP_CMD_ADD,
                      &(msg->l2_nextORpreviousHop),
                      1,
                      sixtop_get_trackbesteffort()
                );
            } else {
                sixtop_vars.handler = SIX_HANDLER_NONE;
            }
        }
        break;
    default:
       openserial_printInfo(
             COMPONENT_SIXTOP,
             ERR_GENERIC,
             (errorparameter_t)111,
             (errorparameter_t)sixtop_vars.six2six_state
       );
       sixtop_setState(SIX_IDLE);
       break;
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
    opcode_IE_ht opcode_ie;
    bandwidth_IE_ht   bandwidth_ie;
    schedule_IE_ht    schedule_ie;
    schedule_IE_ht    blacklist_ie;

    ptr=0;
    memset(&opcode_ie,0,sizeof(opcode_IE_ht));
    memset(&bandwidth_ie,0,sizeof(bandwidth_IE_ht));
    memset(&schedule_ie,0,sizeof(schedule_IE_ht));
    memset(&blacklist_ie,0,sizeof(schedule_IE_ht));


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
    //this is the only groupID that we parse. See page 82.  
    case IEEE802154E_MLME_IE_GROUPID:
        //IE content can be any of the sub-IEs. Parse and see which
        do{

            //read sub IE header
            temp_8b = *((uint8_t*)(pkt->payload)+ptr);
            ptr = ptr + 1;
            temp_16b = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr))<<8);
            ptr = ptr + 1;
            len = len - 2; //remove header fields len
            if(
                (temp_16b & IEEE802154E_DESC_TYPE_LONG) == 
                IEEE802154E_DESC_TYPE_LONG
            ){
                //long sub-IE - last bit is 1
                sublen = temp_16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK;
                subid= 
                    (temp_16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>
                    IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT; 
            } else {
                //short IE - last bit is 0
                sublen = temp_16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK;
                subid = (temp_16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>
                IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT; 
            }
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
            len = len - sublen;
        } while(len>0);
        break;
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
    
    //===== if the version or sfID are correct
    if (version != IANA_6TOP_6P_VERSION || sfId != SFID_SF0){
        if (version != IANA_6TOP_6P_VERSION){
            code = IANA_6TOP_RC_VER_ERR;
        } else {
            code = IANA_6TOP_RC_SFID_ERR;
        }
    } else {
        //====== the version and sfID are correct
        //------ if this is a command
        if (
            commandIdORcode == IANA_6TOP_CMD_ADD    ||
            commandIdORcode == IANA_6TOP_CMD_DELETE ||
            commandIdORcode == IANA_6TOP_CMD_COUNT  ||
            commandIdORcode == IANA_6TOP_CMD_LIST   ||
            commandIdORcode == IANA_6TOP_CMD_CLEAR
        ){
            // if I am already in a 6top transactions
            if (sixtop_vars.six2six_state != SIX_IDLE){
                code = IANA_6TOP_RC_ERR;
            } else {
               sixtop_setState(SIX_REQUEST_RECEIVED);

                switch(commandIdORcode){
                case IANA_6TOP_CMD_ADD:
                case IANA_6TOP_CMD_DELETE:
                    numOfCells = *((uint8_t*)(pkt->payload)+ptr);
                    container  = *((uint8_t*)(pkt->payload)+ptr+1);
                    frameID = container;
                    processIE_retrieve_sixCelllist(pkt,ptr+2,length-2,cellList);
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
                        code = IANA_6TOP_RC_ERR;
                    }

#ifdef _DEBUG_SIXTOP_
                    char      str[150];
                    uint8_t   i;

                    sprintf(str, "LinkReq rcvd - LinkRep prepared: from ");
                    openserial_ncat_uint8_t_hex(str, pkt->l2_nextORpreviousHop.addr_64b[6], 150);
                    openserial_ncat_uint8_t_hex(str, pkt->l2_nextORpreviousHop.addr_64b[7], 150);
                    strncat(str, ", bw=", 150);
                    openserial_ncat_uint32_t(str, (uint32_t)numOfCells, 150);
                    /*strncat(str, ", track=", 150);
                    openserial_ncat_uint32_t(str, (uint32_t)bandwidth_ie->track.instance, 150);
                    strncat(str, ", owner=", 150);
                    openserial_ncat_uint8_t_hex(str, (uint32_t)bandwidth_ie->track.owner.addr_64b[6], 150);
                    openserial_ncat_uint8_t_hex(str, (uint32_t)bandwidth_ie->track.owner.addr_64b[7], 150);
                    */
                    strncat(str, ", nbcells ", 150);
                    openserial_ncat_uint32_t(str, (uint32_t)numOfCells, 150);
                    for(i=0; i<numOfCells; i++){
                       strncat(str, ", slot ", 150);
                       openserial_ncat_uint32_t(str, (uint32_t)cellList[i].tsNum, 150);
                       strncat(str, ", ch ", 150);
                       openserial_ncat_uint32_t(str, (uint32_t)cellList[i].choffset, 150);
                    }
                    openserial_printf(COMPONENT_SIXTOP, str, strlen(str));
#endif


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
                    schedule_removeAllCells(frameID,
                                            &(pkt->l2_nextORpreviousHop));
                    code = IANA_6TOP_RC_SUCCESS;
                    break;
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
            }
            // update state
            sixtop_setState(SIX_WAIT_RESPONSE_SENDDONE);

        } else {
            //------ if this is a return code
            // The response packet is not required, release it
            openqueue_freePacketBuffer(response_pkt);

            // if the code is SUCCESS
            if (commandIdORcode==IANA_6TOP_RC_SUCCESS){
                switch(sixtop_vars.six2six_state){
                case SIX_WAIT_ADDRESPONSE:
                case SIX_WAIT_DELETERESPONSE:
                    processIE_retrieve_sixCelllist(pkt,ptr,length,cellList);
                    // always default frameID
                    frameID = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
                    if (sixtop_vars.six2six_state == SIX_WAIT_ADDRESPONSE){
                        sixtop_addCellsByState(frameID,
                                              cellList,
                                              sixtop_get_trackbesteffort(),
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
                case SIX_WAIT_COUNTRESPONSE:
                    count  = *((uint8_t*)(pkt->payload)+ptr);
                    ptr += 1;
                    count |= (*((uint8_t*)(pkt->payload)+ptr))<<8;
                    openserial_printInfo(COMPONENT_SIXTOP,ERR_SIXTOP_COUNT,
                           (errorparameter_t)count,
                           (errorparameter_t)sixtop_vars.six2six_state);
                    break;
                case SIX_WAIT_LISTRESPONSE:
                    processIE_retrieve_sixCelllist(pkt,ptr,length,cellList);
                    // print out first two cells in the list
                    openserial_printInfo(COMPONENT_SIXTOP,ERR_SIXTOP_LIST,
                           (errorparameter_t)cellList[0].tsNum,
                           (errorparameter_t)cellList[1].tsNum);
                    break;
                case SIX_WAIT_CLEARRESPONSE:

                    break;
                default:
                    code = IANA_6TOP_RC_ERR;
                }
            } else {
                // TBD...
            }
           openserial_printInfo(COMPONENT_SIXTOP,ERR_SIXTOP_RETURNCODE,
                           (errorparameter_t)commandIdORcode,
                           (errorparameter_t)sixtop_vars.six2six_state);
           sixtop_setState(SIX_IDLE);

        }
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
       if(packetfunctions_sameAddress_debug(&(scheduleWalker->neighbor),neighbor,COMPONENT_SIXTOP)){
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

//returns TRUE if slotnum is present in cellList
bool sixtop_cellInList(cellInfo_ht* cellList, uint8_t numCandCells, frameLength_t slotnum){
   uint8_t i;

   for(i=0; i<numCandCells; i++)
      if (cellList[i].tsNum == slotnum)
         return(TRUE);
   return(FALSE);
}


bool sixtop_candidateAddCellList(
      uint8_t*     frameID,
//      uint8_t*     flag,
      cellInfo_ht* cellList,
      track_t      track,
      uint8_t      requiredCells
   ){
   frameLength_t i;
   uint8_t numCandCells;
   

   *frameID = schedule_getFrameHandle();
   

   for(numCandCells=0;numCandCells<SCHEDULEIEMAXNUMCELLS;){
      i = openrandom_get16b() % schedule_getFrameLength();
      if ((schedule_isSlotOffsetAvailable(i)==TRUE) && (!sixtop_cellInList(cellList, numCandCells, i))){
         cellList[numCandCells].tsNum       = i;
         cellList[numCandCells].choffset    = openrandom_get16b()%16;
         cellList[numCandCells].linkoptions = CELLTYPE_TX;
         numCandCells++;
      }
   }
   
   //BUG if we don't have SCHEDULEIEMAXNUMCELLS cells in the list: the receiver will read garbage unitialized data
   if (numCandCells<SCHEDULEIEMAXNUMCELLS || requiredCells==0) {
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
//      uint8_t      numOfLinks,
      cellInfo_ht* cellList,
      track_t      track,
      open_addr_t* previousHop,
      uint8_t      state
   ){
   uint8_t     i;
   open_addr_t temp_neighbor;
   uint8_t     type;
  
   //set schedule according links
   
   for(i = 0;i<SCHEDULEIEMAXNUMCELLS;i++){
      //only schedule when the request side wants to schedule a tx cell
      if(cellList[i].linkoptions != CELLTYPE_OFF){
         switch(state) {

//  todo: cells inserted in the schedule when the linkRep is created, removed if it failed. Idle directly when resp_sent (concurrent rep)
             //case SIX_WAIT_ADDRESPONSE_SENDDONE:
   //         case SIX_IDLE:

               //not yet implemented!!!

             /*  memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
               
               if (track.instance != TRACK_PARENT_CONTROL)
                  type = CELLTYPE_RX;
               else
                  type = CELLTYPE_TXRX;
*/

     //       break;
            case SIX_WAIT_RESPONSE_SENDDONE:
               memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));

               //add a RX link
               schedule_addActiveSlot(
                  cellList[i].tsNum,
                  CELLTYPE_RX,
                  FALSE,
                  cellList[i].choffset,
                  &temp_neighbor,
                  track
               );

               break;
            case SIX_WAIT_ADDRESPONSE:
               memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));

               if (track.instance != TRACK_PARENT_CONTROL)
                   type = CELLTYPE_TX;
                else
                   type = CELLTYPE_TXRX;

               //add a TX link
               schedule_addActiveSlot(
                  cellList[i].tsNum,
                  CELLTYPE_TX,
                  FALSE,
                  cellList[i].choffset,
                  &temp_neighbor,
                  track
               );
               break;
            default:

               openserial_printCritical(COMPONENT_SIXTOP,ERR_GENERIC,
                                             (errorparameter_t)200,
                                             (errorparameter_t)state);


               //log error
               break;
         }
      }
   }
}


// searches for the track associated to this neighbor / slotinfos
track_t sixtop_getTrackCellsByState(
      uint8_t      slotframeID,
      uint8_t      numOfLink,
      cellInfo_ht* cellList,
      open_addr_t* previousHop
   ){
   uint8_t              i;
   slotinfo_element_t   info;
   bool                 found = FALSE;
   track_t              track = sixtop_get_trackbesteffort();

   for(i=0;i<numOfLink;i++){
      if(cellList[i].linkoptions == CELLTYPE_TX){

         schedule_getSlotInfo(
            cellList[i].tsNum,
            previousHop,
            &info
         );

         //all these cells MUST have the same track
         if (found){
            if (!sixtop_is_trackequal(track, info.track))
               openserial_printError(
                     COMPONENT_SIXTOP,
                     ERR_SIXTOP_MULTIPLE_TRACKS,
                     (errorparameter_t)0,
                     (errorparameter_t)0
               );
         }
         else{
            track = info.track;
            found = TRUE;
         }
      }
   }
   return(track);
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
      openserial_printError(COMPONENT_SIXTOP,ERR_GENERIC,
                            (errorparameter_t)48,
                            (errorparameter_t)463);

      available = FALSE;
   } else {

      do {
         if(schedule_isSlotOffsetAvailable(cellList[i].tsNum) == TRUE)
            bw--;
         else
            cellList[i].linkoptions = CELLTYPE_OFF;
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



//======= helper functions


//are these track equal?
bool sixtop_is_trackequal(track_t track1, track_t track2){
   return (packetfunctions_sameAddress_debug(&(track1.owner), &(track2.owner), COMPONENT_SIXTOP)
         && track1.instance == track2.instance);
}

//is this the best effort track?
bool sixtop_is_trackbesteffort(track_t track){

   //error
     if (track.instance == TRACK_BESTEFFORT && track.owner.type != ADDR_NONE){
        openserial_printError(
                     COMPONENT_OTF,
                     ERR_BAD_TRACKID,
                     (errorparameter_t)(uint16_t)(track.owner.type),
                     (errorparameter_t)track.owner.addr_64b[7]
                  );
     }


   return (track.instance == TRACK_BESTEFFORT);
}


//return the best effort track
track_t sixtop_get_trackbesteffort(void){
   track_t track;

   bzero(&(track.owner), sizeof(track.owner));
   track.instance  = TRACK_BESTEFFORT;

   return(track);
}


//return the common track (uses dedicated cells toward the parent, but NO traffic isolation)
track_t sixtop_get_trackcommon(void){
   track_t track;

   bzero(&(track.owner), sizeof(track.owner));
   track.owner.type  = ADDR_64B;
   track.instance    = (uint16_t)TRACK_ALLCOMMON;
   return(track);
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

