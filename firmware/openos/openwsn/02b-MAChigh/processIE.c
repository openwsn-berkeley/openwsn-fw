#include "processIE.h"
#include "openwsn.h"
#include "6top.h"
#include "idmanager.h"
#include "openserial.h"
#include "IEEE802154.h"
#include "openqueue.h"
#include "neighbors.h"
#include "IEEE802154E.h"
#include "schedule.h"
#include "scheduler.h"
#include "packetfunctions.h"

//=========================== variables =======================================
IEHeader_t                IEHeader_vars;
subIE_t                   syncIE_vars;
subIE_t                   frameAndLinkIE_vars;
subIE_t                   timeslotIE_vars;
subIE_t                   channelHoppingIE_vars;
subIE_t                   uResLinkTypeIE_vars;
subIE_t                   uResCommandIE_vars;
subIE_t                   uResBandwidthIE_vars;
subIE_t                   uResGeneralScheduleIE_vars;
syncIEcontent_t           syncIEcontent_vars;
frameAndLinkIEcontent_t   frameAndLinkIEcontent_vars;
timeslotIEcontent_t       timeslotIEcontent_vars;
channelHoppingIEcontent_t channelHoppingIEcontent_vars;
uResLinkTypeIEcontent_t   uResLinkTypeIEcontent_vars;
uResCommandIEcontent_t    uResCommandIEcontent_vars;
uResBandwidthIEcontent_t  uResBandwidthIEcontent_vars;
uResGeneralScheduleIEcontent_t   uResScheduleGeneralIEcontent_vars;
//========================== private ==========================================

//=========================== public ==========================================
//admin
void processIE_init() {
     // initialize IE
    memset(&IEHeader_vars,0,sizeof(IEHeader_t));
    // initialize sync IE
    memset(&syncIE_vars,0,sizeof(subIE_t));
    memset(&syncIEcontent_vars,0,sizeof(syncIEcontent_t));
    // initialize frame&link IE 
    memset(&frameAndLinkIE_vars,0,sizeof(subIE_t));
    memset(&frameAndLinkIEcontent_vars,0,sizeof(frameAndLinkIEcontent_t));
    // initialize timeslot IE  
    memset(&timeslotIE_vars,0,sizeof(subIE_t));
    memset(&timeslotIEcontent_vars,0,sizeof(timeslotIEcontent_t));
    // initialize channel hopping IE 
    memset(&channelHoppingIE_vars,0,sizeof(subIE_t));
    memset(&channelHoppingIEcontent_vars,0,sizeof(channelHoppingIEcontent_t));
    // initialize uRes link type IE
    memset(&uResLinkTypeIE_vars,0,sizeof(subIE_t));
    memset(&uResLinkTypeIEcontent_vars,0,sizeof(uResLinkTypeIEcontent_t));
    // initialize uRes command IE    
    memset(&uResCommandIE_vars,0,sizeof(subIE_t));
    memset(&uResCommandIEcontent_vars,0,sizeof(uResCommandIEcontent_t));
    // initialize uRes bandwidth IE   
    memset(&uResBandwidthIE_vars,0,sizeof(subIE_t)); 
    memset(&uResBandwidthIEcontent_vars,0,sizeof(uResBandwidthIEcontent_t));
    // initialize uRes schedule IE   
    memset(&uResGeneralScheduleIE_vars,0,sizeof(subIE_t));
    memset(&uResScheduleGeneralIEcontent_vars,0,sizeof(uResGeneralScheduleIEcontent_t));
}

//==================set========================
void processIE_setMLME_IE (){
  //set IE length,groupID and type fields
  IEHeader_vars.Length  = 0;
  IEHeader_vars.GroupID = IE_MLME;
  IEHeader_vars.Type    = IE_TYPE_PAYLOA;
  IEHeader_vars.Length  += 2;
  IEHeader_vars.Length  += syncIE_vars.length;
  IEHeader_vars.Length  += frameAndLinkIE_vars.length;
  IEHeader_vars.Length  += timeslotIE_vars.length;
  IEHeader_vars.Length  += channelHoppingIE_vars.length;
  IEHeader_vars.Length  += uResLinkTypeIE_vars.length;
  IEHeader_vars.Length  += uResCommandIE_vars.length;
  IEHeader_vars.Length  += uResBandwidthIE_vars.length;
}

void processIE_setSubSyncIE(){
  //set subIE length,subID and type fields
  /*********************************
  ** the syncIE is added in uRes when sending ADV
  ** the right place to add syncIE should be here.
  
  uint8_t length = 0;
  syncIE_vars.SubID = 26;
  syncIE_vars.type = 0;
  length += 2;
  //set asn(asn will be added in IEEE802154e, res_getADVasn() return 0)
  syncIEcontent_vars.asn = res_getADVasn();
  length += 5;
  syncIEcontent_vars.joinPriority = res_getJoinPriority();
  length += 1;
  syncIE_vars.length = length;
  **
  ** end*/
}	

void processIE_setSubFrameAndLinkIE(){
  //set subIE length,subID and type fields
  uint8_t length = 0;
  frameAndLinkIE_vars.SubID                             = 0x1b;
  frameAndLinkIE_vars.type                              = 0;
  length += 2;
  frameAndLinkIEcontent_vars.numOfSlotframes            = schedule_getNumSlotframe();
  length += 1;

  for(uint8_t i=0;i<frameAndLinkIEcontent_vars.numOfSlotframes;i++)
  {
    //set SlotframeInfo
    frameAndLinkIEcontent_vars.slotframeInfo[i].slotframeID               = i;
    length += 1;
    frameAndLinkIEcontent_vars.slotframeInfo[i].slotframeSize             = schedule_getFrameLength();
    length += 2;
    frameAndLinkIEcontent_vars.slotframeInfo[i].numOfLink                 = schedule_getLinksNumber(i);
    length += 1;
    frameAndLinkIEcontent_vars.slotframeInfo[i].links                     = schedule_getLinksList(i);
    length += 5 * frameAndLinkIEcontent_vars.slotframeInfo[i].numOfLink;
  }
  frameAndLinkIE_vars.length = length;
}

void processIE_setSubTimeslotIE(){
  uint8_t length = 0;
  timeslotIE_vars.SubID     = 0x1c;
  timeslotIE_vars.type      = 0;
  //set timeslotIE's length
  timeslotIE_vars.length    = length;
}

void processIE_setSubChannelHoppingIE(){
  uint8_t length = 0;
  channelHoppingIE_vars.SubID     = 0x9;
  channelHoppingIE_vars.type      = 1;
  //set channelHoppingIE's lenght
  channelHoppingIE_vars.length    = length;
}

void processIE_setSubuResLinkTypeIE(){
  //set subIE length,subID and type fields
  uint8_t length = 0;
  uResLinkTypeIE_vars.SubID       = 0x40;
  uResLinkTypeIE_vars.type        = 0;
  length = length + 2;
  uResLinkTypeIEcontent_vars.numOfSlotframes            = schedule_getNumSlotframe();
  length += 1;

  for(uint8_t i=0;i<uResLinkTypeIEcontent_vars.numOfSlotframes;i++)
  {
    //set SlotframeInfo
    uResLinkTypeIEcontent_vars.slotframeInfo[i].slotframeID               = i;
    length += 1;
    uResLinkTypeIEcontent_vars.slotframeInfo[i].slotframeSize             = schedule_getFrameLength();
    length += 2;
    uResLinkTypeIEcontent_vars.slotframeInfo[i].numOfLink                 = schedule_getLinksNumber(i);
    length += 1;
    uResLinkTypeIEcontent_vars.slotframeInfo[i].links                     = schedule_getLinksList(i);
    length += 5 * uResLinkTypeIEcontent_vars.slotframeInfo[i].numOfLink;
  }
  uResLinkTypeIE_vars.length = length;
}

void processIE_setSubuResCommandIE(uint8_t uResCommandID){
  //set subIE length,subID and type fields
  uint8_t length = 0;
  uResCommandIE_vars.SubID = 0x41;
  uResCommandIE_vars.type  = 0;
  length = length + 2;
  // set uRes Command ID
  uResCommandIEcontent_vars.uResCommandID = uResCommandID;
  length = length + 1;
  uResCommandIE_vars.length = length;
}

void processIE_setSubuResBandwidthIE(uint8_t numOfLinks, uint8_t slotframeID){
  //set subIE length,subID and type fields
  uint8_t length = 0;
  uResBandwidthIE_vars.SubID = 0x42;
  uResBandwidthIE_vars.type  = 0;
  length = length + 2;
  
  uResBandwidthIEcontent_vars.numOfLinks = numOfLinks;
  length = length + 1;
  uResBandwidthIEcontent_vars.slotframeID = slotframeID;
  length = length + 1;
  uResBandwidthIE_vars.length = length;
}

void processIE_setSubuResGeneralSheduleIE(){
}

//=====================get============================= 
void processIE_getMLME_IE(){
}

subIE_t* processIE_getSubSyncIE(){
    return      &syncIE_vars;
}	

subIE_t* processIE_getSubFrameAndLinkIE(){
    return      &frameAndLinkIE_vars;
} 

subIE_t* processIE_getSubChannelHoppingIE(){
    return      &channelHoppingIE_vars;
} 

subIE_t* processIE_getSubTimeslotIE(){
    return      &timeslotIE_vars;
} 

subIE_t* processIE_getSubuResLinkTypeIE(){
  return        &uResLinkTypeIE_vars;
} 

subIE_t* processIE_getSubuResCommandIE(){
  return        &uResCommandIE_vars;
} 

subIE_t* processIE_getSubuResBandwidthIE(){
  return        &uResBandwidthIE_vars;
} 

subIE_t* processIE_getSubuResGeneralSheduleIE(){
  return        &uResGeneralScheduleIE_vars;
}

syncIEcontent_t*  processIE_getSyncIEcontent(){
  return        &syncIEcontent_vars;
}

frameAndLinkIEcontent_t*        processIE_getFrameAndLinkIEcontent(){
  return        &frameAndLinkIEcontent_vars;
}

timeslotIEcontent_t*     processIE_getTimeslotIEcontent(){
  return        &timeslotIEcontent_vars;
}

channelHoppingIEcontent_t*  processIE_getChannelHoppingIEcontent(){
  return        &channelHoppingIEcontent_vars;
}

uResLinkTypeIEcontent_t*        processIE_getuResLinkTypeIEcontent(){
  return        &uResLinkTypeIEcontent_vars;
}

uResBandwidthIEcontent_t*       processIE_getuResBandwidthIEcontent(){
  return        &uResBandwidthIEcontent_vars;
}

uResCommandIEcontent_t*         processIE_getuResCommandIEcontent(){
  return        &uResCommandIEcontent_vars;
}

IEHeader_t*       processIE_getIEHeader(){
  return        &IEHeader_vars;
}

void resetSubIE(){
     // initialize IE
    memset(&IEHeader_vars,0,sizeof(IEHeader_t));
    // initialize sync IE
    memset(&syncIE_vars,0,sizeof(subIE_t));
    // initialize frame&link IE 
    memset(&frameAndLinkIE_vars,0,sizeof(subIE_t));
    // initialize timeslot IE  
    memset(&timeslotIE_vars,0,sizeof(subIE_t));
    // initialize channel hopping IE 
    memset(&channelHoppingIE_vars,0,sizeof(subIE_t));
    // initialize uRes link type IE
    memset(&uResLinkTypeIE_vars,0,sizeof(subIE_t));
    // initialize uRes command IE    
    memset(&uResCommandIE_vars,0,sizeof(subIE_t));
    // initialize uRes bandwidth IE   
    memset(&uResBandwidthIE_vars,0,sizeof(subIE_t)); 
    // initialize uRes schedule IE   
    memset(&uResGeneralScheduleIE_vars,0,sizeof(subIE_t));
}

//add IE to packet

void IEFiled_prependIE  (OpenQueueEntry_t*      msg){
    subIE_t* tempSubIE;
    uint16_t temp_16b = 0;
    
    //add uResBandwidthIE to msg's payload
    temp_16b = 0;
    tempSubIE = processIE_getSubuResBandwidthIE();
    if(tempSubIE->length != 0)
    {
      temp_16b  |=      tempSubIE->length       <<      SUBIE_SHORT_LENGTH;
      temp_16b  |=      SUBIE_RES_BANDWIDTH     <<      SUBIE_SUBID;
      temp_16b  |=      0                       <<      SUBIE_TYPE;
      //add uRes bandwidth IE content
      uResBandwidthIEcontent_t* tempuResBandwidthIEcontent      = processIE_getuResBandwidthIEcontent();
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))                               = tempuResBandwidthIEcontent->numOfLinks;
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))                               = tempuResBandwidthIEcontent->slotframeID;
      //add length subID and subType
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b>>8);
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b);
      //reset bandwidth
      memset(&tempuResBandwidthIEcontent,0,sizeof(uResBandwidthIEcontent_t));
    }
    //reset subIE
    memset(tempSubIE,0,sizeof(subIE_t));
    
    //add uResCommandIE to msg's payload
    temp_16b = 0;
    tempSubIE = processIE_getSubuResCommandIE();
    if(tempSubIE->length != 0)
    {
      temp_16b  |=      tempSubIE->length       <<      SUBIE_SHORT_LENGTH;
      temp_16b  |=      SUBIE_RES_COMMAND       <<      SUBIE_SUBID;
      temp_16b  |=      0                       <<      SUBIE_TYPE;
      //add uResCommandIE content
      uResCommandIEcontent_t*   tempuResCommmandIEcontent       = processIE_getuResCommandIEcontent();
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))                               = tempuResCommmandIEcontent->uResCommandID;
      //add length subID and subType
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b>>8);
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b);
      //reset uResCommandIE
      memset(tempuResCommmandIEcontent,0,sizeof(uResCommandIEcontent_t));
    }
    //reset subIE
    memset(tempSubIE,0,sizeof(subIE_t));
    
    //add uResLinkTypeIE to msg's payload
    temp_16b = 0;
    tempSubIE = processIE_getSubuResLinkTypeIE();
    if(tempSubIE->length != 0)
    {
      temp_16b  |=      tempSubIE->length       <<      SUBIE_SHORT_LENGTH;
      temp_16b  |=      SUBIE_LINKTYPE          <<      SUBIE_SUBID;
      temp_16b  |=      0                       <<      SUBIE_TYPE;
      //add uResLinkType IE content
      uResLinkTypeIEcontent_t*  tempuResLinkTypeIEcontent       = processIE_getuResLinkTypeIEcontent();
      Link_t* tempLink = NULL;
      for(uint8_t i=0;i<tempuResLinkTypeIEcontent->numOfSlotframes;i++)
      {
        //add Links 
        tempLink = tempuResLinkTypeIEcontent->slotframeInfo[i].links;
        // add links
        for(uint8_t j=0;j<tempuResLinkTypeIEcontent->slotframeInfo[i].numOfLink;j++)
        {
          packetfunctions_reserveHeaderSize(msg,sizeof(Link_t));
          packetfunctions_htons(tempLink[j].slotOffset, &(msg->payload[0])); 
          packetfunctions_htons(tempLink[j].channelOffset, &(msg->payload[2]));
          msg->payload[4]     = tempLink[j].link_type;
        }
        //add number of links
        packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
        *((uint8_t*)(msg->payload))     = tempuResLinkTypeIEcontent->slotframeInfo[i].numOfLink;
        //add slotframe length
        packetfunctions_reserveHeaderSize(msg,sizeof(frameLength_t));
        packetfunctions_htons(tempuResLinkTypeIEcontent->slotframeInfo[i].slotframeSize, &(msg->payload[0]));
        //add slotframeID
        packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
        *((uint8_t*)(msg->payload))     = tempuResLinkTypeIEcontent->slotframeInfo[i].slotframeID;
      }
      //reset links
      memset(tempLink,0,MAXACTIVESLOTS*sizeof(Link_t));
      //add number of slotframes
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))     = tempuResLinkTypeIEcontent->numOfSlotframes;
      //add length subID and subType
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b>>8);
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b);
      //reset subIEcontent
      memset(tempuResLinkTypeIEcontent,0,sizeof(uResLinkTypeIEcontent_t));
    }
    //reset subIE
    memset(tempSubIE,0,sizeof(subIE_t));
    
    /*
    //add subIE to msg's payload
    temp_16b = 0;
    if(tempSubIE->length != 0)
    {

    }
    
    temp_16b = 0;
    if(tempSubIE->length != 0)
    {

    }
    */
    //add FrameAndLinksIE to msg's paylaod
    temp_16b = 0;
    tempSubIE = processIE_getSubFrameAndLinkIE();
    if(tempSubIE->length != 0)
    {
      temp_16b  |=      tempSubIE->length       <<      SUBIE_SHORT_LENGTH;
      temp_16b  |=      SUBIE_FRAME_AND_LINK    <<      SUBIE_SUBID;
      temp_16b  |=      0                       <<      SUBIE_TYPE;
      //add frameAndLink IE content
      frameAndLinkIEcontent_t*  tempFrameAndLinkIEcontent       = processIE_getFrameAndLinkIEcontent();
      Link_t* tempLink = NULL;
      for(uint8_t i=0;i<tempFrameAndLinkIEcontent->numOfSlotframes;i++)
      {
        //add Links 
        tempLink = tempFrameAndLinkIEcontent->slotframeInfo[i].links;
        // add links
        for(uint8_t j=0;j<tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;j++)
        {
          packetfunctions_reserveHeaderSize(msg,5); // 2 bytes slotOffset, 2bytes channelOffset+ 1 byte link_type
          packetfunctions_htons(tempLink[j].slotOffset, &(msg->payload[0])); 
          packetfunctions_htons(tempLink[j].channelOffset, &(msg->payload[2]));
          msg->payload[4]     = tempLink[j].link_type;
        }
        //add number of links
        packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
        *((uint8_t*)(msg->payload))     = tempFrameAndLinkIEcontent->slotframeInfo[i].numOfLink;
        //add slotframe length
        packetfunctions_reserveHeaderSize(msg,sizeof(frameLength_t));
        packetfunctions_htons(tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeSize, &(msg->payload[0]));
        //add slotframeID
        packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
        *((uint8_t*)(msg->payload))     = tempFrameAndLinkIEcontent->slotframeInfo[i].slotframeID;
      }
      //reset links
      memset(tempLink,0,MAXACTIVESLOTS*sizeof(Link_t));
      //add number of slotframes
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))     = tempFrameAndLinkIEcontent->numOfSlotframes;
      //add length subID and subType
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b>>8);
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b);
      //reset subIEcontent
      memset(tempFrameAndLinkIEcontent,0,sizeof(frameAndLinkIEcontent_t));
    }
    //reset subIE
    memset(tempSubIE,0,sizeof(subIE_t));
    
    //add syncIE to msg's payload
    temp_16b = 0;
    tempSubIE = processIE_getSubSyncIE();
    if(tempSubIE->length != 0)
    {
      temp_16b  |=      tempSubIE->length       <<      SUBIE_SHORT_LENGTH;
      temp_16b  |=      SUBIE_SYNC              <<      SUBIE_SUBID;
      temp_16b  |=      0                       <<      SUBIE_TYPE;
      //add joinPriority
      syncIEcontent_t*    tempSyncIEcontent       = processIE_getSyncIEcontent();
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))                 = tempSyncIEcontent->joinPriority;
      //reserve asn field
      packetfunctions_reserveHeaderSize(msg,sizeof(asn_t));
      //packetfunctions_writeASN(msg, tempSyncIEcontent->asn);
      msg->l2_ASNpayload       = msg->payload;
      //add length subID and subType
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b>>8);
      packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
      *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b);
      //reset subIEcontent
      memset(tempSyncIEcontent,0,sizeof(syncIEcontent_t));
    }
    //reset subIE
    memset(tempSubIE,0,sizeof(subIE_t));


    //add IE to msg's payload
    temp_16b = 0;
    IEHeader_t* tempIE = processIE_getIEHeader();
    temp_16b    |=      tempIE->Length   <<      IE_LENGTH;
    temp_16b    |=      tempIE->GroupID  <<      IE_GROUPID;
    temp_16b    |=      tempIE->Type     <<      IE_TYPE;
    //add length groupID and Type
    packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
    *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b>>8);
    packetfunctions_reserveHeaderSize(msg,sizeof(uint8_t));
    *((uint8_t*)(msg->payload))               = (uint8_t)(temp_16b);
    //reset IEHeader
    memset(tempIE,0,sizeof(IEHeader_t));
    
}

void IEFiled_retrieveIE (OpenQueueEntry_t*      msg){
     uint8_t       i    = 0;
     //temp subIE variables
     subIE_t      tempSubIE;
     subIE_t*     tempSyncIE;
     subIE_t*     tempFrameAndLinkIE;
     subIE_t*     tempuResLinkTypeIE;
     subIE_t*     tempuResCommandIE;
     subIE_t*     tempuResBandwidthIE;
     //temp subIEContent variables
     syncIEcontent_t*    tempSyncIEcontent;
     frameAndLinkIEcontent_t*  tempFrameAndLinkIEcontent;
     uResLinkTypeIEcontent_t*  tempuResLinkTypeIEcontent;
     uResCommandIEcontent_t*   tempuResCommmandIEcontent;
     uResBandwidthIEcontent_t*  tempuResBandwidthIEcontent;
     
     IEHeader_t* tempIE   = processIE_getIEHeader();
     tempIE->Length     = 0;
     uint16_t temp_16b  = msg->payload[i]+256*msg->payload[i+1];
     i =i + 2;

     tempIE->Length     = (temp_16b               >>      IE_LENGTH)&0x07FF;//11b
     tempIE->GroupID    = (uint8_t)((temp_16b     >>      IE_GROUPID)&0x000F);//4b
     tempIE->Type       = (uint8_t)((temp_16b     >>      IE_TYPE)&0x0001);//1b

     //subIE
     if(i>tempIE->Length)       {return;}
     do{
      temp_16b  = msg->payload[i]+256*msg->payload[i+1];
      i = i + 2;
      tempSubIE.type            = (uint8_t)((temp_16b     >>      SUBIE_TYPE)&0x0001);//1b
      if(tempSubIE.type == SUBIE_TYPE_SHORT)
      {
            tempSubIE.length    = (temp_16b               >>      SUBIE_SHORT_LENGTH)&0x00FF;//8b
            tempSubIE.SubID     = (uint8_t)((temp_16b     >>      SUBIE_SUBID)&0x007F);//7b       
      }
      else
      {
            tempSubIE.length    = (temp_16b               >>      SUBIE_LONG_LENGTH)&0x07FF;//11b
            tempSubIE.SubID     = (uint8_t)((temp_16b     >>      SUBIE_SUBID)&0x000F);//4b   
      }
      switch(tempSubIE.SubID)
      {
      case 0x1a:        //syncIE(subID = 0x1a)
        tempSyncIE = processIE_getSubSyncIE();
        tempSyncIE->length        = tempSubIE.length;
        tempSyncIE->SubID         = tempSubIE.SubID;
        tempSyncIE->type          = tempSubIE.type;
        tempSyncIEcontent = processIE_getSyncIEcontent();
        //length of asn(asn had been stored by ieee802154e)
        i = i+5;
        //store joinPriority
        tempSyncIEcontent->joinPriority     = *((uint8_t*)(msg->payload)+i);
        i++;
        break;

      case 0x1b:        //frameAndLinkIE(subID = 0x1b)
        tempFrameAndLinkIE = processIE_getSubFrameAndLinkIE();
        tempFrameAndLinkIE->length      = tempSubIE.length;
        tempFrameAndLinkIE->SubID       = tempSubIE.SubID;
        tempFrameAndLinkIE->type        = tempSubIE.type;
        tempFrameAndLinkIEcontent = processIE_getFrameAndLinkIEcontent();
        //store number of slotframes
        tempFrameAndLinkIEcontent->numOfSlotframes = *((uint8_t*)(msg->payload)+i);
        i++;
        for(uint8_t j=0;j<tempFrameAndLinkIEcontent->numOfSlotframes;j++)
        {
          //store slotframeID
          tempFrameAndLinkIEcontent->slotframeInfo[j].slotframeID = *((uint8_t*)(msg->payload)+i);
          i++;
          //store length of slotframe
          tempFrameAndLinkIEcontent->slotframeInfo[j].slotframeSize = packetfunctions_ntohs(&(msg->payload[i]));
          i = i + 2;
          //store number of links
          tempFrameAndLinkIEcontent->slotframeInfo[j].numOfLink = *((uint8_t*)(msg->payload)+i);
          i++;
          //sotre links
          tempFrameAndLinkIEcontent->slotframeInfo[i].links = schedule_getLinksList(j);
          Link_t* tempLink =  tempFrameAndLinkIEcontent->slotframeInfo[i].links;
          for(uint8_t k=0;k<tempFrameAndLinkIEcontent->slotframeInfo[j].numOfLink;k++)
          {
            //store slotoffset
            tempLink[k].slotOffset = packetfunctions_ntohs(&(msg->payload[i]));
            i = i + 2;
            //store channeloffset
            tempLink[k].channelOffset = packetfunctions_ntohs(&(msg->payload[i]));
            i  = i + 2;
            //store link type
            tempLink[k].link_type = *((uint8_t*)(msg->payload)+i);
            i++;
          }
        }
        break;
      case 0x40:        //uResLinkTypeIE(subID = 0x40)
        tempuResLinkTypeIE = processIE_getSubuResLinkTypeIE();
        tempuResLinkTypeIE->length      = tempSubIE.length;
        tempuResLinkTypeIE->SubID       = tempSubIE.SubID;
        tempuResLinkTypeIE->type        = tempSubIE.type;
        tempuResLinkTypeIEcontent = processIE_getuResLinkTypeIEcontent();
        //store number of slotframes
        tempuResLinkTypeIEcontent->numOfSlotframes = *((uint8_t*)(msg->payload)+i);
        i++;
        for(uint8_t j=0;j<tempuResLinkTypeIEcontent->numOfSlotframes;j++)
        {
          //store slotframeID
          tempuResLinkTypeIEcontent->slotframeInfo[j].slotframeID = *((uint8_t*)(msg->payload)+i);
          i++;
          //store length of slotframe
          tempuResLinkTypeIEcontent->slotframeInfo[j].slotframeSize = packetfunctions_ntohs(&(msg->payload[i]));
          i = i + 2;
          //store number of links
          tempuResLinkTypeIEcontent->slotframeInfo[j].numOfLink = *((uint8_t*)(msg->payload)+i);
          i++;
          //sotre links
          tempuResLinkTypeIEcontent->slotframeInfo[i].links = schedule_getLinksList(j);
          Link_t* tempLink =  tempuResLinkTypeIEcontent->slotframeInfo[i].links;
          for(uint8_t k=0;k<tempuResLinkTypeIEcontent->slotframeInfo[j].numOfLink;k++)
          {
            //store slotoffset
            tempLink[k].slotOffset = packetfunctions_ntohs(&(msg->payload[i]));
            i = i + 2;
            //store channeloffset
            tempLink[k].channelOffset = packetfunctions_ntohs(&(msg->payload[i]));
            i  = i + 2;
            //store link type
            tempLink[k].link_type = *((uint8_t*)(msg->payload)+i);
            i++;
          }
        }
        break;
      case 0x41:
        tempuResCommandIE = processIE_getSubuResCommandIE();
        tempuResCommandIE->length      = tempSubIE.length;
        tempuResCommandIE->SubID       = tempSubIE.SubID;
        tempuResCommandIE->type        = tempSubIE.type;
        tempuResCommmandIEcontent       = processIE_getuResCommandIEcontent();
        tempuResCommmandIEcontent->uResCommandID                  = *((uint8_t*)(msg->payload)+i);
        i++;
        break;
      case 0x42:
        tempuResBandwidthIE = processIE_getSubuResBandwidthIE();
        tempuResBandwidthIE->length      = tempSubIE.length;
        tempuResBandwidthIE->SubID       = tempSubIE.SubID;
        tempuResBandwidthIE->type        = tempSubIE.type;
        tempuResBandwidthIEcontent    = processIE_getuResBandwidthIEcontent();
        tempuResBandwidthIEcontent->slotframeID                  =  *((uint8_t*)(msg->payload)+i);
        i++;
        tempuResBandwidthIEcontent->numOfLinks                   =  *((uint8_t*)(msg->payload)+i);
        i++;
        break;
      default:
        return;
      }
     } while(i<tempIE->Length);

     packetfunctions_tossHeader(msg, tempIE->Length);

     res_notifRetrieveIEDone(msg);
}
