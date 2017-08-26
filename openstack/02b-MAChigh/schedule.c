#include "opendefs.h"
#include "schedule.h"
#include "openserial.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "sixtop.h"
#include "idmanager.h"
#include "sf0.h"
#include "icmpv6rpl.h"

//=========================== variables =======================================

schedule_vars_t schedule_vars;

//=========================== prototypes ======================================

void schedule_resetEntry(scheduleEntry_t* pScheduleEntry);

//=========================== public ==========================================

//=== admin

/**
\brief Initialize this module.

\post Call this function before calling any other function in this module.
*/
void schedule_init() {
   slotOffset_t    start_slotOffset;
   slotOffset_t    running_slotOffset;
   open_addr_t     temp_neighbor;

   // reset local variables
   memset(&schedule_vars,0,sizeof(schedule_vars_t));
   for (running_slotOffset=0;running_slotOffset<MAXACTIVESLOTS;running_slotOffset++) {
      schedule_resetEntry(&schedule_vars.scheduleBuf[running_slotOffset]);
   }
   schedule_vars.backoffExponent = MINBE-1;
   schedule_vars.maxActiveSlots = MAXACTIVESLOTS;
   
   start_slotOffset = SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET;
   if (idmanager_getIsDAGroot()==TRUE) {
      schedule_startDAGroot();
   }
   
   // serial RX slot(s)
   start_slotOffset += SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   for (running_slotOffset=start_slotOffset;running_slotOffset<start_slotOffset+NUMSERIALRX;running_slotOffset++) {
      schedule_addActiveSlot(
         running_slotOffset,                    // slot offset
         CELLTYPE_SERIALRX,                     // type of slot
         FALSE,                                 // shared?
         0,                                     // channel offset
         &temp_neighbor                         // neighbor
      );
   }
}

/**
\brief Starting the DAGroot schedule propagation.
*/
void schedule_startDAGroot() {
   slotOffset_t    start_slotOffset;
   slotOffset_t    running_slotOffset;
   open_addr_t     temp_neighbor;
   
   start_slotOffset = SCHEDULE_MINIMAL_6TISCH_SLOTOFFSET;
   // set frame length, handle and number (default 1 by now)
   if (schedule_vars.frameLength == 0) {
       // slotframe length is not set, set it to default length
       schedule_setFrameLength(SLOTFRAME_LENGTH);
   } else {
       // slotframe elgnth is set, nothing to do here
   }
   schedule_setFrameHandle(SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE);
   schedule_setFrameNumber(SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_NUMBER);

   // shared TXRX anycast slot(s)
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   temp_neighbor.type             = ADDR_ANYCAST;
   for (running_slotOffset=start_slotOffset;running_slotOffset<start_slotOffset+SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS;running_slotOffset++) {
      schedule_addActiveSlot(
         running_slotOffset,                 // slot offset
         CELLTYPE_TXRX,                      // type of slot
         TRUE,                               // shared?
         SCHEDULE_MINIMAL_6TISCH_CHANNELOFFSET,    // channel offset
         &temp_neighbor                      // neighbor
      );
   }
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_schedule() {
   debugScheduleEntry_t temp;
   
   // increment the row just printed
   schedule_vars.debugPrintRow         = (schedule_vars.debugPrintRow+1)%schedule_vars.maxActiveSlots;
   
   // gather status data
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
   
   // send status data over serial port
   openserial_printStatus(
      STATUS_SCHEDULE,
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
   
   // gather status data
   temp[0] = schedule_vars.backoffExponent;
   temp[1] = schedule_vars.backoff;
   
   // send status data over serial port
   openserial_printStatus(
      STATUS_BACKOFF,
      (uint8_t*)&temp,
      sizeof(temp)
   );
   
   return TRUE;
}

//=== from 6top (writing the schedule)

/**
\brief Set frame length.

\param newFrameLength The new frame length.
*/
void schedule_setFrameLength(frameLength_t newFrameLength) {
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   schedule_vars.frameLength = newFrameLength;
   if (newFrameLength <= MAXACTIVESLOTS) {
      schedule_vars.maxActiveSlots = newFrameLength;
   }
   ENABLE_INTERRUPTS();
}

/**
\brief Set frame handle.

\param frameHandle The new frame handle.
*/
void schedule_setFrameHandle(uint8_t frameHandle) {
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   schedule_vars.frameHandle = frameHandle;
   
   ENABLE_INTERRUPTS();
}

/**
\brief Set frame number.

\param frameNumber The new frame number.
*/
void schedule_setFrameNumber(uint8_t frameNumber) {
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   schedule_vars.frameNumber = frameNumber;
   
   ENABLE_INTERRUPTS();
}

/**
\brief Get the information of a specific slot.

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
   while (slotContainer<=&schedule_vars.scheduleBuf[schedule_vars.maxActiveSlots-1]) {
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
\brief Get the maximum number of active slots.

\returns maximum number of active slots
*/
uint16_t  schedule_getMaxActiveSlots() {
   return schedule_vars.maxActiveSlots;
}

/**
\brief Add a new active slot into the schedule.

\param slotOffset       The slotoffset of the new slot
\param type             The type of the cell
\param shared           Whether this cell is shared (TRUE) or not (FALSE).
\param channelOffset    The channelOffset of the new slot
\param neighbor         The neighbor associated with this cell (all 0's if
   none)
*/
owerror_t schedule_addActiveSlot(
      slotOffset_t    slotOffset,
      cellType_t      type,
      bool            shared,
      channelOffset_t channelOffset,
      open_addr_t*    neighbor
   ) {
   scheduleEntry_t* slotContainer;
   scheduleEntry_t* previousSlotWalker;
   scheduleEntry_t* nextSlotWalker;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   // find an empty schedule entry container
   slotContainer = &schedule_vars.scheduleBuf[0];
   while (
         slotContainer->type!=CELLTYPE_OFF &&
         slotContainer<=&schedule_vars.scheduleBuf[schedule_vars.maxActiveSlots-1]
      ) {
      slotContainer++;
   }
   
   // abort it schedule overflow
   if (slotContainer>&schedule_vars.scheduleBuf[schedule_vars.maxActiveSlots-1]) {
      ENABLE_INTERRUPTS();
      openserial_printError(
         COMPONENT_SCHEDULE,ERR_SCHEDULE_OVERFLOWN,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return E_FAIL;
   }
   
   // fill that schedule entry with parameters passed
   slotContainer->slotOffset                = slotOffset;
   slotContainer->type                      = type;
   slotContainer->shared                    = shared;
   slotContainer->channelOffset             = channelOffset;
   memcpy(&slotContainer->neighbor,neighbor,sizeof(open_addr_t));
   
   // insert in circular list
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
         if (previousSlotWalker->slotOffset == slotContainer->slotOffset) {
            // slot is already in schedule
            openserial_printError(
               COMPONENT_SCHEDULE,ERR_SCHEDULE_ADDDUPLICATESLOT,
               (errorparameter_t)slotContainer->slotOffset,
               (errorparameter_t)0
            );
            // reset the entry
            slotContainer->slotOffset                = 0;
            slotContainer->type                      = CELLTYPE_OFF;
            slotContainer->shared                    = FALSE;
            slotContainer->channelOffset             = 0;
            memset(&slotContainer->neighbor,0,sizeof(open_addr_t));
            ENABLE_INTERRUPTS();
            return E_FAIL;
         }
         previousSlotWalker                 = nextSlotWalker;
      }
      // insert between previousSlotWalker and nextSlotWalker
      previousSlotWalker->next              = slotContainer;
      slotContainer->next                   = nextSlotWalker;
   }
   
   ENABLE_INTERRUPTS();
   return E_SUCCESS;
}

/**
\brief Remove an active slot from the schedule.

\param slotOffset       The slotoffset of the slot to remove.
\param neighbor         The neighbor associated with this cell (all 0's if
   none)
*/
owerror_t schedule_removeActiveSlot(slotOffset_t slotOffset, open_addr_t* neighbor) {
   scheduleEntry_t* slotContainer;
   scheduleEntry_t* previousSlotWalker;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   // find the schedule entry
   slotContainer = &schedule_vars.scheduleBuf[0];
   while (slotContainer<=&schedule_vars.scheduleBuf[schedule_vars.maxActiveSlots-1]) {
      if (
            slotContainer->slotOffset==slotOffset
            &&
            packetfunctions_sameAddress(neighbor,&(slotContainer->neighbor))
            ){
         break;
      }
      slotContainer++;
   }
   
   // abort it could not find
   if (slotContainer>&schedule_vars.scheduleBuf[schedule_vars.maxActiveSlots-1]) {
      ENABLE_INTERRUPTS();
      openserial_printCritical(
         COMPONENT_SCHEDULE,ERR_FREEING_ERROR,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return E_FAIL;
   }
   
   // remove from linked list
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
         if (previousSlotWalker->next==slotContainer){
            break;
         }
         previousSlotWalker                 = previousSlotWalker->next;
      }
      
      // remove this element from the linked list, i.e. have the previous slot
      // "jump" to slotContainer's next
      previousSlotWalker->next              = slotContainer->next;
      
      // update current slot if points to slot I just removed
      if (schedule_vars.currentScheduleEntry==slotContainer) {
         schedule_vars.currentScheduleEntry = slotContainer->next;
      }
   }
   
   // reset removed schedule entry
   schedule_resetEntry(slotContainer);
   
   ENABLE_INTERRUPTS();
   
   return E_SUCCESS;
}

bool schedule_isSlotOffsetAvailable(uint16_t slotOffset){
   
   scheduleEntry_t* scheduleWalker;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   scheduleWalker = schedule_vars.currentScheduleEntry;
   do {
      if(slotOffset == scheduleWalker->slotOffset){
          ENABLE_INTERRUPTS();
          return FALSE;
      }
      scheduleWalker = scheduleWalker->next;
   }while(scheduleWalker!=schedule_vars.currentScheduleEntry);
   
   ENABLE_INTERRUPTS();
   
   return TRUE;
}

uint16_t  schedule_getCellsCounts(uint8_t frameID,cellType_t type, open_addr_t* neighbor){
    uint16_t         count = 0;
    scheduleEntry_t* scheduleWalker;
   
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    
    if (frameID != SCHEDULE_MINIMAL_6TISCH_DEFAULT_SLOTFRAME_HANDLE){
        ENABLE_INTERRUPTS();
        return 0;
    }
   
    scheduleWalker = schedule_vars.currentScheduleEntry;
    do {
       if(
          packetfunctions_sameAddress(&(scheduleWalker->neighbor),neighbor) &&
          type == scheduleWalker->type
       ){
           count++;
       }
       scheduleWalker = scheduleWalker->next;
    }while(scheduleWalker!=schedule_vars.currentScheduleEntry);
   
    ENABLE_INTERRUPTS();
    return count;
}
void schedule_removeAllCells(
    uint8_t        slotframeID,
    open_addr_t*   previousHop
    ){
    uint8_t i;
    
    // remove all entries in schedule with previousHop address
    for(i=0;i<MAXACTIVESLOTS;i++){
        if (packetfunctions_sameAddress(&(schedule_vars.scheduleBuf[i].neighbor),previousHop)){
           schedule_removeActiveSlot(
              schedule_vars.scheduleBuf[i].slotOffset,
              previousHop
           );
        }
    }
}

scheduleEntry_t* schedule_getCurrentScheduleEntry(){
    return schedule_vars.currentScheduleEntry;
}

//=== from otf
uint8_t schedule_getNumOfSlotsByType(cellType_t type){
   uint8_t returnVal;
   scheduleEntry_t* scheduleWalker;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   returnVal = 0;
   scheduleWalker = schedule_vars.currentScheduleEntry;
   do {
      if(type == scheduleWalker->type){
          returnVal += 1;
      }
      scheduleWalker = scheduleWalker->next;
   }while(scheduleWalker!=schedule_vars.currentScheduleEntry);
   
   ENABLE_INTERRUPTS();
   
   return returnVal;
}

uint8_t schedule_getNumberOfFreeEntries(){
   uint8_t i; 
   uint8_t counter;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   counter = 0;
   for(i=0;i<MAXACTIVESLOTS;i++) {
      if(schedule_vars.scheduleBuf[i].type == CELLTYPE_OFF){
         counter++;
      }
   }
   
   ENABLE_INTERRUPTS();
   
   return counter;
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

/**
\brief advance to next active slot
*/
void schedule_advanceSlot() {
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   if (schedule_vars.currentScheduleEntry->slotOffset >= ((scheduleEntry_t*)schedule_vars.currentScheduleEntry->next)->slotOffset
       ) {
       // one slotframe has elapsed
       sf0_notifyNewSlotframe();
   }   
   schedule_vars.currentScheduleEntry = schedule_vars.currentScheduleEntry->next;
   
   ENABLE_INTERRUPTS();
}

/**
\brief return slotOffset of next active slot
*/
slotOffset_t schedule_getNextActiveSlotOffset() {
   slotOffset_t res;   
   
   INTERRUPT_DECLARATION();
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
   frameLength_t returnVal;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   returnVal = schedule_vars.frameLength;
   
   ENABLE_INTERRUPTS();
   
   return returnVal;
}

/**
\brief Get the frame handle.

\returns The frame handle.
*/
uint8_t schedule_getFrameHandle() {
   uint8_t returnVal;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   returnVal = schedule_vars.frameHandle;
   
   ENABLE_INTERRUPTS();
   
   return returnVal;
}

/**
\brief Get the frame number.

\returns The frame number.
*/
uint8_t schedule_getFrameNumber() {
   uint8_t returnVal;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   returnVal = schedule_vars.frameNumber;
   
   ENABLE_INTERRUPTS();
   
   return returnVal;
}
/**
\brief Get the type of the current schedule entry.

\returns The type of the current schedule entry.
*/
cellType_t schedule_getType() {
   cellType_t returnVal;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   returnVal = schedule_vars.currentScheduleEntry->type;
   
   ENABLE_INTERRUPTS();
   
   return returnVal;
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
   channelOffset_t returnVal;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   returnVal = schedule_vars.currentScheduleEntry->channelOffset;
   
   ENABLE_INTERRUPTS();
   
   return returnVal;
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
void schedule_indicateTx(asn_t* asnTimestamp, bool succesfullTx) {
   
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


void schedule_housekeeping(){
    uint8_t     i;
    open_addr_t neighbor;
    
    
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    for(i=0;i<MAXACTIVESLOTS;i++) {
        if(schedule_vars.scheduleBuf[i].type == CELLTYPE_TX){
            // remove Tx cell if it's scheduled to non-preferred parent
            if (icmpv6rpl_getPreferredParentEui64(&neighbor)==TRUE) {
                if(packetfunctions_sameAddress(&neighbor,&(schedule_vars.scheduleBuf[i].neighbor))==FALSE){
                    if (sixtop_setHandler(SIX_HANDLER_SF0)==FALSE){
                       // one sixtop transcation is happening, only one instance at one time
                       continue;
                    }
                    sixtop_request(IANA_6TOP_CMD_CLEAR,&(schedule_vars.scheduleBuf[i].neighbor),1);
                    break;
                }
            }
        }
    }
   
    ENABLE_INTERRUPTS();
}

//=========================== private =========================================

/**
\pre This function assumes interrupts are already disabled.
*/
void schedule_resetEntry(scheduleEntry_t* e) {
   e->slotOffset             = 0;
   e->type                   = CELLTYPE_OFF;
   e->shared                 = FALSE;
   e->channelOffset          = 0;

   e->neighbor.type          = ADDR_NONE;
   memset(&e->neighbor.addr_64b[0], 0x00, sizeof(e->neighbor.addr_64b));

   e->numRx                  = 0;
   e->numTx                  = 0;
   e->numTxACK               = 0;
   e->lastUsedAsn.bytes0and1 = 0;
   e->lastUsedAsn.bytes2and3 = 0;
   e->lastUsedAsn.byte4      = 0;
   e->next                   = NULL;
}
