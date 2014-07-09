#include "openwsn.h"
#include "schedule.h"
#include "openserial.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "6top.h"

//=========================== variables =======================================

schedule_vars_t schedule_vars;
schedule_dbg_t schedule_dbg;
Link_t links[MAXACTIVESLOTS];

//=========================== prototypes ======================================

void schedule_resetEntry(scheduleEntry_t* pScheduleEntry);
bool schedule_checkExistSchedule(uint16_t slotOffset);

//=========================== private ========================================

bool isOneAvailableLink(Link_t tempLink);
bool isOneRequestedLink(Link_t tempLink);

//=========================== public ==========================================

//=== admin

void schedule_init() {
   uint8_t         i;
   slotOffset_t    running_slotOffset;
   open_addr_t     temp_neighbor;

   // reset local variables
   memset(&schedule_vars,0,sizeof(schedule_vars_t));
   for (i=0;i<MAXACTIVESLOTS;i++) {
      schedule_resetEntry(&schedule_vars.scheduleBuf[i]);
   }
   schedule_vars.backoffExponent = MINBE-1;
   memset(&schedule_dbg, 0,sizeof(schedule_dbg_t));

   // set frame length
   schedule_setFrameLength(SUPERFRAME_LENGTH);
   
   // start at slot 0
   running_slotOffset = 0;
   
   // advertisement slot(s)
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   for (i=0;i<NUMADVSLOTS;i++) {
      schedule_addActiveSlot(
         running_slotOffset,      // slot offset
         CELLTYPE_ADV,            // type of slot
         FALSE,                   // shared?
         0,                       // channel offset
         &temp_neighbor,           // neighbor
         FALSE                     //no update but insert
      );
      running_slotOffset++;
   } 
   
   // shared TXRX anycast slot(s)
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   temp_neighbor.type             = ADDR_ANYCAST;
   for (i=0;i<NUMSHAREDTXRX;i++) {
      schedule_addActiveSlot(
         running_slotOffset,      // slot offset
         CELLTYPE_TXRX,           // type of slot
         TRUE,                    // shared?
         0,                       // channel offset
         &temp_neighbor,          // neighbor
         FALSE                    //no update but insert
      );
      running_slotOffset++;
   }
   
   // serial RX slot(s)
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   schedule_addActiveSlot(
      running_slotOffset,         // slot offset
      CELLTYPE_SERIALRX,          // type of slot
      FALSE,                      // shared?
      0,                          // channel offset
      &temp_neighbor,             // neighbor
      FALSE                       //no update but insert
   );
   running_slotOffset++;
   /*
   for (i=0;i<NUMSERIALRX-1;i++) {
      schedule_addActiveSlot(
         running_slotOffset,      // slot offset
         CELLTYPE_MORESERIALRX,   // type of slot
         FALSE,                   // shared?
         0,                       // channel offset
         &temp_neighbor           // neighbor
      );
      running_slotOffset++;
   }
   */
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_schedule() {
   debugScheduleEntry_t temp;
   
   schedule_vars.debugPrintRow         = (schedule_vars.debugPrintRow+1)%MAXACTIVESLOTS;
   
   temp.row                            = schedule_vars.debugPrintRow;
   temp.slotOffset                     = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].slotOffset;
   temp.type                           = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].type;
   temp.shared                         = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].shared;
   temp.channelOffset                  = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].channelOffset;
   memcpy(
      &temp.neighbor,
      &schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].neighbor,
      sizeof(open_addr_t)
   );
   temp.numRx                          = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].numRx;
   temp.numTx                          = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].numTx;
   temp.numTxACK                       = \
      schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].numTxACK;
   memcpy(
      &temp.lastUsedAsn,
      &schedule_vars.scheduleBuf[schedule_vars.debugPrintRow].lastUsedAsn,
      sizeof(asn_t)
   );
   
   openserial_printStatus(STATUS_SCHEDULE,
         (uint8_t*)&temp,
         sizeof(debugScheduleEntry_t)
   );
   
   return TRUE;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_backoff() {
   uint8_t temp[2];
   temp[0] = schedule_vars.backoffExponent;
   temp[1] = schedule_vars.backoff;
   openserial_printStatus(STATUS_BACKOFF,
         (uint8_t*)&temp,
         sizeof(temp));
   return TRUE;
}

//=== from uRES (writing the schedule)

/**
\brief Set frame length.

\param newFrameLength The new frame length.
*/
void schedule_setFrameLength(frameLength_t newFrameLength) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   schedule_vars.frameLength = newFrameLength;
   ENABLE_INTERRUPTS();
}

/**
\brief get the information of a spcific slot.

\param slotOffset
\param neighbor
\param info
*/
void  schedule_getSlotInfo(
   slotOffset_t         slotOffset,
   open_addr_t*         neighbor,
   slotinfo_element_t*  info
){
                           
   scheduleEntry_t* slotContainer;
  
   // find an empty schedule entry container
   slotContainer = &schedule_vars.scheduleBuf[0];
   while (slotContainer->type!=CELLTYPE_OFF && slotContainer<=&schedule_vars.scheduleBuf[MAXACTIVESLOTS-1]) {
       //check that this entry for that neighbour and timeslot is not already scheduled.
       if (packetfunctions_sameAddress(neighbor,&(slotContainer->neighbor))&& (slotContainer->slotOffset==slotOffset)){
               //it exists so this is an update.
               info->link_type                 = slotContainer->type;
               info->shared                    =slotContainer->shared;
               info->channelOffset             = slotContainer->channelOffset;
               return; //as this is an update. No need to re-insert as it is in the same position on the list.
        }
        slotContainer++;
   }
   //return cell type off.
   info->link_type                 = CELLTYPE_OFF;
   info->shared                    = FALSE;
   info->channelOffset             = 0;//set to zero if not set.                          
}

/**
\brief Add a new active slot into the schedule.

If udpate param is set then update it in case it exists.

\param slotOffset
\param type
\param shared
\param channelOffset
\param neighbor
\param isUpdate
*/
owerror_t schedule_addActiveSlot(
      slotOffset_t    slotOffset,
      cellType_t      type,
      bool            shared,
      channelOffset_t channelOffset,
      open_addr_t*    neighbor,
      bool            isUpdate
   ) {
   
   owerror_t outcome;
   
   scheduleEntry_t* slotContainer;
   scheduleEntry_t* previousSlotWalker;
   scheduleEntry_t* nextSlotWalker;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   
   // find an empty schedule entry container
   slotContainer = &schedule_vars.scheduleBuf[0];
   while (slotContainer->type!=CELLTYPE_OFF &&
         slotContainer<=&schedule_vars.scheduleBuf[MAXACTIVESLOTS-1]) {
  
           //check that this entry for that neighbour and timeslot is not already scheduled.
           if (type!=CELLTYPE_SERIALRX && type!=CELLTYPE_MORESERIALRX &&  
               (packetfunctions_sameAddress(neighbor,&(slotContainer->neighbor))||
                 (slotContainer->neighbor.type==ADDR_ANYCAST && isUpdate==TRUE))
                 &&(slotContainer->slotOffset==slotOffset)){
               //it exists so this is an update.
               slotContainer->type                      = type;
               slotContainer->shared                    = shared;
               slotContainer->channelOffset             = channelOffset;
               memcpy(&slotContainer->neighbor,neighbor,sizeof(open_addr_t));//update the address too!
               schedule_dbg.numUpdatedSlotsCur++;
               ENABLE_INTERRUPTS();
               return E_SUCCESS; //as this is an update. No need to re-insert as it is in the same position on the list.
           }
           
           slotContainer++;
   }
   
   if (isUpdate==TRUE) {
     //we are trying to update an item that is not in the schedule list.
     ENABLE_INTERRUPTS();
     return E_FAIL;
   }
   if (slotContainer>&schedule_vars.scheduleBuf[MAXACTIVESLOTS-1]) {
      // schedule has overflown
      outcome=E_FAIL;
      openserial_printCritical(COMPONENT_SCHEDULE,ERR_SCHEDULE_OVERFLOWN,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      
      
   }
   // fill that schedule entry with parameters passed
   slotContainer->slotOffset                = slotOffset;
   slotContainer->type                      = type;
   slotContainer->shared                    = shared;
   slotContainer->channelOffset             = channelOffset;
   memcpy(&slotContainer->neighbor,neighbor,sizeof(open_addr_t));

   if (schedule_vars.currentScheduleEntry==NULL) {
      // this is the first active slot added

      // the next slot of this slot is this slot
      slotContainer->next                   = slotContainer;

      // current slot points to this slot
      schedule_vars.currentScheduleEntry    = slotContainer;
   } else  {
      // this is NOT the first active slot added

      // find position in schedule
      previousSlotWalker                    = schedule_vars.currentScheduleEntry;
      while (1) {
         nextSlotWalker                     = previousSlotWalker->next;
         if (
               (
                     (previousSlotWalker->slotOffset <  slotContainer->slotOffset) &&
                     (slotContainer->slotOffset <  nextSlotWalker->slotOffset)
               )
               ||
               (
                     (previousSlotWalker->slotOffset <  slotContainer->slotOffset) &&
                     (nextSlotWalker->slotOffset <= previousSlotWalker->slotOffset)
               )
               ||
               (
                     (slotContainer->slotOffset <  nextSlotWalker->slotOffset) &&
                     (nextSlotWalker->slotOffset <= previousSlotWalker->slotOffset)
               )
         ) {
            break;
         }
         previousSlotWalker                 = nextSlotWalker;
      }
      // insert between previousSlotWalker and nextSlotWalker
      previousSlotWalker->next              = slotContainer;
      slotContainer->next                   = nextSlotWalker;
   }

   // maintain debug stats
   schedule_dbg.numActiveSlotsCur++;
   if (schedule_dbg.numActiveSlotsCur>schedule_dbg.numActiveSlotsMax) {
      schedule_dbg.numActiveSlotsMax        = schedule_dbg.numActiveSlotsCur;
   }
   outcome=E_SUCCESS;
   ENABLE_INTERRUPTS();
   return outcome;
}



owerror_t   schedule_removeActiveSlot(slotOffset_t   slotOffset, open_addr_t*   neighbor){
  
   owerror_t outcome;
   
   scheduleEntry_t* slotContainer;
   scheduleEntry_t* previousSlotWalker;

   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   
   // find the schedule entry
   slotContainer = &schedule_vars.scheduleBuf[0];
   while (slotContainer->type!=CELLTYPE_OFF && slotContainer<=&schedule_vars.scheduleBuf[MAXACTIVESLOTS-1]) {
          //check that this entry for that neighbour and timeslot is not already scheduled.
           if (packetfunctions_sameAddress(neighbor,&(slotContainer->neighbor))&& (slotContainer->slotOffset==slotOffset)){
               break;
           }
           slotContainer++;
   }
  
   if (slotContainer->next==slotContainer) {
      // this is the last active slot

      // the next slot of this slot is NULL
      slotContainer->next                   = NULL;

      // current slot points to this slot
      schedule_vars.currentScheduleEntry    = NULL;
   } else  {
      // this is NOT the last active slot

      // find the previous in the schedule
      previousSlotWalker                    = schedule_vars.currentScheduleEntry;
      
      while (1) {
        if (previousSlotWalker->next=slotContainer){
            break;
         }
         previousSlotWalker                 = previousSlotWalker->next;
      }
      // remove this element from the linked list
      previousSlotWalker->next              = slotContainer->next;//my next;
      slotContainer->next                   = NULL;
   }

    // clear that schedule entry 
    slotContainer->slotOffset                = 0;
    slotContainer->type                      = CELLTYPE_OFF;
    slotContainer->shared                    = FALSE;
    slotContainer->channelOffset             = 0;
    memset(&slotContainer->neighbor,0,sizeof(open_addr_t));

    // maintain debug stats
    schedule_dbg.numActiveSlotsCur--;
   
    outcome=E_SUCCESS;
    ENABLE_INTERRUPTS();
    
    return outcome;
}




//=== from IEEE802154E: reading the schedule and updating statistics

void schedule_syncSlotOffset(slotOffset_t targetSlotOffset) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   while (schedule_vars.currentScheduleEntry->slotOffset!=targetSlotOffset) {
      schedule_advanceSlot();
   }
   ENABLE_INTERRUPTS();
}

void schedule_advanceSlot() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // advance to next active slot
   schedule_vars.currentScheduleEntry = schedule_vars.currentScheduleEntry->next;
   ENABLE_INTERRUPTS();
}

slotOffset_t schedule_getNextActiveSlotOffset() {
   slotOffset_t res;   
   INTERRUPT_DECLARATION();
   
   // return next active slot's slotOffset
   DISABLE_INTERRUPTS();
   res = ((scheduleEntry_t*)(schedule_vars.currentScheduleEntry->next))->slotOffset;
   ENABLE_INTERRUPTS();
   
   return res;
}

/**
\brief Get the frame length.

\returns The frame length.
*/
frameLength_t schedule_getFrameLength() {
   frameLength_t res;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   res= schedule_vars.frameLength;
   ENABLE_INTERRUPTS();
   
   return res;
}
      
uint8_t schedule_getLinksNumber(uint8_t slotframeID){
    uint8_t i;
    for (i=0;i<MAXACTIVESLOTS;i++){
      if(links[i].link_type == CELLTYPE_OFF)
        break;
   }
   return i;

}

Link_t* schedule_getLinksList(uint8_t slotframeID){
    //return link list
    return links;
}

/**
\brief Get the type of the current schedule entry.

\returns The type of the current schedule entry.
*/
cellType_t schedule_getType() {
   cellType_t res;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   res= schedule_vars.currentScheduleEntry->type;
   ENABLE_INTERRUPTS();
   return res;
}

/**
\brief Get the neighbor associated wit the current schedule entry.

\returns The neighbor associated wit the current schedule entry.
*/
void schedule_getNeighbor(open_addr_t* addrToWrite) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   memcpy(addrToWrite,&(schedule_vars.currentScheduleEntry->neighbor),sizeof(open_addr_t));
   ENABLE_INTERRUPTS();
}

/**
\brief Get the channel offset of the current schedule entry.

\returns The channel offset of the current schedule entry.
*/
channelOffset_t schedule_getChannelOffset() {
   channelOffset_t res;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   res= schedule_vars.currentScheduleEntry->channelOffset;
   ENABLE_INTERRUPTS();
   return res;
}

/**
\brief Check whether I can send on this slot.

This function is called at the beginning of every TX slot.
If the slot is *not* a shared slot, it always return TRUE.
If the slot is a shared slot, it decrements the backoff counter and returns 
TRUE only if it hits 0.

Note that the backoff counter is global, not per slot.

\returns TRUE if it is OK to send on this slot, FALSE otherwise.
 */
bool schedule_getOkToSend() {
   bool returnVal;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   if (schedule_vars.currentScheduleEntry->shared==FALSE) {
      // non-shared slot: backoff does not apply
      
      returnVal = TRUE;
   } else {
      // non-shared slot: check backoff before answering
      
      // decrement backoff
      if (schedule_vars.backoff>0) {
         schedule_vars.backoff--;
      }
      
      // only return TRUE if backoff hit 0
      if (schedule_vars.backoff==0) {
         returnVal = TRUE;
      } else {
         returnVal = FALSE;
      }
   }
   
   ENABLE_INTERRUPTS();
   return returnVal;
}

/**
\brief Reset the backoff and backoffExponent.
*/
void schedule_resetBackoff() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   // reset backoffExponent
   schedule_vars.backoffExponent = MINBE-1;
   // reset backoff
   schedule_vars.backoff         = 0;
   
   ENABLE_INTERRUPTS();
}

/**
\brief Indicate the reception of a packet.
*/
void schedule_indicateRx(asn_t* asnTimestamp) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // increment usage statistics
   schedule_vars.currentScheduleEntry->numRx++;

   // update last used timestamp
   memcpy(&(schedule_vars.currentScheduleEntry->lastUsedAsn), asnTimestamp, sizeof(asn_t));
   ENABLE_INTERRUPTS();
}

/**
\brief Indicate the transmission of a packet.
*/
void schedule_indicateTx(asn_t* asnTimestamp,
                         bool   succesfullTx) {
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // increment usage statistics
   if (schedule_vars.currentScheduleEntry->numTx==0xFF) {
      schedule_vars.currentScheduleEntry->numTx/=2;
      schedule_vars.currentScheduleEntry->numTxACK/=2;
   }
   schedule_vars.currentScheduleEntry->numTx++;
   if (succesfullTx==TRUE) {
      schedule_vars.currentScheduleEntry->numTxACK++;
   }

   // update last used timestamp
   memcpy(&schedule_vars.currentScheduleEntry->lastUsedAsn, asnTimestamp, sizeof(asn_t));

   // update this backoff parameters for shared slots
   if (schedule_vars.currentScheduleEntry->shared==TRUE) {
      if (succesfullTx==TRUE) {
         // reset backoffExponent
         schedule_vars.backoffExponent = MINBE-1;
         // reset backoff
         schedule_vars.backoff         = 0;
      } else {
         // increase the backoffExponent
         if (schedule_vars.backoffExponent<MAXBE) {
            schedule_vars.backoffExponent++;
         }
         // set the backoff to a random value in [0..2^BE]
         schedule_vars.backoff         = openrandom_get16b()%(1<<schedule_vars.backoffExponent);
      }
   }
   
   ENABLE_INTERRUPTS();
}

void schedule_getNetDebugInfo(netDebugScheduleEntry_t* schlist){  
  uint8_t i;
  
  for (i=0;i<MAXACTIVESLOTS;i++){
   schlist[i].last_addr_byte=schedule_vars.scheduleBuf[i].neighbor.addr_64b[7];
   schlist[i].slotOffset=(uint8_t)schedule_vars.scheduleBuf[i].slotOffset&0xFF;
   schlist[i].channelOffset=schedule_vars.scheduleBuf[i].channelOffset;
  }
}

//=========================== reservation ====================================
//returns the number of slot frames?
uint8_t schedule_getNumSlotframe(){
  return SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER;
}

void schedule_uResGenerateCandidataLinkList(uint8_t slotframeID){
    uint8_t j = 0;
    Link_t tempLink;
    // for test
     for (uint8_t i=0;i<MAXACTIVESLOTS;i++)
     {
        tempLink.channelOffset = 0;
        tempLink.slotOffset = i;
        tempLink.link_type = CELLTYPE_TX;
        if(isOneAvailableLink(tempLink) && isOneRequestedLink(tempLink)){
          links[j].channelOffset = tempLink.channelOffset;
          links[j].slotOffset = tempLink.slotOffset;
          links[j].link_type = tempLink.link_type;
          j++;
        }
      }
}

void schedule_uResGenerateRemoveLinkList(uint8_t slotframeID,Link_t tempLink){
    // this function should be called by upper layers to generate links to be remove
    // if someone want to use reservation to remove links, they should be added to links[MAXACTIVESLOTS]
        links[0] = tempLink;
}

void schedule_allocateLinks(uint8_t slotframeID,uint8_t numOfLink,uint8_t bandwidth){
  
  uint8_t j = 0;
  
  for(uint8_t i=0;i<numOfLink;i++)
  {
    if(isOneAvailableLink(links[i]))
    {
      if(i>j)
        memcpy(&(links[j]),&(links[i]),sizeof(Link_t));
      j++;
      if(j == bandwidth)
      {
        memset(&(links[j]),0,(MAXACTIVESLOTS-j)*sizeof(Link_t));
        return;
      }
    }
  }
  memset(&(links[0]),0,MAXACTIVESLOTS*sizeof(Link_t));
}

scheduleEntry_t* schedule_getScheduleEntry(uint16_t slotOffset){
    scheduleEntry_t* tempScheduleEntry = schedule_vars.currentScheduleEntry;
  
  do
  {
    if(slotOffset == tempScheduleEntry->slotOffset)
      return tempScheduleEntry;
    
    tempScheduleEntry = tempScheduleEntry->next;
    
  }while(tempScheduleEntry != schedule_vars.currentScheduleEntry);
  return NULL;
}

void schedule_addLinksToSchedule(uint8_t slotframeID,open_addr_t* previousHop,uint8_t numOfLinks,uint8_t state){
    //set schedule according links
  open_addr_t temp_neighbor;
  for(uint8_t i = 0;i<numOfLinks;i++)
  {
    if(schedule_checkExistSchedule(links[i].slotOffset) == FALSE)
    {
      if(links[i].link_type == CELLTYPE_TX)
      {
        switch(state) {
          case S_SIXTOP_LINKREQUEST_RECEIVE:
            memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
            //add a RX link
            schedule_addActiveSlot(links[i].slotOffset,
              CELLTYPE_RX,
              FALSE,
              links[i].channelOffset,
              &temp_neighbor,
              FALSE);
            break;
          case S_SIXTOP_LINKRESPONSE_RECEIVE:
            memcpy(&temp_neighbor,previousHop,sizeof(open_addr_t));
            //add a TX link
            schedule_addActiveSlot(links[i].slotOffset,
              CELLTYPE_TX,
              FALSE,
              links[i].channelOffset,
              &temp_neighbor,
              FALSE);
            memset(&(links[i]),0,sizeof(Link_t));
            break;
          default:
          //log error
            break;
        }
      }
    }
  }
// memset(&(links[0]),0,MAXACTIVESLOTS*sizeof(Link_t));
}

void schedule_removeLinksFromSchedule(uint8_t slotframeID,uint16_t slotframeSize,uint8_t numOfLink,open_addr_t* previousHop,uint8_t state){
  //set schedule according links
  open_addr_t temp_neighbor;
  scheduleEntry_t* tempScheduleEntry;
  for(uint8_t i = 0;i<numOfLink;i++)
  {
    if(schedule_checkExistSchedule(links[i].slotOffset))
    {
      tempScheduleEntry = schedule_getScheduleEntry(links[i].slotOffset);
      
      if(tempScheduleEntry == NULL)
      {
        //log error
        return;
      }
      //get reference neighbor of Slot
      memcpy(&(temp_neighbor),&(tempScheduleEntry->neighbor),sizeof(open_addr_t));
      
      if((links[i].link_type == CELLTYPE_RX || links[i].link_type == CELLTYPE_TX) && packetfunctions_sameAddress(&(temp_neighbor),previousHop))
      {
        switch (state){
          case S_REMOVELINKREQUEST_SEND:
              // remove CELLTYPE_TX link from shedule
            schedule_removeActiveSlot(links[i].slotOffset,
              &(temp_neighbor));
            break;
          case S_REMOVELINKREQUEST_RECEIVE:
            //remove CELLTYPE_RX link from shedule
            schedule_removeActiveSlot(links[i].slotOffset,
              &(temp_neighbor));
              memset(links,0,MAXACTIVESLOTS*sizeof(Link_t));
            break;
        default:
          //log error
          break;
        }

      }
    }
  }
}

//=========================== private =========================================

bool isOneAvailableLink(Link_t tempLink){
  scheduleEntry_t* tempScheduleEntry = schedule_vars.currentScheduleEntry;
  
  do
  {
    if(tempLink.slotOffset == tempScheduleEntry->slotOffset)
      return FALSE;
    
    tempScheduleEntry = tempScheduleEntry->next;
    
  }while(tempScheduleEntry != schedule_vars.currentScheduleEntry);
  return TRUE;
}

bool isOneRequestedLink(Link_t tempLink){
  for(uint8_t i=0;i<MAXACTIVESLOTS;i++)
  {
    if(tempLink.slotOffset == links[i].slotOffset)
      return FALSE;
  }
  return TRUE;
}

bool schedule_checkExistSchedule(uint16_t slotOffset){
   for (uint8_t i=0;i<MAXACTIVESLOTS;i++){
      if(schedule_vars.scheduleBuf[i].slotOffset == slotOffset)
        return TRUE;
   }
  return FALSE;
}

void schedule_resetEntry(scheduleEntry_t* pScheduleEntry) {
   pScheduleEntry->type                     = CELLTYPE_OFF;
   pScheduleEntry->shared                   = FALSE;
   pScheduleEntry->channelOffset            = 0;

   pScheduleEntry->neighbor.type            = ADDR_NONE;
   memset(&pScheduleEntry->neighbor.addr_64b[0], 0x00, sizeof(pScheduleEntry->neighbor.addr_64b));

   pScheduleEntry->numRx                    = 0;
   pScheduleEntry->numTx                    = 0;
   pScheduleEntry->numTxACK                 = 0;
   pScheduleEntry->lastUsedAsn.bytes0and1   = 0;
   pScheduleEntry->lastUsedAsn.bytes2and3   = 0;
   pScheduleEntry->lastUsedAsn.byte4        = 0;
}
