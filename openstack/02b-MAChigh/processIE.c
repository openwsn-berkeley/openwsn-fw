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
   slotOffset_t      running_slotOffset;
   channelOffset_t   channelOffset;
   frameLength_t     frameLength;

   len            = 0;
   linkOption     = 0;
   
   // for each link in the default schedule, add:
   // - [1B] linkOption bitmap
   // - [2B] channel offset
   // - [2B] timeslot
 
   //===== shared cells
   
   linkOption = (1<<FLAG_TX_S)|(1<<FLAG_RX_S)|(1<<FLAG_SHARED_S)|(1<<FLAG_TIMEKEEPING_S);
   for (running_slotOffset=SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS-1; running_slotOffset < SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS ; running_slotOffset--) {

#ifdef SCHEDULE_SHAREDCELLS_DISTRIBUTED
      slotOffset = running_slotOffset * schedule_getFrameLength() / SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS;      // slot offset
#else
      slotOffset     = running_slotOffset + SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET;
#endif
      channelOffset  = SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET;

/*
      openserial_printCritical(COMPONENT_NEIGHBORS,ERR_GENERIC,
                                    (errorparameter_t)10,
                                    (errorparameter_t)slotOffset);

*/
      //for (slotOffset=lastSlotOffset;slotOffset>SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET;slotOffset--) {
      packetfunctions_reserveHeaderSize(pkt,5);
      pkt->payload[0]   = slotOffset        & 0xFF;
      pkt->payload[1]   = (slotOffset >> 8) & 0xFF;
      pkt->payload[2]   = channelOffset        & 0xFF;
      pkt->payload[3]   = (channelOffset >> 8) & 0xFF;
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

   // reserve space for timeslot template ID
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

/*
port_INLINE uint8_t processIE_prependBandwidthIE(
      OpenQueueEntry_t* pkt, 
      uint8_t           numOfLinks, 
      uint8_t           slotframeID,
      track_t           track
   ){
   

   uint8_t    len;
   mlme_IE_ht mlme_subHeader;
   
   len = 0;
   
   //===== track

   // owner address (64 bits = 8 bytes)
   packetfunctions_reserveHeaderSize(pkt, 8);
   memcpy(pkt->payload, &(track.owner.addr_64b), 8);
   len += 8;

   // instance
   packetfunctions_reserveHeaderSize(pkt, sizeof(uint16_t));
   memcpy(pkt->payload, &(track.instance), sizeof(uint16_t));
   len += sizeof(uint16_t);

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
   mlme_subHeader.length_subID_type  = len;
   mlme_subHeader.length_subID_type |= 
      (MLME_IE_SUBID_BANDWIDTH << 
         MLME_IE_SUBID_SHIFT) |
      IEEE802154E_DESC_TYPE_SHORT;
   
   // copy header
   pkt->payload[0] = mlme_subHeader.length_subID_type        & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8) & 0xFF;
   
   len += 2;

   //record the track info
   memcpy(&(pkt->l2_bandwidthIE_track), &track, sizeof(track_t));

   return len;
}
*/

/*
port_INLINE uint8_t processIE_prependBlacklistIE(
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
   char str[150];

   //===== cell list
   sprintf(str, "LinkRep prepared - blacklist - ");


   for(i=0;i<SCHEDULEIEMAXNUMCELLS;i++) {
      if(cellList[i].linkoptions == CELLTYPE_BUSY){
         // cellobjects:
         // - [2B] slotOffset
         // - [2B] channelOffset
         // - [1B] link_type
         packetfunctions_reserveHeaderSize(pkt,5);
         pkt->payload[0] = (uint8_t)(cellList[i].tsNum  & 0x00FF);
         pkt->payload[1] = (uint8_t)((cellList[i].tsNum & 0xFF00)>>8);
         pkt->payload[2] = (uint8_t)(cellList[i].choffset  & 0x00FF);
         pkt->payload[3] = (uint8_t)((cellList[i].choffset & 0xFF00)>>8);
         pkt->payload[4] = cellList[i].linkoptions;
         len += 5;
         numOfCells++;

         strncat(str, ", slot=", 150);
         openserial_ncat_uint32_t(str, (uint32_t)cellList[i].tsNum, 150);
         strncat(str, "/ch=", 150);
         openserial_ncat_uint32_t(str, (uint32_t)cellList[i].choffset, 150);
         strncat(str, "/status=", 150);
         openserial_ncat_uint32_t(str, (uint32_t)cellList[i].linkoptions, 150);

      }
   }
   if (numOfCells > 0){
      strncat(str, ", NbCellsBusy=", 150);
      openserial_ncat_uint32_t(str, (uint32_t)numOfCells, 150);
      openserial_printf(COMPONENT_SIXTOP, str, strlen(str));

   }

   // record the position of cellObjects
   pkt->l2_scheduleIE_numOfCells  = numOfCells;
   pkt->l2_scheduleIE_cellObjects = pkt->payload;



   //===== number of cells

   // reserve space
   packetfunctions_reserveHeaderSize(pkt,sizeof(uint8_t));

   // prepare header
   temp8b  = numOfCells;
   temp8b |= flag << 7;

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
   mlme_subHeader.length_subID_type  = len;
   mlme_subHeader.length_subID_type |=
      (MLME_IE_SUBID_SCHEDULE << MLME_IE_SUBID_SHIFT) |
      IEEE802154E_DESC_TYPE_SHORT;

   // copy header
   pkt->payload[0] = mlme_subHeader.length_subID_type       & 0xFF;
   pkt->payload[1] = (mlme_subHeader.length_subID_type >> 8)& 0xFF;

   len+=2;

   return len;
}
*/





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

/*port_INLINE uint8_t processIE_prependScheduleIE(
      OpenQueueEntry_t* pkt,
      uint8_t           type,
      uint8_t           frameID,
      uint8_t           flag,
      cellInfo_ht*      cellList
*/
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
               &temp_neighbor,                     // neighbor
               sixtop_get_trackbesteffort()        // with the best effort track (shared cells)
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

