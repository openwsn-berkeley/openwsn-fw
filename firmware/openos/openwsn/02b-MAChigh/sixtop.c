#include "openwsn.h"
#include "sixtop.h"
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
#include "IEEE802154.h"
#include "idmanager.h"
#include "schedule.h"

//=========================== variables =======================================

sixtop_vars_t sixtop_vars;

//=========================== prototypes ======================================

// sixtop command adv/keep alive
void          sendAdv(void);
void          sendKa(void);

// timer interrupt callback
void          sixtop_timer_cb(void);
void          sixtop_timeout_timer_cb(void);

// process the packet if it was related to sixtop
void          sixtop_sendDone(OpenQueueEntry_t* msg, owerror_t error);
bool          sixtop_processIEs(OpenQueueEntry_t* pkt, uint16_t * lenIE);

// process the packet if it was a packet related to link reservation
void          sixtop_notifyReceiveCommand(
   sixtop_opcode_subIE_t*              opcode_ie, 
   sixtop_bandwidth_subIE_t*           bandwidth_ie, 
   sixtop_generalschedule_subIE_t*     schedule_ie,
   open_addr_t*                        addr
);
void          sixtop_notifyReceiveLinkRequest(
   sixtop_bandwidth_subIE_t*           bandwidth_ie,
   sixtop_generalschedule_subIE_t*     schedule_ie,
   open_addr_t*                        addr
);
void          sixtop_notifyReceiveLinkResponse(
   sixtop_bandwidth_subIE_t*           bandwidth_ie,
   sixtop_generalschedule_subIE_t*     schedule_ie,
   open_addr_t*                        addr
);
void          sixtop_notifyReceiveRemoveLinkRequest(
   sixtop_generalschedule_subIE_t*     schedule_ie,
   open_addr_t*                        addr
);

// send internal
owerror_t     sixtop_send_internal(OpenQueueEntry_t* msg, uint8_t iePresent,uint8_t frameVersion);

// help functions
bool          sixtop_availableCells(
   uint8_t frameID, 
   uint8_t numOfCells, 
   sixtop_linkInfo_subIE_t* linklist, 
   uint8_t bandwidth
);
bool          sixtop_uResGenerateCandidataLinkList(
   uint8_t* type,
   uint8_t* frameID,
   uint8_t* flag,
   sixtop_linkInfo_subIE_t* linklist
);
bool          sixtop_uResGenerateRemoveLinkList(
   uint8_t* type,
   uint8_t* frameID,
   uint8_t* flag,
   sixtop_linkInfo_subIE_t* linklist,
   open_addr_t* neighbor
);
void          sixtop_addLinksToSchedule(
   uint8_t slotframeID,
   uint8_t numOfLinks,
   sixtop_linkInfo_subIE_t* linklist,
   open_addr_t* previousHop,
   uint8_t state
);
void          sixtop_removeLinksFromSchedule(
   uint8_t slotframeID,
   uint8_t numOfLink,
   sixtop_linkInfo_subIE_t* linklist,
   open_addr_t* previousHop
);

//=========================== public ==========================================

void sixtop_init() {
   
   sixtop_vars.periodMaintenance = 872+(openrandom_get16b()&0xff); // fires every 1 sec on average
   sixtop_vars.busySendingKa     = FALSE;
   sixtop_vars.busySendingAdv    = FALSE;
   sixtop_vars.dsn               = 0;
   sixtop_vars.MacMgtTaskCounter = 0;
   sixtop_vars.kaPeriod          = KATIMEOUT;
   
   sixtop_vars.timerId = opentimers_start(
      sixtop_vars.periodMaintenance,
      TIMER_PERIODIC,
      TIME_MS,
      sixtop_timer_cb
   );
   
   sixtop_vars.TOtimerId = opentimers_start(
      SIXTOP2SIXTOP_TIMEOUT_MS,
      TIMER_ONESHOT,
      TIME_MS,
      sixtop_timeout_timer_cb                                  
   );
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

//======= configuration

void sixtop_setKaPeriod(uint16_t kaPeriod) {
   if(kaPeriod > KATIMEOUT) {
      sixtop_vars.kaPeriod = KATIMEOUT;
   } else {
      sixtop_vars.kaPeriod = kaPeriod;
   } 
}

//======= scheduling

void sixtop_linkRequest(open_addr_t*  sixtopNeighAddr, uint16_t bandwidth) {
  OpenQueueEntry_t* sixtopPkt;
  uint8_t len=0;
  uint8_t type,frameID,flag;
  bool success;
  sixtop_linkInfo_subIE_t linklist[MAXSCHEDULEDCELLS];
  payload_IE_descriptor_t payload_IE_desc;
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  memset(linklist,0,MAXSCHEDULEDCELLS*sizeof(sixtop_linkInfo_subIE_t));
  
  if(sixtop_vars.State != S_IDLE){
    return;
  }
  
  if(sixtopNeighAddr==NULL){
     return;
  }
  
   //generate candidata links
   success = sixtop_uResGenerateCandidataLinkList(&type,&frameID,&flag,linklist);
   
   if(success == FALSE) {
     //there is no cell available to be schedule
     return;
   }
    // get a free packet buffer
    sixtopPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (sixtopPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   //change state to resLinkRequest command
   sixtop_vars.State  = S_SIXTOP_LINKREQUEST_SEND;
   
   // declare ownership over that packet
   sixtopPkt->creator = COMPONENT_RESERVATION;
   sixtopPkt->owner   = COMPONENT_RESERVATION;
   
   memcpy(&(sixtopPkt->l2_nextORpreviousHop),sixtopNeighAddr,sizeof(open_addr_t));
   
   //set SubFrameAndLinkIE
   len += processIE_prependSixtopGeneralSheduleIE(sixtopPkt,type,frameID,flag,linklist);
   //set uResBandwidthIE
   len += processIE_prependSixtopBandwidthIE(sixtopPkt,bandwidth,SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE);
   //set uResopcodeIE
   len += processIE_prependSixtopOpcodeIE(sixtopPkt,SIXTOP_SOFT_CELL_REQ);
   
   packetfunctions_reserveHeaderSize(sixtopPkt, sizeof(payload_IE_descriptor_t));//the payload IE header
   //prepare IE headers and copy them to the sixtopPkt
   
   payload_IE_desc.length_groupid_type  = len<<IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
   payload_IE_desc.length_groupid_type |= (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_DESC_TYPE_LONG); //
   
   //copy header into the packet
   //little endian
   sixtopPkt->payload[0]= payload_IE_desc.length_groupid_type & 0xFF;
   sixtopPkt->payload[1]= (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
   
   //I has an IE in my payload
   sixtopPkt->l2_IEListPresent = IEEE154_IELIST_YES;
   
   sixtop_send(sixtopPkt);
   
   sixtop_vars.State = S_WAIT_SIXTOP_LINKREQUEST_SENDDONE;
   
   //start the timeout timer
   opentimers_restart(sixtop_vars.TOtimerId);
   opentimers_setPeriod(sixtop_vars.TOtimerId,TIME_MS,SIXTOP2SIXTOP_TIMEOUT_MS);
   ENABLE_INTERRUPTS();
}

void sixtop_removeLinkRequest(open_addr_t*  sixtopNeighAddr){
  OpenQueueEntry_t* sixtopPkt;
  uint8_t len=0;
  bool success;
  uint8_t type,frameID,flag;
  sixtop_linkInfo_subIE_t linklist[MAXSCHEDULEDCELLS];
  payload_IE_descriptor_t payload_IE_desc;
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  memset(linklist,0,MAXSCHEDULEDCELLS*sizeof(sixtop_linkInfo_subIE_t));
  
  if(sixtop_vars.State != S_IDLE)
    return;
  
  if(sixtopNeighAddr!=NULL){
    // get a free packet buffer
    sixtopPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (sixtopPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    // change state to sending removeLinkRequest Command
    sixtop_vars.State = S_REMOVELINKREQUEST_SEND;
    // declare ownership over that packet
    sixtopPkt->creator = COMPONENT_RESERVATION;
    sixtopPkt->owner   = COMPONENT_RESERVATION;
         
    memcpy(&(sixtopPkt->l2_nextORpreviousHop),sixtopNeighAddr,sizeof(open_addr_t));
    
    success = sixtop_uResGenerateRemoveLinkList(&type, &frameID, &flag, linklist, sixtopNeighAddr);
    
    if(success == FALSE){
      // free the packet
      openqueue_freePacketBuffer(sixtopPkt);
      sixtop_vars.State = S_IDLE;
      return;
    }
    //set SubFrameAndLinkIE
    len += processIE_prependSixtopGeneralSheduleIE(sixtopPkt,type,frameID,flag,linklist);
    //set uResopcodeIE
    len += processIE_prependSixtopOpcodeIE(sixtopPkt,SIXTOP_REMOVE_SOFT_CELL_REQUEST);

    packetfunctions_reserveHeaderSize(sixtopPkt, sizeof(payload_IE_descriptor_t));//the payload IE header
    //prepare IE headers and copy them to the sixtopPkt
   
    payload_IE_desc.length_groupid_type  = len<<IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
    payload_IE_desc.length_groupid_type |= (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_DESC_TYPE_LONG); //
   
    //copy header into the packet
    //little endian
    sixtopPkt->payload[0]= payload_IE_desc.length_groupid_type & 0xFF;
    sixtopPkt->payload[1]= (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
    
    //I has an IE in my payload
    sixtopPkt->l2_IEListPresent = IEEE154_IELIST_YES;
  
    sixtop_send(sixtopPkt);
    
    sixtop_vars.State = S_WAIT_REMOVELINKREQUEST_SENDDONE;
  }
  ENABLE_INTERRUPTS();
}

//======= from upper layer

owerror_t sixtop_send(OpenQueueEntry_t *msg) {
   msg->owner        = COMPONENT_SIXTOP;
   msg->l2_frameType = IEEE154_TYPE_DATA;
   if (msg->l2_IEListPresent == IEEE154_IELIST_NO) {
      return sixtop_send_internal(msg,IEEE154_IELIST_NO,IEEE154_FRAMEVERSION_2006);
   } else {
      return sixtop_send_internal(msg,IEEE154_IELIST_YES,IEEE154_FRAMEVERSION);
   }
}

//======= from lower layer

void task_sixtopNotifSendDone() {
   OpenQueueEntry_t* msg;
   // get recently-sent packet from openqueue
   msg = openqueue_sixtopGetSentPacket();
   if (msg==NULL) {
      // log the error
      openserial_printCritical(COMPONENT_SIXTOP,ERR_NO_SENT_PACKET,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      return;
   }
   // declare it as mine
   msg->owner = COMPONENT_SIXTOP;
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
   if (msg->creator == COMPONENT_SIXTOP) {
      if (msg->l2_frameType==IEEE154_TYPE_BEACON) {
         // this is a ADV
         
         // not busy sending ADV anymore
         sixtop_vars.busySendingAdv = FALSE;
      } else {
         // this is a KA
         
         // not busy sending KA anymore
         sixtop_vars.busySendingKa = FALSE;
      }
      // discard packets
      openqueue_freePacketBuffer(msg);
      // restart a random timer
      sixtop_vars.periodMaintenance = 872+(openrandom_get16b()&0xff);
      opentimers_setPeriod(sixtop_vars.timerId,
                           TIME_MS,
                           sixtop_vars.periodMaintenance);
   } else {
     if(msg->creator == COMPONENT_RESERVATION) {
       sixtop_sendDone(msg,msg->l2_sendDoneError);
     }
     else
      // send the rest up the stack
      iphc_sendDone(msg,msg->l2_sendDoneError);
   }
}

void task_sixtopNotifReceive() {
   OpenQueueEntry_t* msg;
   uint16_t lenIE=0;//len of IEs being received if any.
   // get received packet from openqueue
   msg = openqueue_sixtopGetReceivedPacket();
   if (msg==NULL) {
      // log the error
      openserial_printCritical(COMPONENT_SIXTOP,ERR_NO_RECEIVED_PACKET,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      // abort
      return;
   }
   
   // declare it as mine
   msg->owner = COMPONENT_SIXTOP;
   
   if(msg->l2_IEListPresent == IEEE154_IELIST_YES       &&
      msg->l2_frameType == IEEE154_TYPE_DATA            &&
      sixtop_processIEs(msg, &lenIE) == FALSE) {
       // free the packet's RAM memory
       openqueue_freePacketBuffer(msg);
       //log error
       return;
   }
   
   // toss the IEs including Synch
   packetfunctions_tossHeader(msg,lenIE);
   
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
         openserial_printError(COMPONENT_SIXTOP,ERR_MSG_UNKNOWN_TYPE,
                               (errorparameter_t)msg->l2_frameType,
                               (errorparameter_t)0);
         break;
   }
}

//=========================== private =========================================

void sixtop_sendDone(OpenQueueEntry_t* msg, owerror_t error){
  uint8_t i,numOfCells;
  uint8_t* ptr;
  sixtop_linkInfo_subIE_t linklist[MAXSCHEDULEDCELLS];
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  memset(linklist,0,MAXSCHEDULEDCELLS*sizeof(sixtop_linkInfo_subIE_t));
  
  ptr = msg->l2_scheduleIE_cellObjects;
  numOfCells = msg->l2_scheduleIE_numOfCells;
  msg->owner = COMPONENT_RESERVATION;
  
  if(error == E_FAIL) {
    sixtop_vars.State = S_IDLE;
    openqueue_freePacketBuffer(msg);
    return;
  }

  switch (sixtop_vars.State)
  {
  case S_WAIT_SIXTOP_LINKREQUEST_SENDDONE:
    sixtop_vars.State = S_WAIT_FORRESPONSE;
    break;
  case S_WAIT_SIXTOP_LINKRESPONSE_SENDDONE:
    sixtop_vars.State = S_IDLE;
    break;
  case S_WAIT_REMOVELINKREQUEST_SENDDONE:
    if(error == E_SUCCESS && numOfCells > 0){
        for (i=0;i<numOfCells;i++){
         //TimeSlot 2B
         linklist[i].tsNum = (*(ptr))<<8;
         linklist[i].tsNum  |= *(ptr+1);
         //Ch.Offset 2B
         linklist[i].choffset = (*(ptr+2))<<8;
         linklist[i].choffset  |= *(ptr+3);
         //LinkOption bitmap 1B
         linklist[i].linkoptions = *(ptr+4);
         ptr += 5;
      }
      sixtop_removeLinksFromSchedule(msg->l2_scheduleIE_frameID,numOfCells,linklist,&(msg->l2_nextORpreviousHop));
    }
    sixtop_vars.State = S_IDLE;
    leds_debug_off();
    break;
  default:
    //log error
    break;
  }
  
  // discard reservation packets this component has created
  openqueue_freePacketBuffer(msg);
  
  ENABLE_INTERRUPTS();
}

void sixtop_linkResponse(bool success, open_addr_t* tempNeighbor,uint8_t bandwidth, sixtop_generalschedule_subIE_t* schedule_ie){
    OpenQueueEntry_t* sixtopPkt;
    uint8_t len=0;
    uint8_t bw;
    uint8_t type,frameID,flag;
    sixtop_linkInfo_subIE_t* linklist;
    payload_IE_descriptor_t payload_IE_desc;  
    
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    
    // get parameters for scheduleIE
    type = schedule_ie->type;
    frameID = schedule_ie->frameID;
    flag = schedule_ie->flag;
    linklist = schedule_ie->linklist;
  
    // get a free packet buffer
    sixtopPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (sixtopPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    
    //changing state to resLinkRespone command
    sixtop_vars.State = S_SIXTOP_LINKRESPONSE_SEND;
    
    // declare ownership over that packet
    sixtopPkt->creator = COMPONENT_RESERVATION;
    sixtopPkt->owner   = COMPONENT_RESERVATION;
    
    memcpy(&(sixtopPkt->l2_nextORpreviousHop),tempNeighbor,sizeof(open_addr_t));
    
    //set SubFrameAndLinkIE
    len += processIE_prependSixtopGeneralSheduleIE(sixtopPkt,type,frameID,flag,linklist);
    if(success){
      bw = bandwidth;
    } else{
      bw = 0;
    }
    //set uResBandwidthIE
    len += processIE_prependSixtopBandwidthIE(sixtopPkt,bw,SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE);
    //set uResopcodeIE
    len += processIE_prependSixtopOpcodeIE(sixtopPkt,SIXTOP_SOFT_CELL_RESPONSE);

    packetfunctions_reserveHeaderSize(sixtopPkt, sizeof(payload_IE_descriptor_t));//the payload IE header
    //prepare IE headers and copy them to the sixtopPkt
   
    payload_IE_desc.length_groupid_type  = len<<IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
    payload_IE_desc.length_groupid_type |= (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_DESC_TYPE_LONG); //
   
    //copy header into the packet
    //little endian
    sixtopPkt->payload[0]= payload_IE_desc.length_groupid_type & 0xFF;
    sixtopPkt->payload[1]= (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
    
    //I has an IE in my payload
    sixtopPkt->l2_IEListPresent = IEEE154_IELIST_YES;
  
    sixtop_send(sixtopPkt);
  
    sixtop_vars.State = S_WAIT_SIXTOP_LINKRESPONSE_SENDDONE;
    ENABLE_INTERRUPTS();
}

bool sixtop_availableCells(uint8_t frameID, uint8_t numOfCells, sixtop_linkInfo_subIE_t* linklist, uint8_t bandwidth){
  uint8_t i=0,bw=bandwidth;
  bool available = FALSE;
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  if(bw == 0 || bw>MAXSCHEDULEDCELLS || numOfCells>MAXSCHEDULEDCELLS){
    // log wrong parameter error TODO
    
    available = FALSE;
  } else {
    do{
      if(schedule_isSlotOffsetAvailable(linklist[i].tsNum) == TRUE){
        bw--;
      } else {
        linklist[i].linkoptions = CELLTYPE_OFF;
      }
      i++;
    }while(i<numOfCells && bw>0);
      
    if(bw==0){
      //the rest link will not be scheduled, mark them as off type
      while(i<numOfCells){
        linklist[i].linkoptions = CELLTYPE_OFF;
        i++;
      }
      available = TRUE;// local schedule can statisfy the bandwidth of cell request.
    } else {
      available = FALSE;// local schedule can't statisfy the bandwidth of cell request
    }
  }
  ENABLE_INTERRUPTS();
  return available;
}

bool sixtop_uResGenerateCandidataLinkList(uint8_t* type,uint8_t* frameID,uint8_t* flag, sixtop_linkInfo_subIE_t* linklist){
  uint8_t i,j=0;
  //implement your algorithm here to generate candidate link list
  *type = 1;
  *frameID = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
  *flag = 1; // the cells listed in linklist are available to be schedule.
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  for(i=0;i<MAXACTIVESLOTS;i++){
    if(schedule_isSlotOffsetAvailable(i) == TRUE){
      linklist[j].tsNum = i;
      linklist[j].choffset = 0;  // default channeloffset
      linklist[j].linkoptions = CELLTYPE_TX; // always schedule tx 
      j++;
      if(j==MAXSCHEDULEDCELLS){
        break;
      }
    }
  }
  ENABLE_INTERRUPTS();
  if(j==0){
    return FALSE;
  }else{
    return TRUE;
  }
}

bool sixtop_uResGenerateRemoveLinkList(uint8_t* type,uint8_t* frameID,uint8_t* flag,sixtop_linkInfo_subIE_t* linklist,open_addr_t* neighbor){
  uint8_t i,j=0;
  //implement your algorithm here to generate candidata link list
  *type = 1;
  *frameID = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
  *flag = 1;
  slotinfo_element_t  info;
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  for(i=0;i<MAXACTIVESLOTS;i++){
    schedule_getSlotInfo(i,neighbor,&info);
    if(info.link_type == CELLTYPE_TX){
      linklist[j].tsNum = i;
      linklist[j].choffset = info.channelOffset;  // default channeloffset
      linklist[j].linkoptions = CELLTYPE_TX; // always schedule tx 
      j++;
      break; // only delete one cell
    }
  }
  ENABLE_INTERRUPTS();
  
  if(j==0){
    return FALSE;
  }else{
    return TRUE;
  }
}

void sixtop_addLinksToSchedule(uint8_t slotframeID,uint8_t numOfLinks,sixtop_linkInfo_subIE_t* linklist,open_addr_t* previousHop,uint8_t state){
  uint8_t i,j=0;
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  //set schedule according links
  open_addr_t temp_neighbor;
  for(i = 0;i<MAXSCHEDULEDCELLS;i++)
  {
      //only schedule when the request side wants to schedule a tx cell
      if(linklist[i].linkoptions == CELLTYPE_TX)
      {
        switch(state) {
          case S_SIXTOP_LINKREQUEST_RECEIVE:
            memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
            //add a RX link
            schedule_addActiveSlot(linklist[i].tsNum,
              CELLTYPE_RX,
              FALSE,
              linklist[i].choffset,
              &temp_neighbor
            );
            break;
          case S_SIXTOP_LINKRESPONSE_RECEIVE:
            memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
            //add a TX link
            schedule_addActiveSlot(linklist[i].tsNum,
              CELLTYPE_TX,
              FALSE,
              linklist[i].choffset,
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
  ENABLE_INTERRUPTS();
}

void sixtop_removeLinksFromSchedule(uint8_t slotframeID,uint8_t numOfLink,sixtop_linkInfo_subIE_t* linklist,open_addr_t* previousHop){
  uint8_t i=0;
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  //set schedule according links
  for(i=0;i<numOfLink;i++)
  {   
      if(linklist[i].linkoptions == CELLTYPE_TX)
      {
        //remove link from shedule
        schedule_removeActiveSlot(linklist[i].tsNum,
          previousHop);
      }
  }
  ENABLE_INTERRUPTS();
}

//======= timer

/**
\brief Timer handlers which triggers MAC management task.

This function is called in task context by the scheduler after the RES timer
has fired. This timer is set to fire every second, on average.

The body of this function executes one of the MAC management task.
*/
void timers_sixtop_fired(void) {
   sixtop_vars.MacMgtTaskCounter = (sixtop_vars.MacMgtTaskCounter+1)%ADVTIMEOUT;
   
   switch (sixtop_vars.MacMgtTaskCounter) {
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

void timers_sixtop_timeout_fired(void) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // timeout timer fired, reset the state of sixtop to idle
   sixtop_vars.State = S_IDLE;
   ENABLE_INTERRUPTS();
}

port_INLINE bool sixtop_processIEs(OpenQueueEntry_t* pkt, uint16_t * lenIE) {
  uint8_t ptr;
  uint8_t temp_8b,gr_elem_id,subid;
  uint16_t temp_16b,len,sublen;
  sixtop_opcode_subIE_t opcode_ie;
  sixtop_bandwidth_subIE_t bandwidth_ie;
  sixtop_generalschedule_subIE_t schedule_ie;
  
  memset(&opcode_ie,0,sizeof(sixtop_opcode_subIE_t));
  memset(&bandwidth_ie,0,sizeof(sixtop_bandwidth_subIE_t));
  memset(&schedule_ie,0,sizeof(sixtop_generalschedule_subIE_t));  
  
  ptr=0;
  //candidate IE header  if type ==0 header IE if type==1 payload IE
  temp_8b = *((uint8_t*)(pkt->payload)+ptr);
  ptr++;
  temp_16b = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr))<< 8);
  ptr++;
  *lenIE = ptr;
  if ((temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) == IEEE802154E_DESC_TYPE_PAYLOAD_IE){
  //payload IE - last bit is 1
     len=(temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
     gr_elem_id= (temp_16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT;
  }else {
  //header IE - last bit is 0
     len=(temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK)>>IEEE802154E_DESC_LEN_HEADER_IE_SHIFT;
     gr_elem_id = (temp_16b & IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK)>>IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT; 
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
        if ((temp_16b & IEEE802154E_DESC_TYPE_LONG) == IEEE802154E_DESC_TYPE_LONG){
           //long sub-IE - last bit is 1
           sublen=(temp_16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_LEN_LONG_MLME_IE_SHIFT;
           subid= (temp_16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT; 
        }else {
           //short IE - last bit is 0
           sublen =(temp_16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
           subid = (temp_16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT; 
        }
        switch(subid){
          case SIXTOP_MLME_RES_OPCODE_IE_SUBID:
            processIE_retrieveSixtopOpcodeIE(pkt,&ptr,&opcode_ie);
            break;
          case SIXTOP_MLME_RES_BANDWIDTH_IE_SUBID:
            processIE_retrieveSixtopBandwidthIE(pkt,&ptr,&bandwidth_ie);
            break;
          case SIXTOP_MLME_RES_TRACKID_IE_SUBID:
            break;
          case SIXTOP_MLME_RES_GENERAL_SCHEDULE_IE_SUBID:
            processIE_retrieveSixtopGeneralSheduleIE(pkt,&ptr,&schedule_ie);
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
    sixtop_notifyReceiveCommand(&opcode_ie,&bandwidth_ie,&schedule_ie,&(pkt->l2_nextORpreviousHop));
  }
  
  return TRUE;
}

void sixtop_notifyReceiveCommand(sixtop_opcode_subIE_t* opcode_ie, 
                                 sixtop_bandwidth_subIE_t* bandwidth_ie, 
                                 sixtop_generalschedule_subIE_t* schedule_ie,
                                 open_addr_t* addr){
     INTERRUPT_DECLARATION();
     DISABLE_INTERRUPTS();
     
      switch(opcode_ie->opcode)
      {
      case SIXTOP_SOFT_CELL_REQ:
        if(sixtop_vars.State == S_IDLE)
        {
          sixtop_vars.State = S_SIXTOP_LINKREQUEST_RECEIVE;
          //received uResCommand is reserve link request
          sixtop_notifyReceiveLinkRequest(bandwidth_ie,schedule_ie,addr);
        }
        break;
      case SIXTOP_SOFT_CELL_RESPONSE:
        if(sixtop_vars.State == S_WAIT_FORRESPONSE)
        {
          sixtop_vars.State = S_SIXTOP_LINKRESPONSE_RECEIVE;
          //received uResCommand is reserve link response
          sixtop_notifyReceiveLinkResponse(bandwidth_ie,schedule_ie,addr);
        }
        break;
      case SIXTOP_REMOVE_SOFT_CELL_REQUEST:
        if(sixtop_vars.State == S_IDLE)
        {
          sixtop_vars.State = S_REMOVELINKREQUEST_RECEIVE;
          //received uResComand is remove link request
          sixtop_notifyReceiveRemoveLinkRequest(schedule_ie,addr);
        }
        break;
      case SIXTOP_HARD_CELL_REQ:
        break;
      case SIXTOP_REMOVE_HARD_CELL:
        break;
      default:
         // log the error
        break;
      }
    ENABLE_INTERRUPTS();
}

//reservation
void sixtop_notifyReceiveLinkRequest(sixtop_bandwidth_subIE_t* bandwidth_ie, sixtop_generalschedule_subIE_t* schedule_ie,open_addr_t* addr){
  uint8_t bw,numOfcells,frameID;
  bool sucess;
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  frameID = schedule_ie->frameID;
  numOfcells = schedule_ie->numberOfcells;
  bw = bandwidth_ie->numOfLinks;
  
  // need to check whether the links are available to be scheduled.
  if(bw > numOfcells                                                 ||
     schedule_ie->frameID != bandwidth_ie->slotframeID               ||
     sixtop_availableCells(frameID, numOfcells, schedule_ie->linklist, bw) == FALSE){
     sucess = FALSE;
  } else {
    sixtop_addLinksToSchedule(frameID,bw,schedule_ie->linklist,addr,sixtop_vars.State);
    sucess = TRUE;
  }
  
  //call link response command
  sixtop_linkResponse(sucess,addr,bandwidth_ie->numOfLinks,schedule_ie);
  ENABLE_INTERRUPTS();
}

void sixtop_notifyReceiveLinkResponse(sixtop_bandwidth_subIE_t* bandwidth_ie, sixtop_generalschedule_subIE_t* schedule_ie,open_addr_t* addr){
  uint8_t bw,numOfcells,frameID;
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  frameID = schedule_ie->frameID;
  numOfcells = schedule_ie->numberOfcells;
  bw = bandwidth_ie->numOfLinks;
  
  if(bw == 0){
    // link request failed
    // todo- should inform some one
    return;
  } else{
    // need to check whether the links are available to be scheduled.
    if(bw != numOfcells                                                ||
       schedule_ie->frameID != bandwidth_ie->slotframeID               ||
       sixtop_availableCells(frameID, numOfcells, schedule_ie->linklist, bw) == FALSE){
       // link request failed,inform uplayer
    } else {
      sixtop_addLinksToSchedule(frameID,bw,schedule_ie->linklist,addr,sixtop_vars.State);
      // link request success,inform uplayer
    }
  }
  leds_debug_off();
  sixtop_vars.State = S_IDLE;
  
  opentimers_stop(sixtop_vars.TOtimerId);
  
  ENABLE_INTERRUPTS();
}

void sixtop_notifyReceiveRemoveLinkRequest(sixtop_generalschedule_subIE_t* schedule_ie,open_addr_t* addr){
  uint8_t numOfCells,frameID;
  
  INTERRUPT_DECLARATION();
  DISABLE_INTERRUPTS();
  
  sixtop_linkInfo_subIE_t* linklist;
  numOfCells = schedule_ie->numberOfcells;
  frameID = schedule_ie->frameID;
  linklist = schedule_ie->linklist;
  leds_debug_on();
  
  sixtop_removeLinksFromSchedule(frameID,numOfCells,linklist,addr);
  
  sixtop_vars.State = S_IDLE;
  leds_debug_off();
  
  ENABLE_INTERRUPTS();
}

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
owerror_t sixtop_send_internal(OpenQueueEntry_t* msg, uint8_t iePresent, uint8_t frameVersion) {
   // assign a number of retries
   if (packetfunctions_isBroadcastMulticast(&(msg->l2_nextORpreviousHop))==TRUE) {
      msg->l2_retriesLeft = 1;
   } else {
      msg->l2_retriesLeft = TXRETRIES;
   }
   // record this packet's dsn (for matching the ACK)
   msg->l2_dsn = sixtop_vars.dsn++;
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
   msg->owner  = COMPONENT_SIXTOP_TO_IEEE802154E;
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
   uint8_t len = 0;
   
   if (ieee154e_isSynch()==FALSE) {
      // I'm not sync'ed
      
      // delete packets genereted by this module (ADV and KA) from openqueue
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm now busy sending an ADV
      sixtop_vars.busySendingAdv = FALSE;
      
      // stop here
      return;
   }
   
   if (sixtop_vars.busySendingAdv==TRUE) {
      // don't continue if I'm still sending a previous ADV
   }
   
   // if I get here, I will send an ADV
   
   // get a free packet buffer
   adv = openqueue_getFreePacketBuffer(COMPONENT_SIXTOP);
   if (adv==NULL) {
      openserial_printError(COMPONENT_SIXTOP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   
   // declare ownership over that packet
   adv->creator = COMPONENT_SIXTOP;
   adv->owner   = COMPONENT_SIXTOP;
   
   // reserve space for ADV-specific header
   // xv poipoi -- reserving for IEs  -- reverse order.
   //TODO reserve here for slotframe and link IE with minimal schedule information
   len += processIE_prependFrameLinkIE(adv);
   len += processIE_prependSyncIE(adv);
    
   packetfunctions_reserveHeaderSize(adv, sizeof(payload_IE_descriptor_t));//the payload IE header
   //prepare IE headers and copy them to the ADV 
   
   payload_IE_desc.length_groupid_type = len<<IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
   payload_IE_desc.length_groupid_type |=  (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_DESC_TYPE_LONG);
   
   //copy header into the packet
   //little endian
   adv->payload[0]= payload_IE_desc.length_groupid_type & 0xFF;
   adv->payload[1]= (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
  
   // some l2 information about this packet
   adv->l2_frameType                     = IEEE154_TYPE_BEACON;
   adv->l2_nextORpreviousHop.type        = ADDR_16B;
   adv->l2_nextORpreviousHop.addr_16b[0] = 0xff;
   adv->l2_nextORpreviousHop.addr_16b[1] = 0xff;
   
   //I has an IE in my payload
   adv->l2_IEListPresent = IEEE154_IELIST_YES;
   
   // put in queue for MAC to handle
   sixtop_send_internal(adv,IEEE154_IELIST_YES,IEEE154_FRAMEVERSION);
   
   // I'm now busy sending an ADV
   sixtop_vars.busySendingAdv = TRUE;
}

/**
\brief Send an keep-alive message, if necessary.

This is one of the MAC management tasks. This function inlines in the
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
      openqueue_removeAllCreatedBy(COMPONENT_SIXTOP);
      
      // I'm now busy sending a KA
      sixtop_vars.busySendingKa = FALSE;
      
      // stop here
      return;
   }
   
   if (sixtop_vars.busySendingKa==TRUE) {
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
   
   // put in queue for MAC to handle
   sixtop_send_internal(kaPkt,IEEE154_IELIST_NO,IEEE154_FRAMEVERSION_2006);
   
   // I'm now busy sending a KA
   sixtop_vars.busySendingKa = TRUE;

#ifdef OPENSIM
   debugpins_ka_set();
   debugpins_ka_clr();
#endif
}

void sixtop_timer_cb() {
   scheduler_push_task(timers_sixtop_fired,TASKPRIO_SIXTOP);
}

void sixtop_timeout_timer_cb() {
   scheduler_push_task(timers_sixtop_timeout_fired,TASKPRIO_SIXTOP_TIMEOUT);
}
