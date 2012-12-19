#include "processIE.h"
#include "reservation.h"
#include "openwsn.h"
#include "res.h"
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
    frameAndLinkIEcontent_vars.slotframeInfo[i].slotframeSize             = schedule_getSlotframeSize(i);
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
    uResLinkTypeIEcontent_vars.slotframeInfo[i].slotframeSize             = schedule_getSlotframeSize(i);
    length += 2;
    uResLinkTypeIEcontent_vars.slotframeInfo[i].numOfLink                 = schedule_getLinksNumber(i);
    length += 1;
    uResLinkTypeIEcontent_vars.slotframeInfo[i].links                     = schedule_getLinksList(i);
    length += 5 * uResLinkTypeIEcontent_vars.slotframeInfo[i].numOfLink;
  }
  uResLinkTypeIE_vars.length = length;
}

void processIE_setSubuResCommandIE(){
  //set subIE length,subID and type fields
  uint8_t length = 0;
  uResCommandIE_vars.SubID = 0x41;
  uResCommandIE_vars.type  = 0;
  length = length + 2;
  // set uRes Command ID
  uResCommandIEcontent_vars.uResCommandID = reservation_getuResCommandID();
  length = length + 1;
  uResCommandIE_vars.length = length;
}

void processIE_setSubuResBandwidthIE(){
  //set subIE length,subID and type fields
  uint8_t length = 0;
  uResBandwidthIE_vars.SubID = 0x42;
  uResBandwidthIE_vars.type  = 0;
  length = length + 2;

  bandwidth_vars_t tempBandwidth = reservation_getuResBandwidth();
  uResBandwidthIEcontent_vars.numOfLinks = tempBandwidth.numOfLinks;
  length = length + 1;
  uResBandwidthIEcontent_vars.slotframeID = tempBandwidth.slotframeID;
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
