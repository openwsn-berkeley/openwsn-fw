#include "openwsn.h"
#include "res.h"
#include "idmanager.h"
#include "openserial.h"
#include "openqueue.h"
#include "neighbors.h"
#include "iphc.h"
#include "leds.h"
#include "packetfunctions.h"
#include "openrandom.h"
#include "schedule.h"
#include "scheduler.h"
#include "bsp_timer.h"
#include "opentimers.h"
#include "processIE.h"
#include "IEfield.h"
#include "reservation.h"
#include "processIE.h"
#include "ieee802154e.h"
#include "ieee802154.h"

//============================ define =========================================

void reservation_timer_cb();

//=========================== typedef =========================================
//#define NO_UPPER_LAYER_CALLING_RESERVATION
#define ONE_LINK 1

reservation_vars_t reservation_vars;
//=========================== prototypes ======================================

//admin
void    reservation_init(){
  memset(&reservation_vars,0,sizeof(reservation_vars_t));
  reservation_vars.periodReservation = RESPERIOD;
  reservation_vars.timerId = opentimers_start(
      reservation_vars.periodReservation,
      TIMER_PERIODIC,
      TIME_MS,
      reservation_timer_cb
   );
}
// public

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