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


//=========================== typedef =========================================
#define NO_UPPER_LAYER_CALLING_RESERVATION

//=========================== variables =======================================
typedef struct {
  bool                 busySending;     // TRUE when busy sending an reservation command
  open_addr_t          ResNeighborAddr;
  reservation_state_t  State;
  uint8_t              commandID;
  bandwidth_vars_t     bandwidth_vars;
  uint8_t              button_event; //when requestOrRemoveLink%3 is 0 or 1, call uResLinkRequest; when the value is 2, call uResRemoveLink.
} reservation_vars_t;

reservation_vars_t reservation_vars;
//=========================== prototypes ======================================

//admin
void    reservation_init(){
  memset(&reservation_vars,0,sizeof(reservation_vars_t));
}
// public
uint8_t reservation_getuResCommandID(){
  return reservation_vars.commandID;
}

bandwidth_vars_t reservation_getuResBandwidth() {
  return reservation_vars.bandwidth_vars;
}

void    reservation_setuResCommandID(uint8_t commandID){
  reservation_vars.commandID = commandID;
}

void    reservation_setuResBandwidth(uint8_t numOfLinks, uint8_t slotframeID){
  reservation_vars.bandwidth_vars.numOfLinks    = numOfLinks;
  reservation_vars.bandwidth_vars.slotframeID   = slotframeID;
}

void    reservation_notifyReceiveuResLinkRequest(OpenQueueEntry_t* msg){
  
  //qw : indicate receiving ResLinkRequest
  leds_debug_toggle();
  
  uResBandwidthIEcontent_t* tempBandwidthIE = processIE_getuResBandwidthIEcontent();
  //record bandwidth information
  reservation_vars.bandwidth_vars.numOfLinks  = tempBandwidthIE->numOfLinks;
  reservation_vars.bandwidth_vars.slotframeID = tempBandwidthIE->slotframeID;
  
  frameAndLinkIEcontent_t* tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();
    
  for(uint8_t i = 0; i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
  {
    uint8_t slotframeID    = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
    uint8_t numOfLink      = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;
    if(reservation_vars.bandwidth_vars.slotframeID == slotframeID)
      //allocate links for neighbor
      schedule_allocateLinks(slotframeID,numOfLink,reservation_vars.bandwidth_vars.numOfLinks);
  }
  
  schedule_addLinksToSchedule(reservation_vars.bandwidth_vars.slotframeID,&(msg->l2_nextORpreviousHop),reservation_vars.bandwidth_vars.numOfLinks,reservation_vars.State);
  
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
    //qw: turn off yellow led when finish
    leds_debug_toggle();
}

void    reservation_notifyReceiveRemoveLinkRequest(OpenQueueEntry_t* msg){
  
  frameAndLinkIEcontent_t* tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();
    
  for(uint8_t i = 0; i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
  {
    uint8_t slotframeID    = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
    uint16_t slotframeSize = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeSize;
    uint8_t numOfLink      = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;

    schedule_removeLinksFromSchedule(slotframeID,slotframeSize,numOfLink,&(msg->l2_nextORpreviousHop),reservation_vars.State);
  }
  
  reservation_vars.State = S_IDLE;
}

void    reservation_notifyReceiveScheduleRequest(OpenQueueEntry_t* msg){
}

void    reservation_notifyReceiveScheduleResponse(OpenQueueEntry_t* msg){
}


//call res layer
void    reservation_sendDone(OpenQueueEntry_t* msg, error_t error){
      msg->owner = COMPONENT_RESERVATION;

      switch (reservation_vars.State)
      {
      case S_WAIT_RESLINKREQUEST_SENDDONE:
        reservation_vars.State = S_WAIT_FORRESPONSE;
        break;
      case S_WAIT_RESLINKRESPONSE_SENDDONE:
        reservation_vars.State = S_IDLE;
        //qw turn off yellow light when finish
        leds_debug_toggle();
        break;
      case S_WAIT_REMOVELINKREQUEST_SENDDONE:
        reservation_vars.State = S_IDLE;
        //qw turn off yellow light when finish
        leds_debug_toggle();
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
//call by up layer
void reservation_linkRequest() {
  
  if(reservation_vars.State != S_IDLE)
    return;
  
  leds_debug_toggle();
  
  OpenQueueEntry_t* reservationPkt;
  open_addr_t*      reservationNeighAddr;
  
  reservationNeighAddr = neighbors_reservationNeighbor();
  if(reservationNeighAddr!=NULL){
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
  
    //set uRes command ID
    reservation_setuResCommandID(RESERCATIONLINKREQ);
    
    uint8_t numOfSlotframes = schedule_getNumSlotframe();
    
#ifdef NO_UPPER_LAYER_CALLING_RESERVATION
    //generate candidata links
    for(uint8_t i=0;i<numOfSlotframes;i++)
      schedule_uResGenerateCandidataLinkList(i);
#endif
    //set SubFrameAndLinkIE
    processIE_setSubFrameAndLinkIE();
    //set uResCommandIE
    processIE_setSubuResCommandIE();

    //set uResBandwidthIE
    processIE_setSubuResBandwidthIE();
    
    //reset bandwidth to 0
    memset(&(reservation_vars.bandwidth_vars),0,sizeof(bandwidth_vars_t));
    //set IE after set all required subIE
    processIE_setMLME_IE();
    //add an IE to adv's payload
    IEFiled_prependIE(reservationPkt);
    
    //I has an IE in my payload
    reservationPkt->l2_IEListPresent = 1;
  
    res_send(reservationPkt);
    
    reservation_vars.State = S_WAIT_RESLINKREQUEST_SENDDONE;
  }
}

void  reservation_linkResponse(open_addr_t* tempNeighbor){
  
  //qw: seems useless
  //if(reservation_vars.State != S_RESLINKREQUEST_RECEIVE)
    //return;
  //qw: move to when just receiving packet
  //leds_debug_toggle();
  
  leds_debug_toggle();
  
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
    
    //set uRes command ID
    reservation_setuResCommandID(RESERCATIONLINKRESPONSE);

    //set SubFrameAndLinkIE
    processIE_setSubFrameAndLinkIE();
    //set uResCommandIE
    processIE_setSubuResCommandIE();

    //set IE after set all required subIE
    processIE_setMLME_IE();
    //add an IE to adv's payload
    IEFiled_prependIE(reservationPkt);
    
    //I has an IE in my payload
    reservationPkt->l2_IEListPresent = 1;
  
    res_send(reservationPkt);
  
    reservation_vars.State = S_WAIT_RESLINKRESPONSE_SENDDONE;
}

//remove link command
void reservation_removeLinkRequest(){
  
  if(reservation_vars.State != S_IDLE)
    return;
  
  leds_debug_toggle();
  
  OpenQueueEntry_t* reservationPkt;
  open_addr_t*      reservationNeighAddr;
  
  reservationNeighAddr = neighbors_reservationNeighbor();
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
  
    //set uRes command ID
    reservation_setuResCommandID(RESERVATIONREMOVELINKREQUEST);
    
    uint8_t numOfSlotframes = schedule_getNumSlotframe();
#ifdef NO_UPPER_LAYER_CALLING_RESERVATION
    Link_t tempLink;
    tempLink.channelOffset      = 0;
    tempLink.slotOffset         = 7;
    tempLink.linktype           = CELLTYPE_RX;
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
    processIE_setSubuResCommandIE();

    //set IE after set all required subIE
    processIE_setMLME_IE();
    //add an IE to adv's payload
    IEFiled_prependIE(reservationPkt);
    
    //I has an IE in my payload
    reservationPkt->l2_IEListPresent = 1;
  
    res_send(reservationPkt);
    
    reservation_vars.State = S_WAIT_REMOVELINKREQUEST_SENDDONE;
  }
}

void reservation_pretendSendData(){
  //pretend sending an data from upper layer
  OpenQueueEntry_t* reservationPkt;
  open_addr_t*      reservationNeighAddr;
  
  reservationNeighAddr = neighbors_reservationNeighbor();
  if(reservationNeighAddr!=NULL){
    // get a free packet buffer
    reservationPkt = openqueue_getFreePacketBuffer(COMPONENT_RESERVATION);
  
    if (reservationPkt==NULL) {
      openserial_printError(COMPONENT_RESERVATION,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
    }
    
    // pretend that I am COMPONENT_UPPLAYER
    reservationPkt->creator = COMPONENT_UPPLAYER;
    reservationPkt->owner   = COMPONENT_RESERVATION;
         
    memcpy(&(reservationPkt->l2_nextORpreviousHop),reservationNeighAddr,sizeof(open_addr_t));
    
    uint8_t  Halloween[13];
    Halloween[0]  = 0xff;
    Halloween[1]  = (uint8_t)'T';
    Halloween[2]  = (uint8_t)'r';
    Halloween[3]  = (uint8_t)'i';
    Halloween[4]  = (uint8_t)'c';
    Halloween[5]  = (uint8_t)'k';
    Halloween[6]  = (uint8_t)'O';
    Halloween[7]  = (uint8_t)'r';
    Halloween[8]  = (uint8_t)'T';
    Halloween[9]  = (uint8_t)'r';
    Halloween[10]  = (uint8_t)'e';
    Halloween[11] = (uint8_t)'a';
    Halloween[12] = (uint8_t)'t';
    packetfunctions_reserveHeaderSize(reservationPkt,sizeof(Halloween));
    
    memcpy(reservationPkt->payload,Halloween,sizeof(Halloween));
    
    res_send(reservationPkt);
  }
}

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
           
//event
void isr_reservation_button() {
  
  switch (reservation_vars.button_event){
  case 0:
  case 1:
    //set slotframeID and bandwidth
    reservation_setuResBandwidth(1,0);
  
    reservation_linkRequest();
    break;
  case 2:
    reservation_removeLinkRequest();
    break;
  default:
    //pretend that uppler is sending a data
    reservation_pretendSendData();
  }
  
  reservation_vars.button_event += 1;
  //reservation_vars.button_event = (reservation_vars.button_event+1)%3;
}