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
   payload_IE_desc.length_groupid_type = len;
   payload_IE_desc.length_groupid_type |= 
      (IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME  | IEEE802154E_PAYLOAD_DESC_TYPE_MLME); 
   
   // copy header
   pkt->payload[0] = payload_IE_desc.length_groupid_type        & 0xFF;
   pkt->payload[1] = (payload_IE_desc.length_groupid_type >> 8) & 0xFF;
}

port_INLINE void processIE_prepend_sixtopIE(
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
   payload_IE_desc.length_groupid_type = len;
   payload_IE_desc.length_groupid_type |= 
      (IANA_6TOP_IE_GROUP_ID  | IANA_6TOP_IE_GROUP_ID_TYPE); 
   
   // copy header
   pkt->payload[0] = payload_IE_desc.length_groupid_type        & 0xFF;
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
   mlme_subHeader.length_subID_type = sizeof(sync_IE_ht);
   mlme_subHeader.length_subID_type |= 
      (IEEE802154E_MLME_SYNC_IE_SUBID << IEEE802154E_MLME_SYNC_IE_SUBID_SHIFT)|
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0]= mlme_subHeader.length_subID_type        & 0xFF;
   pkt->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;
   
   return len;
}

port_INLINE uint8_t processIE_prependSlotframeLinkIE(OpenQueueEntry_t* pkt){
   mlme_IE_ht mlme_subHeader;
   uint8_t           len;
   uint8_t           linkOption;
   slotOffset_t      slotOffset;
   slotOffset_t      lastSlotOffset;
   frameLength_t     frameLength;
  
   len            = 0;
   linkOption     = 0;
   lastSlotOffset = SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET + SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS;
   
   // for each link in the default schedule, add:
   // - [1B] linkOption bitmap
   // - [2B] channel offset
   // - [2B] timeslot
 
   //===== shared cells
   
   linkOption = (1<<FLAG_TX_S)|(1<<FLAG_RX_S)|(1<<FLAG_SHARED_S)|(1<<FLAG_TIMEKEEPING_S);
   for (slotOffset=lastSlotOffset;slotOffset>SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET;slotOffset--) {
      packetfunctions_reserveHeaderSize(pkt,5);
      pkt->payload[0]   = (slotOffset-1)        & 0xFF;
      pkt->payload[1]   = ((slotOffset-1) >> 8) & 0xFF;
      pkt->payload[2]   = SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET;     // channel offset
      pkt->payload[3]   = 0x00;
      pkt->payload[4]   = linkOption;                          // linkOption
      len+=5;
   }
   
   //===== slotframe IE header
   
   // - [1B] number of links (6)
   // - [2B] Slotframe Size (101)
   // - [1B] slotframe handle (id)
   frameLength = schedule_getFrameLength();
   packetfunctions_reserveHeaderSize(pkt,5);
   pkt->payload[0] = schedule_getFrameNumber();  
   pkt->payload[1] = schedule_getFrameHandle();
   pkt->payload[2] =  frameLength       & 0xFF;
   pkt->payload[3] = (frameLength >> 8) & 0xFF;
   pkt->payload[4] = SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS; //number of links
  
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
   mlme_subHeader.length_subID_type = len;
   mlme_subHeader.length_subID_type |= 
      (IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID << 
         IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID_SHIFT) |
      IEEE802154E_DESC_TYPE_SHORT;
  
   // copy header
   pkt->payload[0]= mlme_subHeader.length_subID_type        & 0xFF;
   pkt->payload[1]= (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len+=2;
   
   return len;
}

port_INLINE uint8_t processIE_prependTSCHTimeslotIE(OpenQueueEntry_t* pkt){
   uint8_t    len;
   mlme_IE_ht mlme_subHeader;
   
   uint16_t    duration;
   
   len = 0;
   duration = ieee154e_getSlotDuration();
   
   if (duration==328){
       // reserve space for timeslot template ID
       packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
       // write header
       *((uint8_t*)(pkt->payload)) = TIMESLOT_TEMPLATE_ID;
       len+=1;
   } else {
       // reserve space for timeslot template ID
       packetfunctions_reserveHeaderSize(pkt,sizeof(uint16_t));
       // write header
       pkt->payload[0] = (uint8_t)(duration & 0x00ff);
       pkt->payload[1] = (uint8_t)((duration>>8) & 0x00ff);
       // reserve space for timeslot template ID
       packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
       // write header
       *((uint8_t*)(pkt->payload)) = 1;
       len+=3;
   }
   
   //===== MLME IE header
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt, sizeof(mlme_IE_ht));
   
   // prepare header
   mlme_subHeader.length_subID_type  = len;
   mlme_subHeader.length_subID_type |= 
      IEEE802154E_MLME_TIMESLOT_IE_SUBID << IEEE802154E_MLME_TIMESLOT_IE_SUBID_SHIFT|
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0] = mlme_subHeader.length_subID_type        & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;
  
   return len;
}
port_INLINE uint8_t processIE_prependChannelHoppingIE(OpenQueueEntry_t* pkt){
   uint8_t    len;
   mlme_IE_ht mlme_subHeader;
   
   len = 0;

   // reserve space for channelhopping template ID
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
   // write header
   *((uint8_t*)(pkt->payload)) = CHANNELHOPPING_TEMPLATE_ID;
   
   len+=1;
   
   //===== MLME IE header
   
   // reserve space
   packetfunctions_reserveHeaderSize(pkt, sizeof(mlme_IE_ht));
   
   // prepare header
   mlme_subHeader.length_subID_type  = len;
   mlme_subHeader.length_subID_type |= 
      IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID << IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID_SHIFT|
      IEEE802154E_DESC_TYPE_LONG;
   
   // copy header
   pkt->payload[0] = mlme_subHeader.length_subID_type        & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;
  
   return len;
}

port_INLINE uint8_t processIE_prepend_sixSubIEHeader(
    OpenQueueEntry_t*    pkt,
    uint8_t len
  ){
    mlme_IE_ht mlme_subHeader;
    //===== MLME IE header
    // reserve space
    packetfunctions_reserveHeaderSize(pkt, sizeof(mlme_IE_ht));
   
    // prepare header
    mlme_subHeader.length_subID_type  = len;
    mlme_subHeader.length_subID_type |= 
        (IANA_6TOP_SUBIE_ID << MLME_IE_SUBID_SHIFT) | 
        IEEE802154E_DESC_TYPE_SHORT;
   
    // copy header
    pkt->payload[0] = mlme_subHeader.length_subID_type       & 0xFF;
    pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8)& 0xFF;
    
    return 2;
}

port_INLINE uint8_t processIE_prepend_sixGeneralMessage(
    OpenQueueEntry_t*    pkt,
    uint8_t code
    ){
    uint8_t    len = 0;
    uint8_t    temp8b;
   
    //===== SFID
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    *((uint8_t*)(pkt->payload)) = SFID_SF0;
    len += 1;
    
    pkt->l2_sixtop_returnCode = code;
  
    //===== version & code
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    temp8b  = IANA_6TOP_6P_VERSION;
    temp8b |= code << 4;
    *((uint8_t*)(pkt->payload)) = temp8b;
    len += 1;
   
    return len;
}

port_INLINE uint8_t processIE_prepend_sixSubID(OpenQueueEntry_t*    pkt){
    uint8_t    len = 0;
   
    //===== SFID
    packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));
    *((uint8_t*)(pkt->payload)) = IANA_6TOP_SUBIE_ID;
    len += 1;
    
    return len;
}

port_INLINE uint8_t processIE_prepend_sixCelllist(
    OpenQueueEntry_t*    pkt,
    cellInfo_ht*         cellList
   ){
    uint8_t    i;
    uint8_t    len;
    uint8_t    numOfCells;
  
    len        = 0;
    numOfCells = 0;
   
    //===== cell list
   
    for(i=0;i<SCHEDULEIEMAXNUMCELLS;i++) {
        if(cellList[i].linkoptions != CELLTYPE_OFF){
            // cellobjects:
            // - [2B] slotOffset
            // - [2B] channelOffset
            // - [1B] link_type
            packetfunctions_reserveHeaderSize(pkt,4); 
            pkt->payload[0] = (uint8_t)(cellList[i].tsNum  & 0x00FF);
            pkt->payload[1] = (uint8_t)((cellList[i].tsNum & 0xFF00)>>8);
            pkt->payload[2] = (uint8_t)(cellList[i].choffset  & 0x00FF);
            pkt->payload[3] = (uint8_t)((cellList[i].choffset & 0xFF00)>>8);
            len += 4;
            numOfCells++;
        }
    }
   
    // record the position of cellObjects
    pkt->l2_sixtop_numOfCells  = numOfCells;
    pkt->l2_sixtop_cellObjects = pkt->payload;
   
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
   open_addr_t          temp_neighbor;
   frameLength_t        oldFrameLength;
   
   localptr = *ptr; 
  
   // number of slot frames 1B
   numSlotFrames = *((uint8_t*)(pkt->payload)+localptr);
   localptr++;
   
   schedule_setFrameNumber(numSlotFrames);

   // for each slotframe
   i=0;
   while(i < numSlotFrames){
      
      // [1B] slotftramehandle 1B
      sfInfo.slotframehandle =*((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      
      schedule_setFrameHandle(sfInfo.slotframehandle);
      
      // [2B] slotframe size
      sfInfo.slotframesize   = *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      sfInfo.slotframesize  |= (*((uint8_t*)(pkt->payload)+localptr))<<8;
      localptr++;;
      
      oldFrameLength = schedule_getFrameLength();
      schedule_setFrameLength(sfInfo.slotframesize);
      
      // [1B] number of links
      sfInfo.numlinks        = *((uint8_t*)(pkt->payload)+localptr);
      localptr++;
      
      if (oldFrameLength == 0) {
         
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
            
            // shared TXRX anycast slot(s)
            memset(&temp_neighbor,0,sizeof(temp_neighbor));
            temp_neighbor.type             = ADDR_ANYCAST;
            schedule_addActiveSlot(
               linkInfo.tsNum,                     // slot offset
               CELLTYPE_TXRX,                      // type of slot
               TRUE,                               // shared?
               linkInfo.choffset,                  // channel offset
               &temp_neighbor                      // neighbor
            );
         }
      }
      i++;
      break; //TODO: this break is put since a single slotframe is managed
   }
   
   *ptr=localptr;
} 

port_INLINE void processIE_retrieve_sixCelllist(
    OpenQueueEntry_t*   pkt,
    uint8_t             ptr,
    uint8_t             length,
    cellInfo_ht*        cellList
    ){
    uint8_t i=0;
    uint8_t localptr = ptr;
    uint8_t len = length;
    while(len>0){
        cellList[i].tsNum     = *((uint8_t*)(pkt->payload)+localptr);
        cellList[i].tsNum    |= (*((uint8_t*)(pkt->payload)+localptr+1))<<8;
        cellList[i].choffset  = *((uint8_t*)(pkt->payload)+localptr+2);
        cellList[i].choffset |= (*((uint8_t*)(pkt->payload)+localptr+3))<<8;
        localptr        += 4;
        len             -= 4;
        // mark with linkoptions as ocuppied
        cellList[i].linkoptions = CELLTYPE_TX;
        i++;
    }
}

port_INLINE bool processIE_EB_IE(OpenQueueEntry_t* pkt, uint16_t* lenIE) {
   uint8_t               ptr;
   uint8_t               temp_8b;
   uint8_t               gr_elem_id;
   uint8_t               subid;
   uint16_t              temp_16b;
   uint16_t              len;
   uint16_t              sublen;
   // flag used for understanding if the slotoffset should be inferred from both ASN and slotframe length
   bool                  f_asn2slotoffset;
   
   ptr=0;
   
   // payload IE header, header IE is processed before when retrieve header  
   
   //candidate IE header  if type ==0 header IE if type==1 payload IE
   temp_8b    = *((uint8_t*)(pkt->payload)+ptr);
   ptr++;
   
   temp_16b   = temp_8b + ((*((uint8_t*)(pkt->payload)+ptr)) << 8);
   ptr++;
   
   *lenIE     = ptr;
   
   if ((temp_16b & IEEE802154E_DESC_TYPE_PAYLOAD_IE) == IEEE802154E_DESC_TYPE_PAYLOAD_IE){
      // payload IE
      
      len          = temp_16b & IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK;
      gr_elem_id   = (temp_16b & IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK)>>IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT;
   } else {
      // header IE
      
      len          = temp_16b & IEEE802154E_DESC_LEN_HEADER_IE_MASK;
      gr_elem_id   = (temp_16b & IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK)>>IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT; 
   }
   
   *lenIE         += len;
   
   //===== sub-elements
   
   switch(gr_elem_id){
      
      case IEEE802154E_MLME_IE_GROUPID:
         // MLME IE
         f_asn2slotoffset = FALSE;
         do {
            
            //read sub IE header
            temp_8b     = *((uint8_t*)(pkt->payload)+ptr);
            ptr         = ptr + 1;
            temp_16b    = temp_8b  + ((*((uint8_t*)(pkt->payload)+ptr))<<8);
            ptr         = ptr + 1;
            
            len         = len - 2; //remove header fields len
            
            if ((temp_16b & IEEE802154E_DESC_TYPE_LONG) == IEEE802154E_DESC_TYPE_LONG){
               // long sub-IE
               
               sublen   = temp_16b & IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK;
               subid    = (temp_16b & IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT; 
            } else {
               // short sub-IE
               
               sublen   = temp_16b & IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK;
               subid    = (temp_16b & IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK)>>IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT; 
            }
            
            switch(subid){
               
               case IEEE802154E_MLME_SYNC_IE_SUBID:
                  // Sync IE: ASN and Join Priority 
                  
                  if (idmanager_getIsDAGroot()==FALSE) {
                     // ASN
                     ieee154e_asnStoreFromEB((uint8_t*)(pkt->payload)+ptr);
                     // ASN is known, but the frame length is not
                     // frame length will be known after parsing the frame and link IE
                     f_asn2slotoffset = TRUE;
                     ptr = ptr + 5;
                     // join priority
                     ieee154e_joinPriorityStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                     ptr = ptr + 1;
                  }
                  break;
               
               case IEEE802154E_MLME_SLOTFRAME_LINK_IE_SUBID:
                  if ((idmanager_getIsDAGroot()==FALSE) && (ieee154e_isSynch()==FALSE)) {
                     processIE_retrieveSlotframeLinkIE(pkt,&ptr);
                  }
                  break;
               
               case IEEE802154E_MLME_TIMESLOT_IE_SUBID:
                  if (idmanager_getIsDAGroot()==FALSE) {
                      // timeslot template ID
                      ieee154e_timeslotTemplateIDStoreFromEB((uint8_t*)(pkt->payload),&ptr);
                  }
                  break;
                  
               case IEEE802154E_MLME_CHANNELHOPPING_IE_SUBID:
                  if (idmanager_getIsDAGroot()==FALSE) {
                      // channelhopping template ID
                      ieee154e_channelhoppingTemplateIDStoreFromEB(*((uint8_t*)(pkt->payload)+ptr));
                      ptr = ptr + 1;
                  }
                  break;
               default:
                  return FALSE;
                  break;
            }
            
            len = len - sublen;
         } while(len>0);
         if (f_asn2slotoffset == TRUE) {
            // at this point, ASN and frame length are known
            // the current slotoffset can be inferred
            ieee154e_syncSlotOffset();
         }
         break;
         
      default:
         *lenIE = 0; //no header or not recognized.
         return FALSE;
   }
   
   if(*lenIE>127) {
      // log the error
      openserial_printError(
         COMPONENT_IEEE802154E,
         ERR_HEADER_TOO_LONG,
         (errorparameter_t)*lenIE,
         (errorparameter_t)1
      );
   }
   return TRUE;
}