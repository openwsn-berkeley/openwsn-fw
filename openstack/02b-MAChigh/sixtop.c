#include "opendefs.h"
#include "sixtop.h"
#include "openserial.h"
#include "openqueue.h"
#include "neighbors.h"
#include "IEEE802154E.h"
#include "iphc.h"
#include "otf.h"
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
#include "pid.h"
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
void          sixtop_notifyReceiveCommand(
   opcode_IE_ht*        opcode_ie, 
   bandwidth_IE_ht*     bandwidth_ie, 
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);
void          sixtop_notifyReceiveLinkRequest(
   bandwidth_IE_ht*     bandwidth_ie,
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);
void          sixtop_linkResponse(
   bool                 success,
   open_addr_t*         tempNeighbor,
   uint8_t              bandwidth,
   schedule_IE_ht*      schedule_ie
);
void          sixtop_notifyReceiveLinkResponse(
   bandwidth_IE_ht*     bandwidth_ie,
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);
void          sixtop_notifyReceiveRemoveLinkRequest(
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);

//=== helper functions

bool          sixtop_candidateAddCellList(
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList
);
bool          sixtop_candidateRemoveCellList(
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList,
   open_addr_t*         neighbor
);
void          sixtop_addCellsByState(
   uint8_t              slotframeID,
   uint8_t              numOfLinks,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop,
   uint8_t              state
);
void          sixtop_removeCellsByState(
   uint8_t              slotframeID,
   uint8_t              numOfLink,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop
);
bool          sixtop_areAvailableCellsToBeScheduled(
   uint8_t              frameID, 
   uint8_t              numOfCells, 
   cellInfo_ht*         cellList, 
   uint8_t              bandwidth
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
   
   sixtop_vars.maintenanceTimerId = opentimers_start(
      sixtop_vars.periodMaintenance,
      TIMER_PERIODIC,
      TIME_MS,
      sixtop_maintenance_timer_cb
   );
   
   sixtop_vars.timeoutTimerId     = opentimers_start(
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

void sixtop_addCells(open_addr_t* neighbor, uint16_t numCells){
   OpenQueueEntry_t* pkt;
   uint8_t           len;
   uint8_t           type;
   uint8_t           frameID;
   uint8_t           flag;
   bool              outcome;
   cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
   frameID    = schedule_getFrameHandle();
   
   memset(cellList,0,sizeof(cellList));
   
   // filter parameters
   if(sixtop_vars.six2six_state!=SIX_IDLE){
      return;
   }
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_IDLE %d\n",sixtop_vars.six2six_state);
#endif
   if (neighbor==NULL){
      return;
   }

   // generate candidate cell list
   outcome = sixtop_candidateAddCellList(
      &type,
      &frameID,
      &flag,
      cellList
   );
   if (outcome == FALSE) {
     return;
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
   sixtop_vars.six2six_state  = SIX_SENDING_ADDREQUEST;
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_SENDING_ADDREQUEST %d\n",sixtop_vars.six2six_state);
#endif
   // take ownership
   pkt->creator = COMPONENT_SIXTOP_RES;
   pkt->owner   = COMPONENT_SIXTOP_RES;
   
   memcpy(
      &(pkt->l2_nextORpreviousHop),
      neighbor,
      sizeof(open_addr_t)
   );
   
   // create packet
   len  = 0;
   len += processIE_prependScheduleIE(pkt,type,frameID,flag,cellList);
   len += processIE_prependBandwidthIE(pkt,numCells,frameID);
   len += processIE_prependOpcodeIE(pkt,SIXTOP_SOFT_CELL_REQ);
   processIE_prependMLMEIE(pkt,len);
   
   // indicate IEs present
   pkt->l2_payloadIEpresent = TRUE;
   
   // send packet
   sixtop_send(pkt);
   
   // update state
   sixtop_vars.six2six_state = SIX_WAIT_ADDREQUEST_SENDDONE;
   
   // arm timeout
   opentimers_setPeriod(
      sixtop_vars.timeoutTimerId,
      TIME_MS,
      SIX2SIX_TIMEOUT_MS
   );
   opentimers_restart(sixtop_vars.timeoutTimerId);
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_WAIT_ADDREQUEST_SENDDONE %d\n",sixtop_vars.six2six_state);
#endif
}

void sixtop_removeCell(open_addr_t* neighbor){
   OpenQueueEntry_t* pkt;
   bool              outcome;
   uint8_t           len;
   uint8_t           type;
   uint8_t           frameID;
   uint8_t           flag;
   cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
   memset(cellList,0,sizeof(cellList));
   
   // filter parameters
   if (sixtop_vars.six2six_state!=SIX_IDLE){
      return;
   }
   if (neighbor==NULL){
      return;
   }
   
   // generate candidate cell list
   outcome = sixtop_candidateRemoveCellList(
      &type, 
      &frameID,
      &flag, 
      cellList, 
      neighbor
   );
   if(outcome == FALSE){
      return;
   }
   
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
   sixtop_vars.six2six_state = SIX_SENDING_REMOVEREQUEST;
   
   // declare ownership over that packet
   pkt->creator = COMPONENT_SIXTOP_RES;
   pkt->owner   = COMPONENT_SIXTOP_RES;
      
   memcpy(
      &(pkt->l2_nextORpreviousHop),
      neighbor,
      sizeof(open_addr_t)
   );
 
   
   // create packet
   len  = 0;
   len += processIE_prependScheduleIE(pkt,type,frameID, flag,cellList);
   len += processIE_prependOpcodeIE(pkt,SIXTOP_REMOVE_SOFT_CELL_REQUEST);
   processIE_prependMLMEIE(pkt,len);
 
   // indicate IEs present
   pkt->l2_payloadIEpresent = TRUE;
   
   // send packet
   sixtop_send(pkt);
   
   // update state
   sixtop_vars.six2six_state = SIX_WAIT_REMOVEREQUEST_SENDDONE;
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_WAIT_REMOVEREQUEST_SENDDONE %d\n",sixtop_vars.six2six_state);
#endif
   // arm timeout
   opentimers_setPeriod(
      sixtop_vars.timeoutTimerId,
      TIME_MS,
      SIX2SIX_TIMEOUT_MS
   );
   opentimers_restart(sixtop_vars.timeoutTimerId);
}

void sixtop_removeCellByInfo(open_addr_t*  neighbor,cellInfo_ht* cellInfo){
   OpenQueueEntry_t* pkt;
   uint8_t           len;
   uint8_t           type;
   uint8_t           frameID;
   uint8_t           flag;
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
       // sixtop handler must not be NONE
       return;
   }
   
   // set cell list. only the first one
   type           = 1;
   frameID        = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
   flag           = 1;
   memcpy(&(cellList[0]),cellInfo,sizeof(cellInfo_ht));
   
   
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
   sixtop_vars.six2six_state = SIX_SENDING_REMOVEREQUEST;
   
   // declare ownership over that packet
   pkt->creator = COMPONENT_SIXTOP_RES;
   pkt->owner   = COMPONENT_SIXTOP_RES;
      
   memcpy(
      &(pkt->l2_nextORpreviousHop),
      neighbor,
      sizeof(open_addr_t)
   );
 
   
   // create packet
   len  = 0;
   len += processIE_prependScheduleIE(pkt,type,frameID, flag,cellList);
   len += processIE_prependOpcodeIE(pkt,SIXTOP_REMOVE_SOFT_CELL_REQUEST);
   processIE_prependMLMEIE(pkt,len);
 
   // indicate IEs present
   pkt->l2_payloadIEpresent = TRUE;
   
   // send packet
   sixtop_send(pkt);
   
   // update state
   sixtop_vars.six2six_state = SIX_WAIT_REMOVEREQUEST_SENDDONE;
   
   // arm timeout
   opentimers_setPeriod(
      sixtop_vars.timeoutTimerId,
      TIME_MS,
      SIX2SIX_TIMEOUT_MS
   );
   opentimers_restart(sixtop_vars.timeoutTimerId);

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
        sixtop_removeCellByInfo(neighbor, &linkInfo);
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

void sixtop_notifyNewSlotframe(void) {
    scheduler_push_task(sixtop_checkSchedule,TASKPRIO_SIXTOP);
}
static uint8_t slotframe_count = 0;

void sixtop_checkSchedule() {
    //once every  slotframes
	if (((slotframe_count)%1)!=0){
		slotframe_count++;
		printf("skip slotframe");
		return;
	}

	slotframe_count++;

	int16_t pid_result;
    open_addr_t neighborAddress;
    uint8_t asn[5];
    memset(&neighborAddress,0,sizeof(open_addr_t));
    if (
        neighbors_getPreferredParentEui64(&neighborAddress)==FALSE || \
        idmanager_getIsDAGroot()
    ) {
        return;
    }
#ifdef PID_CELL_USAGE
    //compute cells needed 
    pid_result = pid_compute_usageOfCell();
#else
    pid_result = pid_compute_packetInQueue();
#endif
    // debug info
    if(idmanager_getMyID(ADDR_64B)->addr_64b[7] == 0x02) {
        ieee154e_getAsn(asn);
        // slotframe, numOfslot(Tx), numOfpacketInQueue
        printf("%d, %d, %d, %d\n",(asn[0]+256*asn[1]+65536*asn[2])/schedule_getFrameLength(),schedule_getNumOfActiveSlot(),pid_result,openqueue_getNumOfPakcetToParent());
//        debugprint_schedule_slotOffset_numOfTx_numOfTxAck();
    }
#ifdef PID_CELL_USAGE
    //reserve cells
    if (pid_result > TARGET_USAGE_RANGE) {
        sixtop_addCells(&neighborAddress,1);
    } else {
        if (pid_result < -TARGET_USAGE_RANGE){
         sixtop_removeCell(&neighborAddress);   
        } else {
            // I am in the target range{-TARGET_RANGE, TARGET_RANGE}, nothing to do
        }
    }
#else
    //reserve cells
    if (pid_result > 1) {
        sixtop_addCells(&neighborAddress,1);
    } else {
        if (pid_result < 0){
         sixtop_removeCell(&neighborAddress);   
        } else {
            // hit the target, nothing to do
        }
    }
#endif
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
   
   output = neighbors_getMyDAGrank();
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
   
   len = 0;
   
   if ((ieee154e_isSynch()==FALSE) || (neighbors_getMyDAGrank()==DEFAULTDAGRANK)){
      // I'm not sync'ed or I did not acquire a DAGrank
      
      // delete packets genereted by this module (EB and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm now busy sending an EB
      sixtop_vars.busySendingEB = FALSE;
      
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
      
      // I'm now busy sending a KA
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
   sixtop_vars.six2six_state = SIX_IDLE;
   sixtop_vars.handler = SIX_HANDLER_NONE;
   opentimers_stop(sixtop_vars.timeoutTimerId);
}

void sixtop_six2six_sendDone(OpenQueueEntry_t* msg, owerror_t error){
   uint8_t i,numOfCells;
   uint8_t* ptr;
   cellInfo_ht cellList[SCHEDULEIEMAXNUMCELLS];
   
   memset(cellList,0,SCHEDULEIEMAXNUMCELLS*sizeof(cellInfo_ht));
  
   ptr = msg->l2_scheduleIE_cellObjects;
   numOfCells = msg->l2_scheduleIE_numOfCells;
   msg->owner = COMPONENT_SIXTOP_RES;
  
   if(error == E_FAIL) {
      sixtop_vars.six2six_state = SIX_IDLE;
      opentimers_stop(sixtop_vars.timeoutTimerId);
      openqueue_freePacketBuffer(msg);
      return;
   }

   switch (sixtop_vars.six2six_state) {
      case SIX_WAIT_ADDREQUEST_SENDDONE:
         sixtop_vars.six2six_state = SIX_WAIT_ADDRESPONSE;
         break;
      case SIX_WAIT_ADDRESPONSE_SENDDONE:
         if (error == E_SUCCESS && numOfCells > 0){
             for (i=0;i<numOfCells;i++){
               //TimeSlot 2B
               cellList[i].tsNum       = *(ptr);
               cellList[i].tsNum      |= (*(ptr+1))<<8;
               //Ch.Offset 2B
               cellList[i].choffset    = *(ptr+2);
               cellList[i].choffset   |= (*(ptr+3))<<8;
               //LinkOption bitmap 1B
               cellList[i].linkoptions = *(ptr+4);
               ptr += 5;
             }
             sixtop_addCellsByState(
                 msg->l2_scheduleIE_frameID,
                 numOfCells,
                 cellList,
                 &(msg->l2_nextORpreviousHop),
                 sixtop_vars.six2six_state);
         }
         sixtop_vars.six2six_state = SIX_IDLE;
         break;
      case SIX_WAIT_REMOVEREQUEST_SENDDONE:
         if(error == E_SUCCESS && numOfCells > 0){
            for (i=0;i<numOfCells;i++){
               //TimeSlot 2B
               cellList[i].tsNum       = *(ptr);
               cellList[i].tsNum      |= (*(ptr+1))<<8;
               //Ch.Offset 2B
               cellList[i].choffset    = *(ptr+2);
               cellList[i].choffset   |= (*(ptr+3))<<8;
               //LinkOption bitmap 1B
               cellList[i].linkoptions = *(ptr+4);
               ptr += 5;
            }
            sixtop_removeCellsByState(
               msg->l2_scheduleIE_frameID,
               numOfCells,
               cellList,
               &(msg->l2_nextORpreviousHop)
            );
         }
         sixtop_vars.six2six_state = SIX_IDLE;
#ifdef SIXTOP_DEBUGINFO
         printf("SIX_IDLE %d\n",sixtop_vars.six2six_state);
#endif
         opentimers_stop(sixtop_vars.timeoutTimerId);
         leds_debug_off();
         if (sixtop_vars.handler == SIX_HANDLER_MAINTAIN){
             sixtop_addCells(&(msg->l2_nextORpreviousHop),1);
             sixtop_vars.handler = SIX_HANDLER_NONE;
         }
         break;
      default:
         //log error
         break;
   }
  
   // discard reservation packets this component has created
   openqueue_freePacketBuffer(msg);
}

port_INLINE bool sixtop_processIEs(OpenQueueEntry_t* pkt, uint16_t * lenIE) {
   uint8_t ptr;
   uint8_t temp_8b,gr_elem_id,subid;
   uint16_t temp_16b,len,sublen;
   opcode_IE_ht opcode_ie;
   bandwidth_IE_ht bandwidth_ie;
   schedule_IE_ht schedule_ie;
 
   ptr=0; 
   memset(&opcode_ie,0,sizeof(opcode_IE_ht));
   memset(&bandwidth_ie,0,sizeof(bandwidth_IE_ht));
   memset(&schedule_ie,0,sizeof(schedule_IE_ht));  
  
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
              case MLME_IE_SUBID_OPCODE:
              processIE_retrieveOpcodeIE(pkt,&ptr,&opcode_ie);
              break;
              case MLME_IE_SUBID_BANDWIDTH:
              processIE_retrieveBandwidthIE(pkt,&ptr,&bandwidth_ie);
              break;
              case MLME_IE_SUBID_TRACKID:
              break;
              case MLME_IE_SUBID_SCHEDULE:
              processIE_retrieveScheduleIE(pkt,&ptr,&schedule_ie);
              break;
          default:
             return FALSE;
             break;
        }
        len = len - sublen;
      } while(len>0);
      
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
  
   if(*lenIE>0) {
      sixtop_notifyReceiveCommand(&opcode_ie,
                                  &bandwidth_ie,
                                  &schedule_ie,
                                  &(pkt->l2_nextORpreviousHop));
   }
  
  return TRUE;
}

void sixtop_notifyReceiveCommand(
   opcode_IE_ht* opcode_ie, 
   bandwidth_IE_ht* bandwidth_ie, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   switch(opcode_ie->opcode){
      case SIXTOP_SOFT_CELL_REQ:
         if(sixtop_vars.six2six_state == SIX_IDLE)
         {
            sixtop_vars.six2six_state = SIX_ADDREQUEST_RECEIVED;
            //received uResCommand is reserve link request
            sixtop_notifyReceiveLinkRequest(bandwidth_ie,schedule_ie,addr);
         }
         break;
      case SIXTOP_SOFT_CELL_RESPONSE:
         if(sixtop_vars.six2six_state == SIX_WAIT_ADDRESPONSE){
           sixtop_vars.six2six_state = SIX_ADDRESPONSE_RECEIVED;
           //received uResCommand is reserve link response
           sixtop_notifyReceiveLinkResponse(bandwidth_ie,schedule_ie,addr);
         }
         break;
      case SIXTOP_REMOVE_SOFT_CELL_REQUEST:
         if(sixtop_vars.six2six_state == SIX_IDLE){
            sixtop_vars.six2six_state = SIX_REMOVEREQUEST_RECEIVED;
          //received uResComand is remove link request
             sixtop_notifyReceiveRemoveLinkRequest(schedule_ie,addr);
        }
        break;
      default:
         // log the error
         break;
      }
}

void sixtop_notifyReceiveLinkRequest(
   bandwidth_IE_ht* bandwidth_ie, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
#ifdef SIXTOP_DEBUGINFO
   printf("SIXTOP_LINKREQUEST_RECEIVED \n");
#endif
   uint8_t bw,numOfcells,frameID;
   bool scheduleCellSuccess;
  
   frameID = schedule_ie->frameID;
   numOfcells = schedule_ie->numberOfcells;
   bw = bandwidth_ie->numOfLinks;
   
   // need to check whether the links are available to be scheduled.
   if(bw > numOfcells                                                 ||
      schedule_ie->frameID != bandwidth_ie->slotframeID               ||
      sixtop_areAvailableCellsToBeScheduled(frameID,
                                            numOfcells,
                                            schedule_ie->cellList, 
                                            bw) == FALSE){
      scheduleCellSuccess = FALSE;
   } else {
      scheduleCellSuccess = TRUE;
   }
  
   //call link response command
   sixtop_linkResponse(scheduleCellSuccess,
                       addr,
                       bandwidth_ie->numOfLinks,
                       schedule_ie);
}

void sixtop_linkResponse(
   bool scheduleCellSuccess, 
   open_addr_t* tempNeighbor,
   uint8_t bandwidth, 
   schedule_IE_ht* schedule_ie){
   
   OpenQueueEntry_t* sixtopPkt;
   uint8_t len=0;
   uint8_t bw;
   uint8_t type,frameID,flag;
   cellInfo_ht* cellList;
    
   // get parameters for scheduleIE
   type = schedule_ie->type;
   frameID = schedule_ie->frameID;
   flag = schedule_ie->flag;
   cellList = schedule_ie->cellList;
  
   // get a free packet buffer
   sixtopPkt = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP_RES);
  
   if(sixtopPkt==NULL) {
      openserial_printError(COMPONENT_SIXTOP_RES,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    
   // changing state to resLinkRespone command
   sixtop_vars.six2six_state = SIX_SENDING_ADDRESPONSE;
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_SENDING_ADDRESPONSE %d\n",sixtop_vars.six2six_state);
#endif
   // declare ownership over that packet
   sixtopPkt->creator = COMPONENT_SIXTOP_RES;
   sixtopPkt->owner   = COMPONENT_SIXTOP_RES;
    
   memcpy(&(sixtopPkt->l2_nextORpreviousHop),tempNeighbor,sizeof(open_addr_t));
   // set SubFrameAndLinkIE
   len += processIE_prependScheduleIE(sixtopPkt,
                                                  type,
                                                  frameID,
                                                  flag,
                                                  cellList);
    
   if(scheduleCellSuccess){
      bw = bandwidth;
   } else {
      bw = 0;
   }
   //add BandwidthIE
   len += processIE_prependBandwidthIE(sixtopPkt,bw,frameID);
   //add opcodeIE
   len += processIE_prependOpcodeIE(sixtopPkt,SIXTOP_SOFT_CELL_RESPONSE);
   //add IE header 
   processIE_prependMLMEIE(sixtopPkt,len);
    
   //I has an IE in my payload
   sixtopPkt->l2_payloadIEpresent = TRUE;
  
   sixtop_send(sixtopPkt);
  
   sixtop_vars.six2six_state = SIX_WAIT_ADDRESPONSE_SENDDONE;
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_WAIT_ADDRESPONSE_SENDDONE %d \n",sixtop_vars.six2six_state);
#endif
}

void sixtop_notifyReceiveLinkResponse(
   bandwidth_IE_ht* bandwidth_ie, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   
   uint8_t bw,numOfcells,frameID;
  
   frameID = schedule_ie->frameID;
   numOfcells = schedule_ie->numberOfcells;
   bw = bandwidth_ie->numOfLinks;
#ifdef SIXTOP_DEBUGINFO
   printf("SIXTOP_RECEIVED_LINKRESPONSE\n");
#endif
   if(bw == 0){
      // link request failed
      // todo- should inform some one
      return;
   } else {
      // need to check whether the links are available to be scheduled.
      if(bw != numOfcells                                                ||
         schedule_ie->frameID != bandwidth_ie->slotframeID               ||
         sixtop_areAvailableCellsToBeScheduled(frameID, 
                                               numOfcells, 
                                               schedule_ie->cellList, 
                                               bw) == FALSE){
         // link request failed,inform uplayer
      } else {
         sixtop_addCellsByState(frameID,
                                bw,
                                schedule_ie->cellList,
                                addr,
                                sixtop_vars.six2six_state);
      // link request success,inform uplayer
      }
   }
   leds_debug_off();
   sixtop_vars.six2six_state = SIX_IDLE;
   sixtop_vars.handler = SIX_HANDLER_NONE;
#ifdef SIXTOP_DEBUGINFO
   printf("SIX_IDLE %d \n",sixtop_vars.six2six_state);
#endif
   opentimers_stop(sixtop_vars.timeoutTimerId);
}

void sixtop_notifyReceiveRemoveLinkRequest(
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   
   uint8_t numOfCells,frameID;
   cellInfo_ht* cellList;
  
   numOfCells = schedule_ie->numberOfcells;
   frameID = schedule_ie->frameID;
   cellList = schedule_ie->cellList;
   
   leds_debug_on();
   
   sixtop_removeCellsByState(frameID,numOfCells,cellList,addr);
   
   if (sixtop_vars.handler == SIX_HANDLER_MAINTAIN) {
       // if sixtop remove request handler is 
       sixtop_vars.handler = SIX_HANDLER_NONE;
   } else {
       if (sixtop_vars.handler == SIX_HANDLER_PID) {
           sixtop_vars.handler = SIX_HANDLER_NONE;
       } else {
           // if any other handlers exist
       }
   }
   
   sixtop_vars.six2six_state = SIX_IDLE;

   leds_debug_off();
}

//======= helper functions

bool sixtop_candidateAddCellList(
      uint8_t*     type,
      uint8_t*     frameID,
      uint8_t*     flag,
      cellInfo_ht* cellList
   ){
   frameLength_t i;
   uint8_t counter;
   uint8_t numCandCells;
   
   *type = 1;
   *frameID = schedule_getFrameHandle();
   *flag = 1; // the cells listed in cellList are available to be schedule.
   
   numCandCells=0;
   for(counter=0;counter<SCHEDULEIEMAXNUMCELLS;counter++){
      i = openrandom_get16b()%schedule_getFrameLength();
      if(schedule_isSlotOffsetAvailable(i)==TRUE){
         cellList[numCandCells].tsNum       = i;
         cellList[numCandCells].choffset    = 0;
         cellList[numCandCells].linkoptions = CELLTYPE_TX;
         numCandCells++;
      }
   }
   
   if (numCandCells==0) {
      return FALSE;
   } else {
      return TRUE;
   }
}

bool sixtop_candidateRemoveCellList(
      uint8_t*     type,
      uint8_t*     frameID,
      uint8_t*     flag,
      cellInfo_ht* cellList,
      open_addr_t* neighbor
   ){
   frameLength_t        i;
   uint8_t              numCandCells;
   slotinfo_element_t   info;
   
   *type           = 1;
   *frameID        = schedule_getFrameHandle();
   *flag           = 1;
  
   numCandCells    = 0;
   for(i=0;i<schedule_getFrameLength();i++){
      schedule_getSlotInfo(i,neighbor,&info);
      if(info.link_type == CELLTYPE_TX){
         cellList[numCandCells].tsNum       = i;
         cellList[numCandCells].choffset    = info.channelOffset;
         cellList[numCandCells].linkoptions = CELLTYPE_TX;
         numCandCells++;
         break; // only delete one cell
      }
   }
   
   if(numCandCells==0){
      return FALSE;
   }else{
      return TRUE;
   }
}

void sixtop_addCellsByState(
      uint8_t      slotframeID,
      uint8_t      numOfLinks,
      cellInfo_ht* cellList,
      open_addr_t* previousHop,
      uint8_t      state
   ){
   uint8_t     i;
   uint8_t     j;
   open_addr_t temp_neighbor;
  
   //set schedule according links
   
   j=0;
   for(i = 0;i<SCHEDULEIEMAXNUMCELLS;i++){
      //only schedule when the request side wants to schedule a tx cell
      if(cellList[i].linkoptions == CELLTYPE_TX){
         switch(state) {
            case SIX_WAIT_ADDRESPONSE_SENDDONE:
               memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
               
               //add a RX link
               schedule_addActiveSlot(
                  cellList[i].tsNum,
                  CELLTYPE_RX,
                  FALSE,
                  cellList[i].choffset,
                  &temp_neighbor
               );
               
               break;
            case SIX_ADDRESPONSE_RECEIVED:
               memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
               //add a TX link
               schedule_addActiveSlot(
                  cellList[i].tsNum,
                  CELLTYPE_TX,
                  FALSE,
                  cellList[i].choffset,
                  &temp_neighbor
               );
               break;
            default:
               //log error
               break;
         }
         j++;
         if(j==numOfLinks){
            break;
         }
      }
   }
}

void sixtop_removeCellsByState(
      uint8_t      slotframeID,
      uint8_t      numOfLink,
      cellInfo_ht* cellList,
      open_addr_t* previousHop
   ){
   uint8_t i;
   
   for(i=0;i<numOfLink;i++){   
      if(cellList[i].linkoptions == CELLTYPE_TX){
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
      cellInfo_ht* cellList, 
      uint8_t      bandwidth
   ){
   uint8_t i;
   uint8_t bw;
   bool    available;
   
   i          = 0;
   bw         = bandwidth;
   available  = FALSE;
  
   if(bw == 0 || bw>SCHEDULEIEMAXNUMCELLS || numOfCells>SCHEDULEIEMAXNUMCELLS){
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
      }while(i<numOfCells && bw>0);
      
      if(bw==0){
         //the rest link will not be scheduled, mark them as off type
         while(i<numOfCells){
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
