/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:23.722130.
*/
#include "openwsn_obj.h"
#include "sixtop_obj.h"
#include "openserial_obj.h"
#include "openqueue_obj.h"
#include "neighbors_obj.h"
#include "IEEE802154E_obj.h"
#include "iphc_obj.h"
#include "packetfunctions_obj.h"
#include "openrandom_obj.h"
#include "scheduler_obj.h"
#include "opentimers_obj.h"
#include "debugpins_obj.h"
#include "leds_obj.h"
#include "processIE_obj.h"
#include "IEEE802154_obj.h"
#include "idmanager_obj.h"
#include "schedule_obj.h"
//START OF TELEMATICS CODE
#include "security.h"
//END OF TELEMATICS CODE

//=========================== variables =======================================

// declaration of global variable _sixtop_vars_ removed during objectification.

//=========================== prototypes ======================================

// send internal
owerror_t sixtop_send_internal(OpenMote* self, 
   OpenQueueEntry_t*    msg,
   uint8_t              iePresent,
   uint8_t              frameVersion
);

// timer interrupt callbacks
void sixtop_maintenance_timer_cb(OpenMote* self);
void sixtop_timeout_timer_cb(OpenMote* self);

//=== EB/KA task

void timer_sixtop_management_fired(OpenMote* self);
void sixtop_sendEB(OpenMote* self);
void sixtop_sendKA(OpenMote* self);

//=== six2six task

void timer_sixtop_six2six_timeout_fired(OpenMote* self);
void sixtop_six2six_sendDone(OpenMote* self, 
   OpenQueueEntry_t*    msg,
   owerror_t            error
);
bool sixtop_processIEs(OpenMote* self, 
   OpenQueueEntry_t*    pkt,
   uint16_t*            lenIE
);
void sixtop_notifyReceiveCommand(OpenMote* self, 
   opcode_IE_ht*        opcode_ie, 
   bandwidth_IE_ht*     bandwidth_ie, 
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);
void sixtop_notifyReceiveLinkRequest(OpenMote* self, 
   bandwidth_IE_ht*     bandwidth_ie,
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);
void sixtop_linkResponse(OpenMote* self, 
   bool                 success,
   open_addr_t*         tempNeighbor,
   uint8_t              bandwidth,
   schedule_IE_ht*      schedule_ie
);
void sixtop_notifyReceiveLinkResponse(OpenMote* self, 
   bandwidth_IE_ht*     bandwidth_ie,
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);
void sixtop_notifyReceiveRemoveLinkRequest(OpenMote* self, 
   schedule_IE_ht*      schedule_ie,
   open_addr_t*         addr
);

//=== helper functions

bool sixtop_candidateAddCellList(OpenMote* self, 
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList
);
bool sixtop_candidateRemoveCellList(OpenMote* self, 
   uint8_t*             type,
   uint8_t*             frameID,
   uint8_t*             flag,
   cellInfo_ht*         cellList,
   open_addr_t*         neighbor
);
void sixtop_addCellsByState(OpenMote* self, 
   uint8_t              slotframeID,
   uint8_t              numOfLinks,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop,
   uint8_t              state
);
void sixtop_removeCellsByState(OpenMote* self, 
   uint8_t              slotframeID,
   uint8_t              numOfLink,
   cellInfo_ht*         cellList,
   open_addr_t*         previousHop
);
bool sixtop_areAvailableCellsToBeScheduled(OpenMote* self, 
   uint8_t              frameID, 
   uint8_t              numOfCells, 
   cellInfo_ht*         cellList, 
   uint8_t              bandwidth
);

//=========================== public ==========================================

void sixtop_init(OpenMote* self) {
   
   (self->sixtop_vars).periodMaintenance  = 872 +( openrandom_get16b(self)&0xff);
   (self->sixtop_vars).busySendingKA      = FALSE;
   (self->sixtop_vars).busySendingEB      = FALSE;
   (self->sixtop_vars).dsn                = 0;
   (self->sixtop_vars).mgtTaskCounter     = 0;
   (self->sixtop_vars).kaPeriod           = MAXKAPERIOD;
   
   (self->sixtop_vars).maintenanceTimerId = opentimers_start(self, 
      (self->sixtop_vars).periodMaintenance,
      TIMER_PERIODIC,
      TIME_MS,
      sixtop_maintenance_timer_cb
   );
   
   (self->sixtop_vars).timeoutTimerId     = opentimers_start(self, 
      SIX2SIX_TIMEOUT_MS,
      TIMER_ONESHOT,
      TIME_MS,
      sixtop_timeout_timer_cb
   );
}

void sixtop_setKaPeriod(OpenMote* self, uint16_t kaPeriod) {
   if(kaPeriod > MAXKAPERIOD) {
      (self->sixtop_vars).kaPeriod = MAXKAPERIOD;
   } else {
      (self->sixtop_vars).kaPeriod = kaPeriod;
   } 
}

//======= scheduling

void sixtop_addCells(OpenMote* self, open_addr_t* neighbor, uint16_t numCells){
   OpenQueueEntry_t* pkt;
   uint8_t           len;
   uint8_t           type;
   uint8_t           frameID;
   uint8_t           flag;
   bool              outcome;
   cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
   frameID    = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
   
   memset(cellList,0,sizeof(cellList));
   
   // filter parameters
   if((self->sixtop_vars).six2six_state!=SIX_IDLE){
      return;
   }
   if (neighbor==NULL){
      return;
   }
  
   // generate candidate cell list
   outcome = sixtop_candidateAddCellList(self, 
      &type,
      &frameID,
      &flag,
      cellList
   );
   if (outcome == FALSE) {
     return;
   }
   
   // get a free packet buffer
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_SIXTOP_RES);
   if (pkt==NULL) {
 openserial_printError(self, 
         COMPONENT_SIXTOP_RES,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }
   
   // update state
   (self->sixtop_vars).six2six_state  = SIX_SENDING_ADDREQUEST;
   
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
   len += processIE_prependSheduleIE(self, pkt,type,frameID,flag,cellList);
   len += processIE_prependBandwidthIE(self, pkt,numCells,frameID);
   len += processIE_prependOpcodeIE(self, pkt,SIXTOP_SOFT_CELL_REQ);
 processIE_prependMLMEIE(self, pkt,len);
   
   // indicate IEs present
   pkt->l2_IEListPresent = IEEE154_IELIST_YES;
   
   // send packet
 sixtop_send(self, pkt);
   
   // update state
   (self->sixtop_vars).six2six_state = SIX_WAIT_ADDREQUEST_SENDDONE;
   
   // arm timeout
 opentimers_setPeriod(self, 
      (self->sixtop_vars).timeoutTimerId,
      TIME_MS,
      SIX2SIX_TIMEOUT_MS
   );
 opentimers_restart(self, (self->sixtop_vars).timeoutTimerId);
}

void sixtop_removeCell(OpenMote* self, open_addr_t* neighbor){
   OpenQueueEntry_t* pkt;
   bool              outcome;
   uint8_t           len;
   uint8_t           type;
   uint8_t           frameID;
   uint8_t           flag;
   cellInfo_ht       cellList[SCHEDULEIEMAXNUMCELLS];
   
   memset(cellList,0,sizeof(cellList));
   
   // filter parameters
   if ((self->sixtop_vars).six2six_state!=SIX_IDLE){
      return;
   }
   if (neighbor==NULL){
      return;
   }
   
   // generate candidate cell list
   outcome = sixtop_candidateRemoveCellList(self, 
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
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_SIXTOP_RES);
   if(pkt==NULL) {
 openserial_printError(self, 
         COMPONENT_SIXTOP_RES,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }
   
   // update state
   (self->sixtop_vars).six2six_state = SIX_SENDING_REMOVEREQUEST;
   
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
   len += processIE_prependSheduleIE(self, pkt,type,frameID, flag,cellList);
   len += processIE_prependOpcodeIE(self, pkt,SIXTOP_REMOVE_SOFT_CELL_REQUEST);
 processIE_prependMLMEIE(self, pkt,len);
 
   // indicate IEs present
   pkt->l2_IEListPresent = IEEE154_IELIST_YES;
   
   // send packet
 sixtop_send(self, pkt);
   
   // update state
   (self->sixtop_vars).six2six_state = SIX_WAIT_REMOVEREQUEST_SENDDONE;
   
   // arm timeout
 opentimers_setPeriod(self, 
      (self->sixtop_vars).timeoutTimerId,
      TIME_MS,
      SIX2SIX_TIMEOUT_MS
   );
 opentimers_restart(self, (self->sixtop_vars).timeoutTimerId);
}

//======= from upper layer

owerror_t sixtop_send(OpenMote* self, OpenQueueEntry_t *msg) {
   
   // set metadata
   msg->owner        = COMPONENT_SIXTOP;
   msg->l2_frameType = IEEE154_TYPE_DATA;

    //START OF TELEMATICS CODE
     msg->l2_security = TRUE;
     msg->l2_securityLevel = 5;
     msg->l2_keyIdMode = 3;
     if( idmanager_getIsDAGroot(self)){
      open_addr_t* temp_addr;
      temp_addr = idmanager_getMyID(self, ADDR_64B);
      memcpy(&(msg->l2_keySource), temp_addr, sizeof(open_addr_t));
     }else{
 neighbors_getPreferredParentEui64(self, &(msg->l2_keySource));
     }
     msg->l2_keyIndex = 1;
     //END OF TELEMATICS CODE
   
   if (msg->l2_IEListPresent == IEEE154_IELIST_NO) {
      return sixtop_send_internal(self, 
         msg,
         IEEE154_IELIST_NO,
         IEEE154_FRAMEVERSION_2006
      );
   } else {
      return sixtop_send_internal(self, 
         msg,
         IEEE154_IELIST_YES,
         IEEE154_FRAMEVERSION
      );
   }
}

//======= from lower layer

void task_sixtopNotifSendDone(OpenMote* self) {
   OpenQueueEntry_t* msg;
   
   // get recently-sent packet from openqueue
   msg = openqueue_sixtopGetSentPacket(self);
   if (msg==NULL) {
 openserial_printCritical(self, 
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
 neighbors_indicateTx(self, 
         &(msg->l2_nextORpreviousHop),
         msg->l2_numTxAttempts,
         TRUE,
         &msg->l2_asn
      );
   } else {
 neighbors_indicateTx(self, 
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
            // this is a ADV
            
            // not busy sending ADV anymore
            (self->sixtop_vars).busySendingEB = FALSE;
         } else {
            // this is a KA
            
            // not busy sending KA anymore
            (self->sixtop_vars).busySendingKA = FALSE;
         }
         // discard packets
 openqueue_freePacketBuffer(self, msg);
         
         // restart a random timer
         (self->sixtop_vars).periodMaintenance = 872+( openrandom_get16b(self)&0xff);
 opentimers_setPeriod(self, 
            (self->sixtop_vars).maintenanceTimerId,
            TIME_MS,
            (self->sixtop_vars).periodMaintenance
         );
         break;
      
      case COMPONENT_SIXTOP_RES:
 sixtop_six2six_sendDone(self, msg,msg->l2_sendDoneError);
         break;
      
      default:
         // send the rest up the stack
 iphc_sendDone(self, msg,msg->l2_sendDoneError);
         break;
   }
}

void task_sixtopNotifReceive(OpenMote* self) {
   OpenQueueEntry_t* msg;
   uint16_t          lenIE;
   
   // get received packet from openqueue
   msg = openqueue_sixtopGetReceivedPacket(self);
   if (msg==NULL) {
 openserial_printCritical(self, 
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
         msg->l2_frameType==IEEE154_TYPE_DATA            &&
         msg->l2_IEListPresent==IEEE154_IELIST_YES       &&
 sixtop_processIEs(self, msg, &lenIE) == FALSE
      ) {
      // free the packet's RAM memory
 openqueue_freePacketBuffer(self, msg);
      //log error
      return;
   }
   
   // toss the header IEs
 packetfunctions_tossHeader(self, msg,lenIE);
   
   // update neighbor statistics
 neighbors_indicateRx(self, 
      &(msg->l2_nextORpreviousHop),
      msg->l1_rssi,
      &msg->l2_asn,
      msg->l2_joinPriorityPresent,
      msg->l2_joinPriority
   );
   
   // reset it to avoid race conditions with this var.
   msg->l2_joinPriorityPresent = FALSE;

 //START OF TELEMATICS CODE
      if(msg->l2_security== TRUE){
       security_incomingFrame(msg);
      }
      //END OF TELEMATICS CODE

   // send the packet up the stack, if it qualifies
   switch (msg->l2_frameType) {
      case IEEE154_TYPE_BEACON:
      case IEEE154_TYPE_DATA:
      case IEEE154_TYPE_CMD:
    	  if (msg->length>0) {
    	          	 //START OF TELEMATICS CODE
    	  			  //discard duplicated packets
    	  			  if(msg->l2_toDiscard == FALSE){
    	  			  //END OF TELEMATICS CODE
    	  					  // send to upper layer
 iphc_receive(self, msg);
    	  				  }
    	               else {
    	              // free up the RAM
 openqueue_freePacketBuffer(self, msg);
    	           }
    	        }else{
    	          	 // free up the RAM
 openqueue_freePacketBuffer(self, msg);
    	           }
    	           break;
      case IEEE154_TYPE_ACK:
      default:
         // free the packet's RAM memory
 openqueue_freePacketBuffer(self, msg);
         // log the error
 openserial_printError(self, 
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
bool debugPrint_myDAGrank(OpenMote* self) {
   uint16_t output;
   
   output = 0;
   
   output = neighbors_getMyDAGrank(self);
 openserial_printStatus(self, STATUS_DAGRANK,(uint8_t*)&output,sizeof(uint16_t));
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
owerror_t sixtop_send_internal(OpenMote* self, 
   OpenQueueEntry_t* msg, 
   uint8_t iePresent, 
   uint8_t frameVersion) {

   // assign a number of retries
   if (
 packetfunctions_isBroadcastMulticast(self, &(msg->l2_nextORpreviousHop))==TRUE
      ) {
      msg->l2_retriesLeft = 1;
   } else {
      msg->l2_retriesLeft = TXRETRIES;
   }
   // record this packet's dsn (for matching the ACK)
   msg->l2_dsn = (self->sixtop_vars).dsn++;
   // this is a new packet which I never attempted to send
   msg->l2_numTxAttempts = 0;
   // transmit with the default TX power
   msg->l1_txPower = TX_POWER;
   // record the location, in the packet, where the l2 payload starts
   msg->l2_payload = msg->payload;
    //START OF TELEMATICS CODE
       if(msg->l2_security == IEEE154_SEC_YES_SECURITY){

    	   security_outgoingFrame(msg,msg->l2_securityLevel,msg->l2_keyIdMode,&msg->l2_keySource,msg->l2_keyIndex);
       }
       //END OF TELEMATICS CODE

   // add a IEEE802.15.4 header
 ieee802154_prependHeader(self, msg,
                            msg->l2_frameType,
                            iePresent,
                            frameVersion,
                            //START OF TELEMATICS CODE
							msg->l2_security,
							//END OF TELEMATICS CODE
                            msg->l2_dsn,
                            &(msg->l2_nextORpreviousHop)
                            );
   // reserve space for 2-byte CRC
 packetfunctions_reserveFooterSize(self, msg,2);
   // change owner to IEEE802154E fetches it from queue
   msg->owner  = COMPONENT_SIXTOP_TO_IEEE802154E;
   return E_SUCCESS;
}

// timer interrupt callbacks

void sixtop_maintenance_timer_cb(OpenMote* self) {
 scheduler_push_task(self, timer_sixtop_management_fired,TASKPRIO_SIXTOP);
}

void sixtop_timeout_timer_cb(OpenMote* self) {
 scheduler_push_task(self, timer_sixtop_six2six_timeout_fired,TASKPRIO_SIXTOP_TIMEOUT);
}

//======= EB/KA task

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timer_sixtop_management_fired(OpenMote* self) {
   (self->sixtop_vars).mgtTaskCounter = ((self->sixtop_vars).mgtTaskCounter+1)%ADVTIMEOUT;
   
   switch ((self->sixtop_vars).mgtTaskCounter) {
      case 0:
         // called every ADVTIMEOUT seconds
 sixtop_sendEB(self);
         break;
      case 1:
         // called every ADVTIMEOUT seconds
 neighbors_removeOld(self);
         break;
      default:
         // called every second, except twice every ADVTIMEOUT seconds
 sixtop_sendKA(self);
         break;
   }
}

/**
\brief Send an advertisement.

This is one of the MAC management tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sixtop_sendEB(OpenMote* self) {
   OpenQueueEntry_t* adv;
   uint8_t len;
   
   len = 0;
   
   if ( ieee154e_isSynch(self)==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (ADV and KA) from openqueue
 openqueue_removeAllCreatedBy(self, COMPONENT_SIXTOP);
      
      // I'm now busy sending an ADV
      (self->sixtop_vars).busySendingEB = FALSE;
      
      // stop here
      return;
   }
   
   if ((self->sixtop_vars).busySendingEB==TRUE) {
      // don't continue if I'm still sending a previous ADV
   }
   
   // if I get here, I will send an ADV
   
   // get a free packet buffer
   adv = openqueue_getFreePacketBuffer(self, COMPONENT_SIXTOP);
   if (adv==NULL) {
 openserial_printError(self, COMPONENT_SIXTOP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   adv->creator = COMPONENT_SIXTOP;
   adv->owner   = COMPONENT_SIXTOP;

   //START OF TELEMATICS CODE
  adv->l2_security 					 = FALSE;
  //END OF TELEMATICS CODE

   
   // reserve space for ADV-specific header
   // reserving for IEs.
   len += processIE_prependSlotframeLinkIE(self, adv);
   len += processIE_prependSyncIE(self, adv);
   
   //add IE header 
 processIE_prependMLMEIE(self, adv,len);
  
   // some l2 information about this packet
   adv->l2_frameType                     = IEEE154_TYPE_BEACON;
   adv->l2_nextORpreviousHop.type        = ADDR_16B;
   adv->l2_nextORpreviousHop.addr_16b[0] = 0xff;
   adv->l2_nextORpreviousHop.addr_16b[1] = 0xff;
   
   //I has an IE in my payload
   adv->l2_IEListPresent = IEEE154_IELIST_YES;
   
   // put in queue for MAC to handle
 sixtop_send_internal(self, adv,IEEE154_IELIST_YES,IEEE154_FRAMEVERSION);
   
   // I'm now busy sending an ADV
   (self->sixtop_vars).busySendingEB = TRUE;
}

/**
\brief Send an keep-alive message, if necessary.

This is one of the MAC management tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sixtop_sendKA(OpenMote* self) {
   OpenQueueEntry_t* kaPkt;
   open_addr_t*      kaNeighAddr;
   
   if ( ieee154e_isSynch(self)==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (ADV and KA) from openqueue
 openqueue_removeAllCreatedBy(self, COMPONENT_SIXTOP);
      
      // I'm now busy sending a KA
      (self->sixtop_vars).busySendingKA = FALSE;
      
      // stop here
      return;
   }
   
   if ((self->sixtop_vars).busySendingKA==TRUE) {
      // don't proceed if I'm still sending a KA
      return;
   }
   
   kaNeighAddr = neighbors_getKANeighbor(self, (self->sixtop_vars).kaPeriod);
   if (kaNeighAddr==NULL) {
      // don't proceed if I have no neighbor I need to send a KA to
      return;
   }
   
   // if I get here, I will send a KA
   
   // get a free packet buffer
   kaPkt = openqueue_getFreePacketBuffer(self, COMPONENT_SIXTOP);
   if (kaPkt==NULL) {
 openserial_printError(self, COMPONENT_SIXTOP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)1,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   kaPkt->creator = COMPONENT_SIXTOP;
   kaPkt->owner   = COMPONENT_SIXTOP;

   //START OF TELEMATICS CODE
      kaPkt->l2_security = FALSE;
      //END OF TELEMATICS CODE
   
   // some l2 information about this packet
   kaPkt->l2_frameType = IEEE154_TYPE_DATA;
   memcpy(&(kaPkt->l2_nextORpreviousHop),kaNeighAddr,sizeof(open_addr_t));
   
   // put in queue for MAC to handle
 sixtop_send_internal(self, kaPkt,IEEE154_IELIST_NO,IEEE154_FRAMEVERSION_2006);
   
   // I'm now busy sending a KA
   (self->sixtop_vars).busySendingKA = TRUE;

#ifdef OPENSIM
 debugpins_ka_set(self);
 debugpins_ka_clr(self);
#endif
}

//======= six2six task

void timer_sixtop_six2six_timeout_fired(OpenMote* self) {
   // timeout timer fired, reset the state of sixtop to idle
   (self->sixtop_vars).six2six_state = SIX_IDLE;
}

void sixtop_six2six_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error){
   uint8_t i,numOfCells;
   uint8_t* ptr;
   cellInfo_ht cellList[SCHEDULEIEMAXNUMCELLS];
   
   memset(cellList,0,SCHEDULEIEMAXNUMCELLS*sizeof(cellInfo_ht));
  
   ptr = msg->l2_scheduleIE_cellObjects;
   numOfCells = msg->l2_scheduleIE_numOfCells;
   msg->owner = COMPONENT_SIXTOP_RES;
  
   if(error == E_FAIL) {
      (self->sixtop_vars).six2six_state = SIX_IDLE;
 openqueue_freePacketBuffer(self, msg);
      return;
   }

   switch ((self->sixtop_vars).six2six_state)
   {
   case SIX_WAIT_ADDREQUEST_SENDDONE:
      (self->sixtop_vars).six2six_state = SIX_WAIT_ADDRESPONSE;
      break;
   case SIX_WAIT_ADDRESPONSE_SENDDONE:
      (self->sixtop_vars).six2six_state = SIX_IDLE;
      break;
   case SIX_WAIT_REMOVEREQUEST_SENDDONE:
      if(error == E_SUCCESS && numOfCells > 0){
         for (i=0;i<numOfCells;i++){
            //TimeSlot 2B
            cellList[i].tsNum = (*(ptr))<<8;
            cellList[i].tsNum  |= *(ptr+1);
            //Ch.Offset 2B
            cellList[i].choffset = (*(ptr+2))<<8;
            cellList[i].choffset  |= *(ptr+3);
            //LinkOption bitmap 1B
            cellList[i].linkoptions = *(ptr+4);
            ptr += 5;
      }
 sixtop_removeCellsByState(self, msg->l2_scheduleIE_frameID,
                                numOfCells,
                                cellList,
                                &(msg->l2_nextORpreviousHop));
      }
      (self->sixtop_vars).six2six_state = SIX_IDLE;
 leds_debug_off(self);
      break;
   default:
      //log error
      break;
   }
  
   // discard reservation packets this component has created
 openqueue_freePacketBuffer(self, msg);
}

port_INLINE bool sixtop_processIEs(OpenMote* self, OpenQueueEntry_t* pkt, uint16_t * lenIE) {
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
   temp_16b = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr))<< 8);
   ptr++;
   *lenIE = ptr;
   if(
      (temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) == 
      IEEE802154E_DESC_TYPE_PAYLOAD_IE
   ){
   //payload IE - last bit is 1
      len = 
         (temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK) >> 
         IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
      gr_elem_id = 
         (temp_16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>
         IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT;
   }else {
   //header IE - last bit is 0
      len = 
         (temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK)>>
         IEEE802154E_DESC_LEN_HEADER_IE_SHIFT;
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
           temp_16b = temp_8b  +(*((uint8_t*)(pkt->payload)+ptr) << 8);
           ptr = ptr + 1;
           len = len - 2; //remove header fields len
           if(
              (temp_16b & IEEE802154E_DESC_TYPE_LONG) == 
              IEEE802154E_DESC_TYPE_LONG
              ){
              //long sub-IE - last bit is 1
              sublen =
                 (temp_16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK)>>
                 IEEE802154E_DESC_LEN_LONG_MLME_IE_SHIFT;
              subid= 
                 (temp_16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>
                 IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT; 
           } else {
              //short IE - last bit is 0
              sublen = 
                 (temp_16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK)>>
                 IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
              subid = (temp_16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>
                 IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT; 
           }
           switch(subid){
              case MLME_IE_SUBID_OPCODE:
 processIE_retrieveOpcodeIE(self, pkt,&ptr,&opcode_ie);
              break;
              case MLME_IE_SUBID_BANDWIDTH:
 processIE_retrieveBandwidthIE(self, pkt,&ptr,&bandwidth_ie);
              break;
              case MLME_IE_SUBID_TRACKID:
              break;
              case MLME_IE_SUBID_SCHEDULE:
 processIE_retrieveSheduleIE(self, pkt,&ptr,&schedule_ie);
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
 openserial_printError(self, COMPONENT_IEEE802154E,ERR_HEADER_TOO_LONG,
                           (errorparameter_t)*lenIE,
                           (errorparameter_t)1);
   }
  
   if(*lenIE>0) {
 sixtop_notifyReceiveCommand(self, &opcode_ie,
                                  &bandwidth_ie,
                                  &schedule_ie,
                                  &(pkt->l2_nextORpreviousHop));
   }
  
  return TRUE;
}

void sixtop_notifyReceiveCommand(OpenMote* self, 
   opcode_IE_ht* opcode_ie, 
   bandwidth_IE_ht* bandwidth_ie, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   
   switch(opcode_ie->opcode){
      case SIXTOP_SOFT_CELL_REQ:
         if((self->sixtop_vars).six2six_state == SIX_IDLE)
         {
            (self->sixtop_vars).six2six_state = SIX_ADDREQUEST_RECEIVED;
            //received uResCommand is reserve link request
 sixtop_notifyReceiveLinkRequest(self, bandwidth_ie,schedule_ie,addr);
         }
         break;
      case SIXTOP_SOFT_CELL_RESPONSE:
         if((self->sixtop_vars).six2six_state == SIX_WAIT_ADDRESPONSE){
           (self->sixtop_vars).six2six_state = SIX_ADDRESPONSE_RECEIVED;
           //received uResCommand is reserve link response
 sixtop_notifyReceiveLinkResponse(self, bandwidth_ie,schedule_ie,addr);
         }
         break;
      case SIXTOP_REMOVE_SOFT_CELL_REQUEST:
         if((self->sixtop_vars).six2six_state == SIX_IDLE){
            (self->sixtop_vars).six2six_state = SIX_REMOVEREQUEST_RECEIVED;
          //received uResComand is remove link request
 sixtop_notifyReceiveRemoveLinkRequest(self, schedule_ie,addr);
        }
        break;
      default:
         // log the error
         break;
      }
}

void sixtop_notifyReceiveLinkRequest(OpenMote* self, 
   bandwidth_IE_ht* bandwidth_ie, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   
   uint8_t bw,numOfcells,frameID;
   bool scheduleCellSuccess;
  
   frameID = schedule_ie->frameID;
   numOfcells = schedule_ie->numberOfcells;
   bw = bandwidth_ie->numOfLinks;
   
   // need to check whether the links are available to be scheduled.
   if(bw > numOfcells                                                 ||
      schedule_ie->frameID != bandwidth_ie->slotframeID               ||
 sixtop_areAvailableCellsToBeScheduled(self, frameID,
                                            numOfcells,
                                            schedule_ie->cellList, 
                                            bw) == FALSE){
      scheduleCellSuccess = FALSE;
   } else {
 sixtop_addCellsByState(self, 
         frameID,
         bw,
         schedule_ie->cellList,addr,(self->sixtop_vars).six2six_state);
      scheduleCellSuccess = TRUE;
   }
  
   //call link response command
 sixtop_linkResponse(self, scheduleCellSuccess,
                       addr,
                       bandwidth_ie->numOfLinks,
                       schedule_ie);
}

void sixtop_linkResponse(OpenMote* self, 
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
   sixtopPkt = openqueue_getFreePacketBuffer(self, COMPONENT_SIXTOP_RES);
  
   if(sixtopPkt==NULL) {
 openserial_printError(self, COMPONENT_SIXTOP_RES,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    
   // changing state to resLinkRespone command
   (self->sixtop_vars).six2six_state = SIX_SENDING_ADDRESPONSE;
    
   // declare ownership over that packet
   sixtopPkt->creator = COMPONENT_SIXTOP_RES;
   sixtopPkt->owner   = COMPONENT_SIXTOP_RES;
    
   memcpy(&(sixtopPkt->l2_nextORpreviousHop),tempNeighbor,sizeof(open_addr_t));
    
   // set SubFrameAndLinkIE
   len += processIE_prependSheduleIE(self, sixtopPkt,
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
   len += processIE_prependBandwidthIE(self, sixtopPkt,bw,frameID);
   //add opcodeIE
   len += processIE_prependOpcodeIE(self, sixtopPkt,SIXTOP_SOFT_CELL_RESPONSE);
   //add IE header 
 processIE_prependMLMEIE(self, sixtopPkt,len);
    
   //I has an IE in my payload
   sixtopPkt->l2_IEListPresent = IEEE154_IELIST_YES;
  
 sixtop_send(self, sixtopPkt);
  
   (self->sixtop_vars).six2six_state = SIX_WAIT_ADDRESPONSE_SENDDONE;
}

void sixtop_notifyReceiveLinkResponse(OpenMote* self, 
   bandwidth_IE_ht* bandwidth_ie, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   
   uint8_t bw,numOfcells,frameID;
  
   frameID = schedule_ie->frameID;
   numOfcells = schedule_ie->numberOfcells;
   bw = bandwidth_ie->numOfLinks;
  
   if(bw == 0){
      // link request failed
      // todo- should inform some one
      return;
   } else {
      // need to check whether the links are available to be scheduled.
      if(bw != numOfcells                                                ||
         schedule_ie->frameID != bandwidth_ie->slotframeID               ||
 sixtop_areAvailableCellsToBeScheduled(self, frameID, 
                                               numOfcells, 
                                               schedule_ie->cellList, 
                                               bw) == FALSE){
         // link request failed,inform uplayer
      } else {
 sixtop_addCellsByState(self, frameID,
                                bw,
                                schedule_ie->cellList,
                                addr,
                                (self->sixtop_vars).six2six_state);
      // link request success,inform uplayer
      }
   }
 leds_debug_off(self);
   (self->sixtop_vars).six2six_state = SIX_IDLE;
  
 opentimers_stop(self, (self->sixtop_vars).timeoutTimerId);
}

void sixtop_notifyReceiveRemoveLinkRequest(OpenMote* self, 
   schedule_IE_ht* schedule_ie,
   open_addr_t* addr){
   
   uint8_t numOfCells,frameID;
   cellInfo_ht* cellList;
  
   numOfCells = schedule_ie->numberOfcells;
   frameID = schedule_ie->frameID;
   cellList = schedule_ie->cellList;
   
 leds_debug_on(self);
  
 sixtop_removeCellsByState(self, frameID,numOfCells,cellList,addr);
  
   (self->sixtop_vars).six2six_state = SIX_IDLE;

 leds_debug_off(self);
}

//======= helper functions

bool sixtop_candidateAddCellList(OpenMote* self, 
      uint8_t*     type,
      uint8_t*     frameID,
      uint8_t*     flag,
      cellInfo_ht* cellList
   ){
   uint8_t i;
   uint8_t numCandCells;
   
   *type = 1;
   *frameID = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
   *flag = 1; // the cells listed in cellList are available to be schedule.
   
   numCandCells=0;
   for(i=0;i<MAXACTIVESLOTS;i++){
      if( schedule_isSlotOffsetAvailable(self, i)==TRUE){
         cellList[numCandCells].tsNum       = i;
         cellList[numCandCells].choffset    = 0;
         cellList[numCandCells].linkoptions = CELLTYPE_TX;
         numCandCells++;
         if(numCandCells==SCHEDULEIEMAXNUMCELLS){
            break;
         }
      }
   }
   
   if (numCandCells==0) {
      return FALSE;
   } else {
      return TRUE;
   }
}

bool sixtop_candidateRemoveCellList(OpenMote* self, 
      uint8_t*     type,
      uint8_t*     frameID,
      uint8_t*     flag,
      cellInfo_ht* cellList,
      open_addr_t* neighbor
   ){
   uint8_t              i;
   uint8_t              numCandCells;
   slotinfo_element_t   info;
   
   *type           = 1;
   *frameID        = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
   *flag           = 1;
  
   numCandCells    = 0;
   for(i=0;i<MAXACTIVESLOTS;i++){
 schedule_getSlotInfo(self, i,neighbor,&info);
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

void sixtop_addCellsByState(OpenMote* self, 
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
            case SIX_ADDREQUEST_RECEIVED:
               memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
               //add a RX link
 schedule_addActiveSlot(self, cellList[i].tsNum,
                                      CELLTYPE_RX,
                                      FALSE,
                                      cellList[i].choffset,
                                      &temp_neighbor);
               break;
            case SIX_ADDRESPONSE_RECEIVED:
               memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
               //add a TX link
 schedule_addActiveSlot(self, cellList[i].tsNum,
                                      CELLTYPE_TX,
                                      FALSE,
                                      cellList[i].choffset,
                                      &temp_neighbor);
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

void sixtop_removeCellsByState(OpenMote* self, 
      uint8_t      slotframeID,
      uint8_t      numOfLink,
      cellInfo_ht* cellList,
      open_addr_t* previousHop
   ){
   uint8_t i;
   
   for(i=0;i<numOfLink;i++){   
      if(cellList[i].linkoptions == CELLTYPE_TX){
 schedule_removeActiveSlot(self, 
            cellList[i].tsNum,
            previousHop
         );
      }
   }
}

bool sixtop_areAvailableCellsToBeScheduled(OpenMote* self, 
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
         if( schedule_isSlotOffsetAvailable(self, cellList[i].tsNum) == TRUE){
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
