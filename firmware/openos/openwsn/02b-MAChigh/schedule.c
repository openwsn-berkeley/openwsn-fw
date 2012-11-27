#include "openwsn.h"
#include "schedule.h"
#include "openserial.h"
#include "idmanager.h"
#include "openrandom.h"
#include "board_info.h"

//=========================== variables =======================================

typedef struct {
   scheduleEntry_t  scheduleBuf[MAXACTIVESLOTS];
   scheduleEntry_t* currentScheduleEntry;
   uint16_t         frameLength;
   slotOffset_t     debugPrintRow;
} schedule_vars_t;

schedule_vars_t schedule_vars;

typedef struct {
   uint8_t          numActiveSlotsCur;
   uint8_t          numActiveSlotsMax;
} schedule_dbg_t;

schedule_dbg_t schedule_dbg;

//=========================== prototypes ======================================

void schedule_resetEntry(scheduleEntry_t* pScheduleEntry);

//=========================== public ==========================================

//=== admin

void schedule_init() {
   uint8_t     i;
   open_addr_t temp_neighbor;

   // reset local variables
   memset(&schedule_vars,0,sizeof(schedule_vars_t));
   memset(&schedule_dbg, 0,sizeof(schedule_dbg_t));
   for (i=0;i<MAXACTIVESLOTS;i++){
      schedule_resetEntry(&schedule_vars.scheduleBuf[i]);
   }

   // set frame length
   schedule_setFrameLength(9);

   // slot 0 is advertisement slot
   i = 0;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   schedule_addActiveSlot(i,
         CELLTYPE_ADV,
         FALSE,
         0,
         &temp_neighbor);

   // slot 1 is shared TXRX anycast
   i = 1;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   temp_neighbor.type             = ADDR_ANYCAST;
   schedule_addActiveSlot(i,
         CELLTYPE_TXRX,
         TRUE,
         0,
         &temp_neighbor);
   
   i = 2;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   temp_neighbor.type             = ADDR_ANYCAST;
   schedule_addActiveSlot(i,
         CELLTYPE_TXRX,
         TRUE,
         0,
         &temp_neighbor);
   
   
   i = 3;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   temp_neighbor.type             = ADDR_ANYCAST;
   schedule_addActiveSlot(i,
         CELLTYPE_TXRX,
         TRUE,
         0,
         &temp_neighbor);
   
   
   i = 4;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   temp_neighbor.type             = ADDR_ANYCAST;
   schedule_addActiveSlot(i,
         CELLTYPE_TXRX,
         TRUE,
         0,
         &temp_neighbor);

   // slot 2 is SERIALRX
   i = 5;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   schedule_addActiveSlot(i,
         CELLTYPE_SERIALRX,
         FALSE,
         0,
         &temp_neighbor);

   // slot 3 is MORESERIALRX
   i = 6;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   schedule_addActiveSlot(i,
         CELLTYPE_MORESERIALRX,
         FALSE,
         0,
         &temp_neighbor);

   // slot 4 is MORESERIALRX
   i = 7;
   memset(&temp_neighbor,0,sizeof(temp_neighbor));
   schedule_addActiveSlot(i,
         CELLTYPE_MORESERIALRX,
         FALSE,
         0,
         &temp_neighbor);
}

bool debugPrint_schedule() {
   debugScheduleEntry_t temp;
   schedule_vars.debugPrintRow    = (schedule_vars.debugPrintRow+1)%MAXACTIVESLOTS;
   temp.row                       = schedule_vars.debugPrintRow;
   temp.scheduleEntry             = schedule_vars.scheduleBuf[schedule_vars.debugPrintRow];
   openserial_printStatus(STATUS_SCHEDULE,
         (uint8_t*)&temp,
         sizeof(debugScheduleEntry_t));
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
\brief Add a new active slot into the schedule.

\param newFrameLength The new frame length.
 */
void schedule_addActiveSlot(slotOffset_t    slotOffset,
      cellType_t      type,
      bool            shared,
      uint8_t         channelOffset,
      open_addr_t*    neighbor) {
   scheduleEntry_t* slotContainer;
   scheduleEntry_t* previousSlotWalker;
   scheduleEntry_t* nextSlotWalker;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   // find an empty schedule entry container
   slotContainer = &schedule_vars.scheduleBuf[0];
   while (slotContainer->type!=CELLTYPE_OFF &&
         slotContainer<=&schedule_vars.scheduleBuf[MAXACTIVESLOTS-1]) {
      slotContainer++;
   }
   if (slotContainer>&schedule_vars.scheduleBuf[MAXACTIVESLOTS-1]) {
      // schedule has overflown
      while(1);
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
   ENABLE_INTERRUPTS();
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

This function is called at the beginning of every TX slot. If the slot is not a
shared slot, it always return TRUE. If the slot is a shared slot, it decrements
the backoff counter and returns TRUE only if it hits 0.

\returns TRUE if it is OK to send on this slot, FALSE otherwise.
 */
bool schedule_getOkToSend() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   // decrement backoff of that slot
   if (schedule_vars.currentScheduleEntry->backoff>0) {
      schedule_vars.currentScheduleEntry->backoff--;
   }
   // check whether backoff has hit 0
   if (
         schedule_vars.currentScheduleEntry->shared==FALSE ||
         (
               schedule_vars.currentScheduleEntry->shared==TRUE &&
               schedule_vars.currentScheduleEntry->backoff==0
         )
   ) {
      ENABLE_INTERRUPTS();
      return TRUE;
   } else {
      ENABLE_INTERRUPTS();
      return FALSE;
   }
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
 void schedule_indicateTx(asn_t*   asnTimestamp,
   bool     succesfullTx) {
   
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

   // update this slot's backoff parameters
   if (succesfullTx==TRUE) {
      // reset backoffExponent
      schedule_vars.currentScheduleEntry->backoffExponent   = MINBE-1;
      // reset backoff
      schedule_vars.currentScheduleEntry->backoff           = 0;
   } else {
      // increase the backoffExponent
      if (schedule_vars.currentScheduleEntry->backoffExponent<MAXBE) {
         schedule_vars.currentScheduleEntry->backoffExponent++;
      }
      // set the backoff to a random value in [0..2^BE]
      schedule_vars.currentScheduleEntry->backoff =
            openrandom_get16b()%(1<<schedule_vars.currentScheduleEntry->backoffExponent);
   }
   ENABLE_INTERRUPTS();
}

/**
// The functio below returns a direct pointer to the scheduleBuf. Modifying 
this structure means you are modifying the scheduleBuf. Be careful when using
the pointer and try only to read from the buffer

NOBODY uses that- poipoi xv check and delete if not needed.
*/
/*void scheduleBuf_getAll(scheduleEntry_t *blist){
 blist=&schedule_vars.scheduleBuf[0];
}*/


//TODO, check that the number of bytes is not bigger than maxbytes. If so, retun error.
void schedule_getNetDebugInfo(netDebugScheduleEntry_t *schlist, uint8_t maxbytes){
  
  uint8_t i;
  for (i=0;i<MAXACTIVESLOTS;i++){
   schlist[i].last_addr_byte=schedule_vars.scheduleBuf[i].neighbor.addr_64b[7];
   schlist[i].slotOffset=(uint8_t)schedule_vars.scheduleBuf[i].slotOffset&0xFF;
   schlist[i].channelOffset=schedule_vars.scheduleBuf[i].channelOffset;
  }    

}
//=========================== private =========================================

void schedule_resetEntry(scheduleEntry_t* pScheduleEntry) {
   pScheduleEntry->type                     = CELLTYPE_OFF;
   pScheduleEntry->shared                   = FALSE;
   pScheduleEntry->backoffExponent          = MINBE-1;
   pScheduleEntry->backoff                  = 0;
   pScheduleEntry->channelOffset            = 0;
   pScheduleEntry->neighbor.type            = ADDR_NONE;
   pScheduleEntry->neighbor.addr_64b[0]     = 0x14;
   pScheduleEntry->neighbor.addr_64b[1]     = 0x15;
   pScheduleEntry->neighbor.addr_64b[2]     = 0x92;
   pScheduleEntry->neighbor.addr_64b[3]     = 0x09;
   pScheduleEntry->neighbor.addr_64b[4]     = 0x02;
   pScheduleEntry->neighbor.addr_64b[5]     = 0x2c;
   pScheduleEntry->neighbor.addr_64b[6]     = 0x00;
   pScheduleEntry->numRx                    = 0;
   pScheduleEntry->numTx                    = 0;
   pScheduleEntry->numTxACK                 = 0;
   pScheduleEntry->lastUsedAsn.bytes0and1   = 0;
   pScheduleEntry->lastUsedAsn.bytes2and3   = 0;
   pScheduleEntry->lastUsedAsn.byte4        = 0;
}
