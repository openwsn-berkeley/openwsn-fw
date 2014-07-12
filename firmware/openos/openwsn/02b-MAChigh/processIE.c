#include "openwsn.h"
#include "processIE.h"
#include "sixtop.h"
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

//=========================== public ==========================================

port_INLINE uint8_t processIE_prependSyncIE(OpenQueueEntry_t* pkt){
  MLME_IE_subHeader_t mlme_subHeader;
  uint8_t len = 0;
  //asn + jp 
  packetfunctions_reserveHeaderSize(pkt, sizeof(synch_IE_t));
  pkt->l2_ASNpayload               = pkt->payload; //keep a pointer to where the ASN should be.
  // the actual value of the current ASN and JP will be written by the
  // IEEE802.15.4e when transmitting
  len += sizeof(synch_IE_t);
  
  //subIE header
  packetfunctions_reserveHeaderSize(pkt, sizeof(MLME_IE_subHeader_t));//the MLME header
  //copy mlme sub-header               
  mlme_subHeader.length_subID_type=sizeof(synch_IE_t) << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
  mlme_subHeader.length_subID_type |= (IEEE802154E_MLME_SYNC_IE_SUBID << IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT) | IEEE802154E_DESC_TYPE_SHORT;
  //little endian          
  pkt->payload[0]= mlme_subHeader.length_subID_type & 0xFF;
  pkt->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
  len += 2;

  return len;
}	

port_INLINE uint8_t processIE_prependFrameLinkIE(OpenQueueEntry_t* pkt){
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
    packetfunctions_reserveHeaderSize(pkt,5);
    //ts
    pkt->payload[0]= slot & 0xFF;
    pkt->payload[1]= (slot >> 8) & 0xFF;
    //ch.offset as minimal draft
    pkt->payload[2]= 0x00;
    pkt->payload[3]= 0x00;
    //linkOption
    pkt->payload[4]= linkOption;
    len+=5;
    slot--;
  }
 
  //eb slot
  linkOption = (1<<FLAG_TX_S)|(1<<FLAG_RX_S)|(1<<FLAG_SHARED_S)|(1<<FLAG_TIMEKEEPING_S);
  packetfunctions_reserveHeaderSize(pkt,5);
  len+=5;
 //ts
  pkt->payload[0]= SCHEDULE_MINIMAL_6TISCH_EB_CELLS & 0xFF;
  pkt->payload[1]= (SCHEDULE_MINIMAL_6TISCH_EB_CELLS >> 8) & 0xFF;
  //ch.offset as minimal draft
  pkt->payload[2]= 0x00;
  pkt->payload[3]= 0x00;
 
  pkt->payload[4]= linkOption;
 //now slotframe ie general fields
    //1B number of links == 6 
    //Slotframe Size 2B = 101 timeslots
    //1B slotframe handle (id)
  packetfunctions_reserveHeaderSize(pkt,5);//
  len+=5;
  
  pkt->payload[0]= SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER;  
  pkt->payload[1]= SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
  pkt->payload[2]= SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE & 0xFF;
  pkt->payload[3]= (SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE >> 8) & 0xFF;
  pkt->payload[4]= 0x06; //number of links
  
  //MLME sub IE header 
  //1b -15 short ==0x00
  //7b -8-14 Sub-ID=0x1b
  //8b - Length = 2 mlme-header + 5 slotframe general header +(6links*5bytes each) 
  packetfunctions_reserveHeaderSize(pkt, sizeof(MLME_IE_subHeader_t));//the MLME header
   
   
   //copy mlme sub-header               
  mlme_subHeader.length_subID_type = len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
  mlme_subHeader.length_subID_type |= (IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID << IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT) | IEEE802154E_DESC_TYPE_SHORT;
  
  //little endian          
  pkt->payload[0]= mlme_subHeader.length_subID_type & 0xFF;
  pkt->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
  len+=2;//count len of mlme header
   
  return len;
}

port_INLINE uint8_t processIE_prependTimeslotIE(OpenQueueEntry_t* pkt){
  //tengfei-todo
  return 0;
}

port_INLINE uint8_t processIE_prependChannelHoppingIE(OpenQueueEntry_t* pkt){
  //tengfei-todo
  return 0;
}

port_INLINE uint8_t processIE_prependSixtopLinkTypeIE(OpenQueueEntry_t* pkt){
  //tengfei-todo
  return 0;
}

port_INLINE uint8_t processIE_prependSixtopOpcodeIE(OpenQueueEntry_t* pkt,uint8_t uResCommandID){
  uint8_t len = 0;
  MLME_IE_subHeader_t mlme_subHeader;
  
  //OpcodeID
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = uResCommandID;
  len += 1;  
  
  //subIE
  mlme_subHeader.length_subID_type  = len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
  mlme_subHeader.length_subID_type |= SIXTOP_MLME_RES_OPCODE_IE_SUBID << SIXTOP_MLME_RES_OPCODE_IE_SUBID_SHIFT | IEEE802154E_DESC_TYPE_SHORT;
  packetfunctions_reserveHeaderSize(pkt, sizeof(MLME_IE_subHeader_t));//the MLME header
  //little endian          
  pkt->payload[0] = mlme_subHeader.length_subID_type & 0xFF;
  pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
  len += 2;
  
  return len;
}

port_INLINE uint8_t processIE_prependSixtopBandwidthIE(OpenQueueEntry_t* pkt, uint8_t numOfLinks, uint8_t slotframeID){
  uint8_t len = 0;
  MLME_IE_subHeader_t mlme_subHeader;
  
  //numcell
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = numOfLinks;
  len += 1;
  
  //slotframeID
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = slotframeID;
  len += 1;
  
  //subIE header
  mlme_subHeader.length_subID_type  = len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
  mlme_subHeader.length_subID_type |= SIXTOP_MLME_RES_BANDWIDTH_IE_SUBID << SIXTOP_MLME_RES_BANDWIDTH_IE_SUBID_SHIFT | IEEE802154E_DESC_TYPE_SHORT;
  packetfunctions_reserveHeaderSize(pkt, sizeof(MLME_IE_subHeader_t));//the MLME header
  //little endian          
  pkt->payload[0] = mlme_subHeader.length_subID_type & 0xFF;
  pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
  len += 2;
  
  return len;
}

// add ScheduleIE to packet, using TLV structure (type-length-value)
port_INLINE uint8_t processIE_prependSixtopGeneralSheduleIE(OpenQueueEntry_t* pkt,uint8_t type,uint8_t frameID,uint8_t flag,sixtop_linkInfo_subIE_t* linklist){
  uint8_t i,len = 0;
  uint8_t temp8b;
  uint8_t numOfCells = 0;
  MLME_IE_subHeader_t mlme_subHeader;
  for(i=0;i<MAXSCHEDULEDCELLS;i++) {
    if(linklist[i].linkoptions != CELLTYPE_OFF){
      // cellobjects
      packetfunctions_reserveHeaderSize(pkt,5); // 2 bytes slotOffset, 2bytes channelOffset+ 1 byte link_type
      packetfunctions_htons(linklist[i].tsNum, &(pkt->payload[0])); 
      packetfunctions_htons(linklist[i].choffset, &(pkt->payload[2]));
      pkt->payload[4] = linklist[i].linkoptions;
      len += 5;
      numOfCells++;
    }
  }
  // record the position of cellObjects
  pkt->l2_scheduleIE_numOfCells  = numOfCells;
  pkt->l2_scheduleIE_cellObjects = pkt->payload;
  
  // numcell + F, 1B
  temp8b  = 0;
  temp8b |= numOfCells << 1;
  temp8b |= flag << 0;
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = temp8b;
  len += 1;
  
  // slotframeID
  temp8b = frameID;
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = temp8b;
  len += 1;
  
  //record the frameID (use for removing the cell when removelink request commad was transimitted successufully)
  pkt->l2_scheduleIE_frameID = frameID;
  
  // length
  temp8b = len;
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = temp8b;
  len += 1;
  
  // type
  temp8b = type;
  packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
  *((uint8_t*)(pkt->payload)) = temp8b;
  len += 1;
  
  //subIE header
  mlme_subHeader.length_subID_type  = len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
  mlme_subHeader.length_subID_type |= SIXTOP_MLME_RES_GENERAL_SCHEDULE_IE_SUBID << SIXTOP_MLME_RES_GENERAL_SCHEDULE_IE_SUBID_SHIFT | IEEE802154E_DESC_TYPE_SHORT;
  packetfunctions_reserveHeaderSize(pkt, sizeof(MLME_IE_subHeader_t));//the MLME header
  //little endian          
  pkt->payload[0] = mlme_subHeader.length_subID_type & 0xFF;
  pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
  len+=2;
  
  return len;
}

//===================== process sub IE============================= 

port_INLINE void processIE_retrieveSyncIEcontent(OpenQueueEntry_t* pkt,uint8_t * ptr){
}

port_INLINE void processIE_retrieveSlotframeLinkIE(OpenQueueEntry_t* pkt,uint8_t * ptr){
  uint8_t numSlotFrames,i,j,localptr;
  sixtop_slotframelink_subIE_t sfInfo; 
  sixtop_linkInfo_subIE_t linkInfo;
  localptr=*ptr; 
  // number of slot frames 1B
  numSlotFrames = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  // for each slotframe
  i=0;
  while(i < numSlotFrames){
   //1-slotftramehandle 1B
    sfInfo.slotframehandle=*((uint8_t*)(pkt->payload)+localptr);
    localptr++;
    //2-slotframe size 2B
    sfInfo.slotframesize = *((uint8_t*)(pkt->payload)+localptr);
    localptr++;
    sfInfo.slotframesize |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
    localptr++;;
    //3-number of links 1B   
    sfInfo.numlinks= *((uint8_t*)(pkt->payload)+localptr);
    localptr++;
   
    for (j=0;j<sfInfo.numlinks;j++){
      //for each link 5Bytes
       //TimeSlot 2B
       linkInfo.tsNum = *((uint8_t*)(pkt->payload)+localptr);
       localptr++;
       linkInfo.tsNum  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
       localptr++;
       //Ch.Offset 2B
       linkInfo.choffset = *((uint8_t*)(pkt->payload)+localptr);
       localptr++;
       linkInfo.choffset  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
       localptr++;
       //LinkOption bitmap 1B
       linkInfo.linkoptions = *((uint8_t*)(pkt->payload)+localptr);
       localptr++;
       //xv poipoi
       //TODO - inform schedule of that link so it can update if needed.
    } 
    i++;
  } 
  *ptr=localptr;     
} 

port_INLINE void processIE_retrieveChannelHoppingIE(OpenQueueEntry_t* pkt,uint8_t * ptr){
} 

port_INLINE void processIE_retrieveTimeslotIE(OpenQueueEntry_t* pkt,uint8_t * ptr){
}

port_INLINE void processIE_retrieveSixtopLinkTypeIE(OpenQueueEntry_t* pkt,uint8_t * ptr){
}

port_INLINE void processIE_retrieveSixtopOpcodeIE(OpenQueueEntry_t* pkt,uint8_t * ptr,sixtop_opcode_subIE_t* opcodeInfo){
  uint8_t localptr;
  localptr=*ptr; 
  // OpCode 1B
  opcodeInfo->opcode = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  
  *ptr=localptr; 
} 

port_INLINE void processIE_retrieveSixtopBandwidthIE(OpenQueueEntry_t* pkt,uint8_t * ptr,sixtop_bandwidth_subIE_t* bandwidthInfo){
  uint8_t localptr;
  localptr=*ptr; 
  // slotframeID 1B
  bandwidthInfo->slotframeID = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  // number of cells 1B
  bandwidthInfo->numOfLinks = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  *ptr=localptr; 
}

port_INLINE void processIE_retrieveSixtopGeneralSheduleIE(OpenQueueEntry_t* pkt,uint8_t * ptr,sixtop_generalschedule_subIE_t* scheduleInfo){
  uint8_t i,temp8b,localptr;
  localptr=*ptr; 
  //type 1B
  scheduleInfo->type = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  //length 1B
  scheduleInfo->length = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  //frameID 1B
  scheduleInfo->frameID = *((uint8_t*)(pkt->payload)+localptr);
  localptr++;
  //number of cell and flag 1B
  temp8b = *((uint8_t*)(pkt->payload)+localptr);
  scheduleInfo->numberOfcells = temp8b >> 1;
  scheduleInfo->flag = temp8b & 0xFE ? TRUE : FALSE;
  localptr++;
  
  if(scheduleInfo->length > MAXSCHEDULEDCELLS) {
    //log error
    return;
  }
  
  for (i=0;i<scheduleInfo->length;i++){
    //for each cell 5Bytes
     //TimeSlot 2B
     scheduleInfo->linklist[i].tsNum = *((uint8_t*)(pkt->payload)+localptr);
     localptr++;
     scheduleInfo->linklist[i].tsNum  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
     localptr++;
     //Ch.Offset 2B
     scheduleInfo->linklist[i].choffset = *((uint8_t*)(pkt->payload)+localptr);
     localptr++;
     scheduleInfo->linklist[i].choffset  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
     localptr++;
     //LinkOption bitmap 1B
     scheduleInfo->linklist[i].linkoptions = *((uint8_t*)(pkt->payload)+localptr);
     localptr++;
  }
  *ptr=localptr; 
}
