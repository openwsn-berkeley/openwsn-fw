#include "opendefs.h"
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

port_INLINE void processIE_prependMLMEIE(
      OpenQueueEntry_t* pkt, 
      uint8_t           len
   ){
   payload_IE_ht payload_IE_desc;
   
   // reserve space
   packetfunctions_reserveHeaderSize(
      pkt, 
      sizeof(payload_IE_ht)
   );
   
   // prepare header
   payload_IE_desc.length_groupid_type = 
      len << IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT;
   payload_IE_desc.length_groupid_type |= 
      (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_DESC_TYPE_LONG); 
   
   // copy header
   pkt->payload[0] =  payload_IE_desc.length_groupid_type       & 0xFF;
   pkt->payload[1] = (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
}

//===== prepend IEs

port_INLINE uint8_t processIE_prependSyncIE(OpenQueueEntry_t* pkt){
   mlme_IE_ht mlme_subHeader;
   uint8_t    len;
  
   len = 0;
  
   //=== sync IE
   
   // reserve space
   packetfunctions_reserveHeaderSize(
      pkt,
      sizeof(sync_IE_ht)
   );
   
   // Keep a pointer to where the ASN will be
   // Note: the actual value of the current ASN and JP will be written by the
   //    IEEE802.15.4e when transmitting
   pkt->l2_ASNpayload               = pkt->payload; 
   
   len += sizeof(sync_IE_ht);
  
   //=== MLME IE
  
   // reserve space
   packetfunctions_reserveHeaderSize(
      pkt,
      sizeof(mlme_IE_ht)
   );
   
   // prepare header
   mlme_subHeader.length_subID_type = 
      sizeof(sync_IE_ht) << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
   mlme_subHeader.length_subID_type |= 
      (IEEE802154E_MLME_SYNC_IE_SUBID << IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT)|
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0]= mlme_subHeader.length_subID_type & 0xFF;
   pkt->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;
   
   return len;
}

port_INLINE uint8_t processIE_prependSlotframeLinkIE(OpenQueueEntry_t* pkt){
   mlme_IE_ht mlme_subHeader;
   uint8_t    len;
   uint8_t    linkOption;
   uint16_t   slot;
  
   len        = 0;
   linkOption = 0;
   slot       = SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS+\
                SCHEDULE_MINIMAL_6TISCH_EB_CELLS;
   
   // for each link in the default schedule, add:
   // - [1B] linkOption bitmap
   // - [2B] channel offset
   // - [2B] timeslot
 
   //===== shared cells
   
   linkOption = (1<<FLAG_TX_S)|(1<<FLAG_RX_S)|(1<<FLAG_SHARED_S);
   while(slot>SCHEDULE_MINIMAL_6TISCH_EB_CELLS){
      packetfunctions_reserveHeaderSize(pkt,5);
      pkt->payload[0]   =  slot       & 0xFF;
      pkt->payload[1]   = (slot >> 8) & 0xFF;
      pkt->payload[2]   = 0x00;             // channel offset
      pkt->payload[3]   = 0x00;
      pkt->payload[4]   = linkOption;       // linkOption
      len+=5;
      slot--;
   }
 
   //===== EB cell
   
   linkOption = (1<<FLAG_TX_S)          |
                (1<<FLAG_RX_S)          |
                (1<<FLAG_SHARED_S)      |
                (1<<FLAG_TIMEKEEPING_S);
   packetfunctions_reserveHeaderSize(pkt,5);
   pkt->payload[0] =  SCHEDULE_MINIMAL_6TISCH_EB_CELLS       & 0xFF;
   pkt->payload[1] = (SCHEDULE_MINIMAL_6TISCH_EB_CELLS >> 8) & 0xFF;
   pkt->payload[2] = 0x00; //  channel offset
   pkt->payload[3] = 0x00;
   pkt->payload[4] = linkOption;
   
   len+=5;
   
   //===== slotframe IE header
   
   // - [1B] number of links (6)
   // - [2B] Slotframe Size (101)
   // - [1B] slotframe handle (id)
   packetfunctions_reserveHeaderSize(pkt,5);
   pkt->payload[0] = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER;  
   pkt->payload[1] = SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE;
   pkt->payload[2] =  SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE       & 0xFF;
   pkt->payload[3] = (SCHEDULE_MINIMAL_6TISCH_SLOTFRAME_SIZE >> 8) & 0xFF;
   pkt->payload[4] = 0x06; //number of links
  
   len+=5;
   
   //===== MLME IE header
   // - [1b] 15 short ==0x00
   // - [7b] 8-14 Sub-ID=0x1b
   // - [8b] Length
   
   // reserve space
   packetfunctions_reserveHeaderSize(
      pkt,
      sizeof(mlme_IE_ht)
   );
   
   // prepare header
   mlme_subHeader.length_subID_type = 
      len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
   mlme_subHeader.length_subID_type |= 
      (IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID << 
         IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT)      | 
      IEEE802154E_DESC_TYPE_SHORT;
  
   // copy header
   pkt->payload[0]=  mlme_subHeader.length_subID_type       & 0xFF;
   pkt->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len+=2;
   
   return len;
}

port_INLINE uint8_t processIE_prependOpcodeIE(
      OpenQueueEntry_t* pkt,
      uint8_t           uResCommandID
   ){
   uint8_t    len;
   mlme_IE_ht mlme_subHeader;
  
   len = 0;
   
   //===== command ID
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // write header
   *((uint8_t*)(pkt->payload)) = uResCommandID;
   
   len += 1;  
  
   //===== MLME IE header
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt, sizeof(mlme_IE_ht));
   
   // prepare header
   mlme_subHeader.length_subID_type  = 
      len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
   mlme_subHeader.length_subID_type |= 
      MLME_IE_SUBID_OPCODE << MLME_IE_SUBID_SHIFT|
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0] =  mlme_subHeader.length_subID_type       & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;
  
   return len;
}

port_INLINE uint8_t processIE_prependBandwidthIE(
      OpenQueueEntry_t* pkt, 
      uint8_t           numOfLinks, 
      uint8_t           slotframeID
   ){
   
   uint8_t    len;
   mlme_IE_ht mlme_subHeader;
   
   len = 0;
   
   //===== number of cells
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // write header
   *((uint8_t*)(pkt->payload)) = numOfLinks;
   
   len += 1;
   
   //===== number of cells
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // write header
   *((uint8_t*)(pkt->payload)) = slotframeID;
   
   len += 1;
   
   //===== MLME IE header
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt, sizeof(mlme_IE_ht));
   
   // prepare header
   mlme_subHeader.length_subID_type  = 
      len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
   mlme_subHeader.length_subID_type |= 
      (MLME_IE_SUBID_BANDWIDTH << 
         MLME_IE_SUBID_SHIFT) |
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0] =  mlme_subHeader.length_subID_type       & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;
  
   return len;
}

port_INLINE uint8_t processIE_prependSheduleIE(
      OpenQueueEntry_t* pkt,
      uint8_t           type,
      uint8_t           frameID,
      uint8_t           flag,
      cellInfo_ht*      cellList
   ){
   uint8_t    i;
   uint8_t    len;
   uint8_t    temp8b;
   uint8_t    numOfCells;
   mlme_IE_ht mlme_subHeader;
  
   len        = 0;
   numOfCells = 0;
   
   //===== cell list
   
   for(i=0;i<SCHEDULEIEMAXNUMCELLS;i++) {
      if(cellList[i].linkoptions != CELLTYPE_OFF){
         // cellobjects:
         // - [2B] slotOffset
         // - [2B] channelOffset
         // - [1B] link_type
         packetfunctions_reserveHeaderSize(pkt,5); 
         packetfunctions_htons(cellList[i].tsNum,    &(pkt->payload[0])); 
         packetfunctions_htons(cellList[i].choffset, &(pkt->payload[2]));
         pkt->payload[4] = cellList[i].linkoptions;
         len += 5;
         numOfCells++;
      }
   }
   
   // record the position of cellObjects
   pkt->l2_scheduleIE_numOfCells  = numOfCells;
   pkt->l2_scheduleIE_cellObjects = pkt->payload;
   
   //===== number of cells
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // prepare header
   temp8b  = 0;
   temp8b |= numOfCells << 1;
   temp8b |= flag << 0;
   
   // copy header
   *((uint8_t*)(pkt->payload)) = temp8b;
   
   len += 1;
   
   //===== slotframeID
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // prepare header
   temp8b = frameID;
   
   // copy header
   *((uint8_t*)(pkt->payload)) = temp8b;
   
   len += 1;
  
   // record the frameID 
   pkt->l2_scheduleIE_frameID = frameID;
   
   //===== length
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // prepare header
   temp8b = len;
   
   // copy header
   *((uint8_t*)(pkt->payload)) = temp8b;
   
   // length
   len += 1;
  
   //===== type
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   
   // prepare header
   temp8b = type;
   
   // copy header
   *((uint8_t*)(pkt->payload)) = temp8b;
   
   len += 1;
   
   //===== MLME IE header
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt, sizeof(mlme_IE_ht));
   
   // prepare header
   mlme_subHeader.length_subID_type  = 
      len << IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT;
   mlme_subHeader.length_subID_type |= 
      (MLME_IE_SUBID_SCHEDULE << 
         MLME_IE_SUBID_SHIFT) | 
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0] = mlme_subHeader.length_subID_type & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len+=2;
  
   return len;
}

//===== retrieve IEs

port_INLINE void processIE_retrieveSlotframeLinkIE(
      OpenQueueEntry_t* pkt,
      uint8_t*          ptr
   ){
   uint8_t              numSlotFrames;
   uint8_t              i;
   uint8_t              j;
   uint8_t              localptr;
   slotframeLink_IE_ht  sfInfo; 
   cellInfo_ht          linkInfo;
   
   localptr = *ptr; 
  
   // number of slot frames 1B
   numSlotFrames = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   // for each slotframe
   i=0;
   while(i < numSlotFrames){
      
      // [1B] slotftramehandle 1B
      sfInfo.slotframehandle =*((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      
      // [2B] slotframe size
      sfInfo.slotframesize   = *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      sfInfo.slotframesize  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
      localptr++;;
      
      // [1B] number of links
      sfInfo.numlinks        = *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      
      for (j=0;j<sfInfo.numlinks;j++){
         
         // [2B] TimeSlot
         linkInfo.tsNum = *((uint8_t*)(pkt->payload)+localptr);
         localptr++;
         linkInfo.tsNum  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
         localptr++;
         
         // [2B] Ch.Offset
         linkInfo.choffset = *((uint8_t*)(pkt->payload)+localptr);
         localptr++;
         linkInfo.choffset  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
         localptr++;
         
         // [1B] LinkOption bitmap
         linkInfo.linkoptions = *((uint8_t*)(pkt->payload)+localptr);
         localptr++;
         
         // TODO: inform schedule of that link so it can update if needed.
      } 
      i++;
   }
   
   *ptr=localptr;
} 

port_INLINE void processIE_retrieveOpcodeIE(
      OpenQueueEntry_t* pkt,
      uint8_t*          ptr,
      opcode_IE_ht*     opcodeInfo
   ){
   uint8_t localptr;
   
   localptr=*ptr; 
   
   opcodeInfo->opcode = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
  
   *ptr=localptr; 
} 

port_INLINE void processIE_retrieveBandwidthIE(
      OpenQueueEntry_t* pkt,
      uint8_t *         ptr,
      bandwidth_IE_ht*  bandwidthInfo
   ){
   uint8_t localptr;
   
   localptr=*ptr; 
   
   // [1B] slotframeID
   bandwidthInfo->slotframeID = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   // [1B] number of cells
   bandwidthInfo->numOfLinks = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   *ptr=localptr; 
}

port_INLINE void processIE_retrieveSheduleIE(
      OpenQueueEntry_t* pkt,
      uint8_t*          ptr,
      schedule_IE_ht*   scheduleInfo
   ){
   uint8_t i;
   uint8_t temp8b;
   uint8_t localptr;
  
   localptr=*ptr;
   
   // [1B] type
   scheduleInfo->type             = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   // [1B] length
   scheduleInfo->length           = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   // [1B] frameID
   scheduleInfo->frameID          = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   // [1B] number of cell and flag
   temp8b = *((uint8_t*)(pkt->payload)+localptr);
   scheduleInfo->numberOfcells    = temp8b >> 1;
   scheduleInfo->flag             = temp8b & 0xFE ? TRUE : FALSE;
   localptr++;
  
   if(scheduleInfo->numberOfcells > SCHEDULEIEMAXNUMCELLS) {
      //log error
      return;
   }
   
   for (i=0;i<scheduleInfo->numberOfcells;i++){
      
      // [1B] TimeSlot
      scheduleInfo->cellList[i].tsNum = 
         (*((uint8_t*)(pkt->payload)+localptr))<<8;
      localptr++;
      
      scheduleInfo->cellList[i].tsNum  |= 
         *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      
      // [2B] Ch.Offset
      scheduleInfo->cellList[i].choffset = 
         (*((uint8_t*)(pkt->payload)+localptr))<<8;
      localptr++;
      
      scheduleInfo->cellList[i].choffset  |= 
         *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      
      // [1B] LinkOption bitmap
      scheduleInfo->cellList[i].linkoptions = 
         *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
   }
   *ptr=localptr; 
}
