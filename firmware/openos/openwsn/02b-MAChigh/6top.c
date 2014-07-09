#include "openwsn.h"
#include "6top.h"
#include "openserial.h"
#include "openqueue.h"
#include "neighbors.h"
#include "IEEE802154E.h"
#include "iphc.h"
#include "packetfunctions.h"
#include "openrandom.h"
#include "scheduler.h"
#include "opentimers.h"
#include "debugpins.h"
#include "leds.h"
#include "processIE.h"
#include "ieee802154.h"
#include "idmanager.h"
//============================ define =========================================

void reservation_timer_cb();

//=========================== variables =======================================

res_vars_t res_vars;
//#define NO_UPPER_LAYER_CALLING_RESERVATION
#define ONE_LINK 1
reservation_vars_t reservation_vars;

//=========================== prototypes ======================================

owerror_t res_send_internal(OpenQueueEntry_t* msg, uint8_t iePresent,uint8_t frameVersion);
void    sendAdv(void);
void    sendKa(void);
void    res_timer_cb(void);
uint8_t res_copySlotFrameAndLinkIE(OpenQueueEntry_t* adv);//returns reserved size

//=========================== public ==========================================

void res_init() {
   
   res_vars.periodMaintenance = 872+(openrandom_get16b()&0xff); // fires every 1 sec on average
   res_vars.busySendingKa     = FALSE;
   res_vars.busySendingAdv    = FALSE;
   res_vars.dsn               = 0;
   res_vars.MacMgtTaskCounter = 0;
   
   res_vars.timerId = opentimers_start(
      res_vars.periodMaintenance,
      TIMER_PERIODIC,
      TIME_MS,
      res_timer_cb
   );
   
   memset(&reservation_vars,0,sizeof(reservation_vars_t));
   reservation_vars.periodReservation = RESPERIOD;
   reservation_vars.timerId = opentimers_start(
      reservation_vars.periodReservation,
      TIMER_PERIODIC,
      TIME_MS,
      reservation_timer_cb
   );
   
   res_vars.kaPeriod          = KATIMEOUT;
}

//admin
void    reservation_init(){
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_myDAGrank() {
   uint16_t output=0;
   output = neighbors_getMyDAGrank();
   openserial_printStatus(STATUS_DAGRANK,(uint8_t*)&output,sizeof(uint16_t));
   return TRUE;
}

//======= from upper layer

owerror_t res_send(OpenQueueEntry_t *msg) {
   msg->owner        = COMPONENT_RES;
   msg->l2_frameType = IEEE154_TYPE_DATA;
   if(msg->l2_IEListPresent == IEEE154_IELIST_NO) {
    return res_send_internal(msg,IEEE154_IELIST_NO,IEEE154_FRAMEVERSION_2006);
   } else {
    return res_send_internal(msg,IEEE154_IELIST_YES,IEEE154_FRAMEVERSION);
   }
}

void res_setKaPeriod(uint16_t kaPeriod) {
   if(kaPeriod > KATIMEOUT) {
     res_vars.kaPeriod = KATIMEOUT;
   } else {
     res_vars.kaPeriod = kaPeriod;
   } 
}

//======= from lower layer

void task_resNotifSendDone() {
   OpenQueueEntry_t* msg;
   // get recently-sent packet from openqueue
   msg = openqueue_resGetSentPacket();
   if (msg==NULL) {
      // log the error
      openserial_printCritical(COMPONENT_RES,ERR_NO_SENT_PACKET,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      return;
   }
   // declare it as mine
   msg->owner = COMPONENT_RES;
   // indicate transmission (to update statistics)
   if (msg->l2_sendDoneError==E_SUCCESS) {
      neighbors_indicateTx(&(msg->l2_nextORpreviousHop),
                           msg->l2_numTxAttempts,
                           TRUE,
                           &msg->l2_asn);
   } else {
      neighbors_indicateTx(&(msg->l2_nextORpreviousHop),
                           msg->l2_numTxAttempts,
                           FALSE,
                           &msg->l2_asn);
   }
   // send the packet to where it belongs
   if (msg->creator == COMPONENT_RES) {
      if (msg->l2_frameType==IEEE154_TYPE_BEACON) {
         // this is a ADV
         
         // not busy sending ADV anymore
         res_vars.busySendingAdv = FALSE;
      } else {
         // this is a KA
         
         // not busy sending KA anymore
         res_vars.busySendingKa = FALSE;
      }
      // discard packets
      openqueue_freePacketBuffer(msg);
      // restart a random timer
      res_vars.periodMaintenance = 872+(openrandom_get16b()&0xff);
      opentimers_setPeriod(res_vars.timerId,
                           TIME_MS,
                           res_vars.periodMaintenance);
   } else {
     if(msg->creator == COMPONENT_RESERVATION) {
       reservation_sendDone(msg,msg->l2_sendDoneError);
     }
     else
      // send the rest up the stack
      iphc_sendDone(msg,msg->l2_sendDoneError);
   }
}

void task_resNotifReceive() {
   OpenQueueEntry_t* msg;
   
   // get received packet from openqueue
   msg = openqueue_resGetReceivedPacket();
   if (msg==NULL) {
      // log the error
      openserial_printCritical(COMPONENT_RES,ERR_NO_RECEIVED_PACKET,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      return;
   }
   
   // declare it as mine
   msg->owner = COMPONENT_RES;
   
   // indicate reception (to update statistics)
   neighbors_indicateRx(&(msg->l2_nextORpreviousHop),
                        msg->l1_rssi,
                        &msg->l2_asn,
                        msg->l2_joinPriorityPresent,
                        msg->l2_joinPriority);
   
   msg->l2_joinPriorityPresent=FALSE; //reset it to avoid race conditions with this var.
   
   // send the packet up the stack, if it qualifies
   switch (msg->l2_frameType) {
      case IEEE154_TYPE_BEACON:
      case IEEE154_TYPE_DATA:
      case IEEE154_TYPE_CMD:
         if (msg->length>0) {
           if(msg->l2_frameType==IEEE154_TYPE_DATA && msg->l2_IEListPresent == IEEE154_IELIST_YES){
            IEFiled_retrieveIE(msg); 
            openqueue_freePacketBuffer(msg);
           } else {
            // send to upper layer
            iphc_receive(msg);
           }
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
         openserial_printError(COMPONENT_RES,ERR_MSG_UNKNOWN_TYPE,
                               (errorparameter_t)msg->l2_frameType,
                               (errorparameter_t)0);
         break;
   }
}

//======= timer

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timers_res_fired(void) {
   res_vars.MacMgtTaskCounter = (res_vars.MacMgtTaskCounter+1)%ADVTIMEOUT;
   
   switch (res_vars.MacMgtTaskCounter) {
      case 0:
         // called every ADVTIMEOUT seconds
         sendAdv();
         break;
      case 1:
         // called every ADVTIMEOUT seconds
         neighbors_removeOld();
         break;
      default:
         // called every second, except twice every ADVTIMEOUT seconds
         sendKa();
         break;
   }
}

//=========================== private =========================================

/**
\brief Transfer packet to MAC.

This function adds a IEEE802.15.4 header to the packet and leaves it the 
OpenQueue buffer. The very last thing it does is assigning this packet to the 
virtual component COMPONENT_RES_TO_IEEE802154E. Whenever it gets a change,
IEEE802154E will handle the packet.

\param[in] msg The packet to the transmitted
\param[in] iePresent Indicates wheter an Information Element is present in the
   packet.
\param[in] frameVersion The frame version to write in the packet.

\returns E_SUCCESS iff successful.
*/
owerror_t res_send_internal(OpenQueueEntry_t* msg, uint8_t iePresent, uint8_t frameVersion) {
   // assign a number of retries
   if (packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))==TRUE) {
      msg->l2_retriesLeft = 1;
   } else {
      msg->l2_retriesLeft = TXRETRIES;
   }
   // record this packet's dsn (for matching the ACK)
   msg->l2_dsn = res_vars.dsn++;
   // this is a new packet which I never attempted to send
   msg->l2_numTxAttempts = 0;
   // transmit with the default TX power
   msg->l1_txPower = TX_POWER;
   // record the location, in the packet, where the l2 payload starts
   msg->l2_payload = msg->payload;
   // add a IEEE802.15.4 header
   ieee802154_prependHeader(msg,
                            msg->l2_frameType,
                            iePresent,
                            frameVersion,
                            IEEE154_SEC_NO_SECURITY,
                            msg->l2_dsn,
                            &(msg->l2_nextORpreviousHop)
                            );
   // reserve space for 2-byte CRC
   packetfunctions_reserveFooterSize(msg,2);
   // change owner to IEEE802154E fetches it from queue
   msg->owner  = COMPONENT_RES_TO_IEEE802154E;
   return E_SUCCESS;
}

/**
\brief Send an advertisement.

This is one of the MAC managament tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sendAdv() {
   OpenQueueEntry_t* adv;
   payload_IE_descriptor_t payload_IE_desc;
   MLME_IE_subHeader_t mlme_subHeader;
   uint8_t slotframeIElen=0;
   
   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (ADV and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_RES);
      
      // I'm now busy sending an ADV
      res_vars.busySendingAdv = FALSE;
      
      // stop here
      return;
   }
   
   if (res_vars.busySendingAdv==TRUE) {
      // don't continue if I'm still sending a previous ADV
   }
   
   // if I get here, I will send an ADV
   
   // get a free packet buffer
   adv = openqueue_getFreePacketBuffer(COMPONENT_RES);
   if (adv==NULL) {
      openserial_printError(COMPONENT_RES,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   adv->creator = COMPONENT_RES;
   adv->owner   = COMPONENT_RES;
   
   // reserve space for ADV-specific header
   // xv poipoi -- reserving for IEs  -- reverse order.
   //TODO reserve here for slotframe and link IE with minimal schedule information
   slotframeIElen = res_copySlotFrameAndLinkIE(adv);
   //create Sync IE with JP and ASN 
   packetfunctions_reserveHeaderSize(adv, sizeof(synch_IE_t));//the asn + jp
   adv->l2_ASNpayload               = adv->payload; //keep a pointer to where the ASN should be.
   // the actual value of the current ASN and JP will be written by the
   // IEEE802.15.4e when transmitting
   packetfunctions_reserveHeaderSize(adv, sizeof(MLME_IE_subHeader_t));//the MLME header
   //copy mlme sub-header               
   mlme_subHeader.length_subID_type=sizeof(synch_IE_t) << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
   mlme_subHeader.length_subID_type |= (IEEE802154E_MLME_SYNC_IE_SUBID << IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT) | IEEE802154E_DESC_TYPE_SHORT;
   //little endian          
   adv->payload[0]= mlme_subHeader.length_subID_type & 0xFF;
   adv->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
    
   packetfunctions_reserveHeaderSize(adv, sizeof(payload_IE_descriptor_t));//the payload IE header
   //prepare IE headers and copy them to the ADV 
   
   payload_IE_desc.length_groupid_type = (sizeof(MLME_IE_subHeader_t)+sizeof(synch_IE_t)+slotframeIElen)<<IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
   payload_IE_desc.length_groupid_type |=  (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_DESC_TYPE_LONG); //
   
   //copy header into the packet
   //little endian
   adv->payload[0]= payload_IE_desc.length_groupid_type & 0xFF;
   adv->payload[1]= (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
  
   // some l2 information about this packet
   adv->l2_frameType                     = IEEE154_TYPE_BEACON;
   adv->l2_nextORpreviousHop.type        = ADDR_16B;
   adv->l2_nextORpreviousHop.addr_16b[0] = 0xff;
   adv->l2_nextORpreviousHop.addr_16b[1] = 0xff;
   
   // put in queue for MAC to handle
   res_send_internal(adv,IEEE154_IELIST_YES,IEEE154_FRAMEVERSION);
   
   // I'm now busy sending an ADV
   res_vars.busySendingAdv = TRUE;
}

port_INLINE uint8_t res_copySlotFrameAndLinkIE(OpenQueueEntry_t* adv){
  MLME_IE_subHeader_t mlme_subHeader;
  uint8_t len=0;
  uint8_t linkOption=0;
  uint16_t slot=SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS+SCHEDULE_MINIMAL_6TISCH_EB_CELLS;
  
  //reverse order and little endian. -- 
 
  //for each link in the schedule (in basic configuration)
  //copy to adv 1B linkOption bitmap
  //copy to adv 2B ch.offset
  //copy to adv 2B timeslot
 
  //shared cells
  linkOption = (1<<FLAG_TX_S)|(1<<FLAG_RX_S)|(1<<FLAG_SHARED_S);
  while(slot>SCHEDULE_MINIMAL_6TISCH_EB_CELLS){
    packetfunctions_reserveHeaderSize(adv,5);
    //ts
    adv->payload[0]= slot & 0xFF;
    adv->payload[1]= (slot >> 8) & 0xFF;
    //ch.offset as minimal draft
    adv->payload[2]= 0x00;
    adv->payload[3]= 0x00;
    //linkOption
    adv->payload[4]= linkOption;
    len+=5;
    slot--;
  }
 
  //eb slot
  linkOption = (1<<FLAG_TX_S)|(1<<FLAG_RX_S)|(1<<FLAG_SHARED_S)|(1<<FLAG_TIMEKEEPING_S);
  packetfunctions_reserveHeaderSize(adv,5);
  len+=5;
 //ts
  adv->payload[0]= SCHEDULE_MINIMAL_6TISCH_EB_CELLS & 0xFF;
  adv->payload[1]= (SCHEDULE_MINIMAL_6TISCH_EB_CELLS >> 8) & 0xFF;
  //ch.offset as minimal draft
  adv->payload[2]= 0x00;
  adv->payload[3]= 0x00;
 
  adv->payload[4]= linkOption;
 //now slotframe ie general fields
    //1B number of links == 6 
    //Slotframe Size 2B = 101 timeslots
    //1B slotframe handle (id)
  packetfunctions_reserveHeaderSize(adv,5);//
  len+=5;
  
  adv->payload[0]= SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER;  
  adv->payload[1]= SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
  adv->payload[2]= SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE & 0xFF;
  adv->payload[3]= (SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE >> 8) & 0xFF;
  adv->payload[4]= 0x06; //number of links
  
  //MLME sub IE header 
  //1b -15 short ==0x00
  //7b -8-14 Sub-ID=0x1b
  //8b - Length = 2 mlme-header + 5 slotframe general header +(6links*5bytes each) 
  packetfunctions_reserveHeaderSize(adv, sizeof(MLME_IE_subHeader_t));//the MLME header
   
   
   //copy mlme sub-header               
  mlme_subHeader.length_subID_type = len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
  mlme_subHeader.length_subID_type |= (IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID << IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT) | IEEE802154E_DESC_TYPE_SHORT;
  
  //little endian          
  adv->payload[0]= mlme_subHeader.length_subID_type & 0xFF;
  adv->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
  len+=2;//count len of mlme header
   
  return len;
}

/**
\brief Send an keep-alive message, if nessary.

This is one of the MAC managament tasks. This function inlines in the
timers_res_fired() function, but is declared as a separate function for better
readability of the code.
*/
port_INLINE void sendKa() {
   OpenQueueEntry_t* kaPkt;
   open_addr_t*      kaNeighAddr;
   
/*
#ifdef OPENSIM
   debugpins_debug_set();
   debugpins_debug_clr();
#endif
*/
   
   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (ADV and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_RES);
      
      // I'm now busy sending a KA
      res_vars.busySendingKa = FALSE;
      
      // stop here
      return;
   }
   
   if (res_vars.busySendingKa==TRUE) {
      // don't proceed if I'm still sending a KA
      return;
   }
   
   kaNeighAddr = neighbors_getKANeighbor(res_vars.kaPeriod);
   if (kaNeighAddr==NULL) {
      // don't proceed if I have no neighbor I need to send a KA to
      return;
   }
   
   // if I get here, I will send a KA
   
   // get a free packet buffer
   kaPkt = openqueue_getFreePacketBuffer(COMPONENT_RES);
   if (kaPkt==NULL) {
      openserial_printError(COMPONENT_RES,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)1,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   kaPkt->creator = COMPONENT_RES;
   kaPkt->owner   = COMPONENT_RES;
   
   // some l2 information about this packet
   kaPkt->l2_frameType = IEEE154_TYPE_DATA;
   memcpy(&(kaPkt->l2_nextORpreviousHop),kaNeighAddr,sizeof(open_addr_t));
   
   // put in queue for MAC to handle
   res_send_internal(kaPkt,IEEE154_IELIST_NO,IEEE154_FRAMEVERSION_2006);
   
   // I'm now busy sending a KA
   res_vars.busySendingKa = TRUE;

#ifdef OPENSIM
   debugpins_ka_set();
   debugpins_ka_clr();
#endif
}

void res_timer_cb() {
   scheduler_push_task(timers_res_fired,TASKPRIO_RES);
}

void    res_notifRetrieveIEDone(OpenQueueEntry_t *msg){
  // sync IE is toss in ieee802154e
  reservation_notifyReceiveuResCommand(msg);
}

// reservation
void    reservation_notifyReceiveuResLinkRequest(OpenQueueEntry_t* msg){
  uint8_t numOfLinksBw,slotframeIDBw; //in BW IE
  uint8_t slotframeID,numOfLink; //in Frame and Link IE
  
  //qw : indicate receiving ResLinkRequest
//  leds_debug_toggle();
  
  uResBandwidthIEcontent_t* tempBandwidthIE = processIE_getuResBandwidthIEcontent();
  //record bandwidth information
  numOfLinksBw  = tempBandwidthIE->numOfLinks;
  slotframeIDBw = tempBandwidthIE->slotframeID;
  
  frameAndLinkIEcontent_t* tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();
    
  for(uint8_t i = 0; i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
  {
    slotframeID    = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
    numOfLink      = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;
    if(slotframeIDBw == slotframeID){
      //allocate links for neighbor
      schedule_allocateLinks(slotframeID,numOfLink,numOfLinksBw);
    }
  }
  
  schedule_addLinksToSchedule(slotframeIDBw,&(msg->l2_nextORpreviousHop),numOfLinksBw,reservation_vars.State);
  
  //reservation_vars.State = S_IDLE;
  
  //call link response command
  reservation_linkResponse(&(msg->l2_nextORpreviousHop));
  
}

void    reservation_notifyReceiveuResLinkResponse(OpenQueueEntry_t* msg){

    frameAndLinkIEcontent_t* tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();
    
    for(uint8_t i = 0; i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
    {
      uint8_t slotframeID    = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
      uint8_t numOfLink      = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;
      
      schedule_addLinksToSchedule(slotframeID,&(msg->l2_nextORpreviousHop),numOfLink,reservation_vars.State);

    }
    
    reservation_vars.State = S_IDLE;

    leds_debug_off();
}

void    reservation_notifyReceiveRemoveLinkRequest(OpenQueueEntry_t* msg){
  
  leds_debug_on();
    
  frameAndLinkIEcontent_t* tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();
    
  for(uint8_t i = 0; i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
  {
    uint8_t slotframeID    = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
    uint16_t slotframeSize = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeSize;
    uint8_t numOfLink      = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;

    schedule_removeLinksFromSchedule(slotframeID,slotframeSize,numOfLink,&(msg->l2_nextORpreviousHop),reservation_vars.State);
  }
  
  reservation_vars.State = S_IDLE;
  leds_debug_off();
}

void    reservation_notifyReceiveScheduleRequest(OpenQueueEntry_t* msg){
}

void    reservation_notifyReceiveScheduleResponse(OpenQueueEntry_t* msg){
}


//call res layer
void    reservation_sendDone(OpenQueueEntry_t* msg, owerror_t error){
      msg->owner = COMPONENT_RESERVATION;

      switch (reservation_vars.State)
      {
      case S_WAIT_RESLINKREQUEST_SENDDONE:
        reservation_vars.State = S_WAIT_FORRESPONSE;
        break;
      case S_WAIT_RESLINKRESPONSE_SENDDONE:
        reservation_vars.State = S_IDLE;
        break;
      case S_WAIT_REMOVELINKREQUEST_SENDDONE:
        reservation_vars.State = S_IDLE;
        leds_debug_off();
        break;
      default:
        //log error
        break;
      }
      // discard reservation packets this component has created
      openqueue_freePacketBuffer(msg);
}


void    reservation_notifyReceiveuResCommand(OpenQueueEntry_t* msg){
      //reset sub IE
      resetSubIE();
  
      uResCommandIEcontent_t* tempuResCommandIEcontent = processIE_getuResCommandIEcontent();
      switch(tempuResCommandIEcontent->uResCommandID)
      {
      case 0:
        if(reservation_vars.State == S_IDLE)
        {
          reservation_vars.State = S_RESLINKREQUEST_RECEIVE;
          //received uResCommand is reserve link request
          reservation_notifyReceiveuResLinkRequest(msg);
        }
        break;
      case 1:
        if(reservation_vars.State == S_WAIT_FORRESPONSE)
        {
          reservation_vars.State = S_RESLINKRESPONSE_RECEIVE;
          //received uResCommand is reserve link response
          reservation_notifyReceiveuResLinkResponse(msg);
        }
        break;
      case 2:
        if(reservation_vars.State == S_IDLE)
        {
          reservation_vars.State = S_REMOVELINKREQUEST_RECEIVE;
          //received uResComand is remove link request
          reservation_notifyReceiveRemoveLinkRequest(msg);
        }
        break;
      case 3:
        //received uResCommand is schedule request
        reservation_notifyReceiveScheduleRequest(msg);
        break;
      case 4:
        //received uResCommand is schedule response
        reservation_notifyReceiveScheduleResponse(msg);
        break;
      default:
         // log the error
        break;
      }
}

//call by up layer to reserve a link with a neighbour
void reservation_linkRequest(open_addr_t*  reservationNeighAddr, uint16_t bandwidth) {
  
  OpenQueueEntry_t* reservationPkt;
  
  if(reservation_vars.State != S_IDLE)
    return;
  
//  leds_debug_toggle();
  
  if(reservationNeighAddr==NULL){
     return;
  }
    // get a free packet buffer
    reservationPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (reservationPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    
    //change state to resLinkRequest command
    reservation_vars.State = S_RESLINKREQUEST_SEND;
    
    // declare ownership over that packet
    reservationPkt->creator = COMPONENT_RESERVATION;
    reservationPkt->owner   = COMPONENT_RESERVATION;
         
    memcpy(&(reservationPkt->l2_nextORpreviousHop),reservationNeighAddr,sizeof(open_addr_t));
    
    uint8_t numOfSlotframes = schedule_getNumSlotframe();
    
#ifdef NO_UPPER_LAYER_CALLING_RESERVATION
    //generate candidata links
    for(uint8_t i=0;i<numOfSlotframes;i++)
      schedule_uResGenerateCandidataLinkList(i);
#endif
    
    //set SubFrameAndLinkIE
    processIE_setSubFrameAndLinkIE();
    //set uResCommandIE
    processIE_setSubuResCommandIE(RESERCATIONLINKREQ);

    //set uResBandwidthIE
    processIE_setSubuResBandwidthIE(bandwidth,0);
    
    //set IE after set all required subIE
    processIE_setMLME_IE();
    
    //add an IE to adv's payload
    IEFiled_prependIE(reservationPkt);
    
    //I has an IE in my payload
    reservationPkt->l2_IEListPresent = IEEE154_IELIST_YES;
    //debug
//    packetfunctions_reserveHeaderSize(reservationPkt,10);
    
    
//    for(uint8_t i=0;i<10;i++) {
//       reservationPkt->payload[i]=0x09;
//    }   
    
    res_send(reservationPkt);
    
    reservation_vars.State = S_WAIT_RESLINKREQUEST_SENDDONE;
}

void  reservation_linkResponse(open_addr_t* tempNeighbor){
  
  OpenQueueEntry_t* reservationPkt;
  
    // get a free packet buffer
    reservationPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (reservationPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    
    //changing state to resLinkRespone command
    reservation_vars.State = S_RESLINKRESPONSE_SEND;
    
    // declare ownership over that packet
    reservationPkt->creator = COMPONENT_RESERVATION;
    reservationPkt->owner   = COMPONENT_RESERVATION;
    
    memcpy(&(reservationPkt->l2_nextORpreviousHop),tempNeighbor,sizeof(open_addr_t));

    //set SubFrameAndLinkIE
    processIE_setSubFrameAndLinkIE();
    
    //set uResCommandIE  //set uRes command ID
    processIE_setSubuResCommandIE(RESERCATIONLINKRESPONSE);

    //set IE after set all required subIE
    processIE_setMLME_IE();
    
    IEFiled_prependIE(reservationPkt);
    
    //I has an IE in my payload
    reservationPkt->l2_IEListPresent = IEEE154_IELIST_YES;
  
    res_send(reservationPkt);
  
    reservation_vars.State = S_WAIT_RESLINKRESPONSE_SENDDONE;
}

//remove one link command
void reservation_removeLinkRequest(open_addr_t*  reservationNeighAddr){
  
  OpenQueueEntry_t* reservationPkt;
  
  if(reservation_vars.State != S_IDLE)
    return;
  
//  leds_debug_toggle();
  
  if(reservationNeighAddr!=NULL){
    // get a free packet buffer
    reservationPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (reservationPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    // change state to sending removeLinkRequest Command
    reservation_vars.State = S_REMOVELINKREQUEST_SEND;
    // declare ownership over that packet
    reservationPkt->creator = COMPONENT_RESERVATION;
    reservationPkt->owner   = COMPONENT_RESERVATION;
         
    memcpy(&(reservationPkt->l2_nextORpreviousHop),reservationNeighAddr,sizeof(open_addr_t));
    
    uint8_t numOfSlotframes = schedule_getNumSlotframe();
#ifdef NO_UPPER_LAYER_CALLING_RESERVATION
    // this is the cell that will removed by neighbor
    Link_t tempLink;
    tempLink.channelOffset      = 0;
    tempLink.slotOffset         = 8; // By experiment, I knew slotoffset 8 is created, so I can deleted it here.  7, make sure?
    tempLink.link_type           = CELLTYPE_RX;// this is for cell removed by neighbor
    //generate links to be removed
    for(uint8_t i=0;i<numOfSlotframes;i++)
      schedule_uResGenerateRemoveLinkList(i,tempLink);
#endif
    //set SubFrameAndLinkIE
    processIE_setSubFrameAndLinkIE();
    
    //remove links in local
    frameAndLinkIEcontent_t* tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();

    for(uint8_t i = 0; i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
    {
      uint8_t slotframeID    = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
      uint16_t slotframeSize = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeSize;
      uint8_t numOfLink      = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;

      schedule_removeLinksFromSchedule(slotframeID,slotframeSize,numOfLink,&(reservationPkt->l2_nextORpreviousHop),reservation_vars.State);
    }
    
    //set uResCommandIE
    processIE_setSubuResCommandIE(RESERVATIONREMOVELINKREQUEST);

    //set IE after set all required subIE
    processIE_setMLME_IE();
    //add an IE to adv's payload
    IEFiled_prependIE(reservationPkt);
    
    //I has an IE in my payload
    reservationPkt->l2_IEListPresent = IEEE154_IELIST_YES;
  
    res_send(reservationPkt);
    
    reservation_vars.State = S_WAIT_REMOVELINKREQUEST_SENDDONE;
  }
}

//void reservation_pretendSendData(){
//  //pretend sending an data from upper layer
//  OpenQueueEntry_t* reservationPkt;
//  open_addr_t*      reservationNeighAddr;
//  
//  reservationNeighAddr = neighbors_reservationNeighbor();
//  if(reservationNeighAddr!=NULL){
//    // get a free packet buffer
//    reservationPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
//  
//    if (reservationPkt==NULL) {
//      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
//      return;
//    }
//    
//    // pretend that I am COMPONENT_UPPLAYER
//    reservationPkt->creator = COMPONENT_UPPLAYER;
//    reservationPkt->owner   = COMPONENT_RESERVATION;
//         
//    memcpy(&(reservationPkt->l2_nextORpreviousHop),reservationNeighAddr,sizeof(open_addr_t));
//    
//    uint8_t  Halloween[13];
//    Halloween[0]  = 0xff;
//    Halloween[1]  = (uint8_t)'T';
//    Halloween[2]  = (uint8_t)'r';
//    Halloween[3]  = (uint8_t)'i';
//    Halloween[4]  = (uint8_t)'c';
//    Halloween[5]  = (uint8_t)'k';
//    Halloween[6]  = (uint8_t)'O';
//    Halloween[7]  = (uint8_t)'r';
//    Halloween[8]  = (uint8_t)'T';
//    Halloween[9]  = (uint8_t)'r';
//    Halloween[10]  = (uint8_t)'e';
//    Halloween[11] = (uint8_t)'a';
//    Halloween[12] = (uint8_t)'t';
//    packetfunctions_reserveHeaderSize(reservationPkt,sizeof(Halloween));
//    
//    memcpy(reservationPkt->payload,Halloween,sizeof(Halloween));
//    
//    res_send(reservationPkt);
//  }
//}

void reservation_pretendReceiveData(OpenQueueEntry_t* msg){
    uint8_t  Halloween[6];
    Halloween[0]  = msg->payload[0];
    Halloween[1]  = msg->payload[1];
    Halloween[2]  = msg->payload[2];
    Halloween[3]  = msg->payload[3];
    Halloween[4]  = msg->payload[4];
    Halloween[5]  = msg->payload[5];
    
    openserial_printData(Halloween, sizeof(Halloween));
    
    openqueue_freePacketBuffer(msg);
} 

//===================== task ==================
/**
\brief this function is going to schedule or remove one link.
*/
void timers_reservation_fired() {
  open_addr_t*  reservationNeighAddr;
  if(ieee154e_isSynch()==FALSE) {
    return;
  }
  
  if(idmanager_getIsDAGroot() == TRUE){
    return;
  }
  
  if(reservation_vars.addORremove == TRUE) {
    leds_debug_on();
    reservation_vars.addORremove = FALSE;
    // I'm going to require to remove one link
    reservationNeighAddr = neighbors_reservationNeighbor();
    reservation_removeLinkRequest(reservationNeighAddr);
  } else {
    leds_debug_on();
    reservation_vars.addORremove = TRUE;
    // I'm going to require to add one link
    reservationNeighAddr = neighbors_reservationNeighbor();
    reservation_linkRequest(reservationNeighAddr,1);
  }
  
}

//========================== private =========================================

void reservation_timer_cb() {
  scheduler_push_task(timers_reservation_fired,TASKPRIO_RESERVATION);
}

//event
void isr_reservation_button() {
  open_addr_t*  reservationNeighAddr;
  
  switch (reservation_vars.button_event){
  case 0:
  case 1:
    //set slotframeID and bandwidth
  
    reservationNeighAddr = neighbors_reservationNeighbor();
    reservation_linkRequest(reservationNeighAddr,2);
    break;
  case 2:
    reservationNeighAddr = neighbors_reservationNeighbor();
  
    reservation_removeLinkRequest(reservationNeighAddr);
    break;
  default:
    //pretend that uppler is sending a data
    reservation_pretendSendData();
  }
  
  reservation_vars.button_event += 1;
  //reservation_vars.button_event = (reservation_vars.button_event+1)%3;
}
