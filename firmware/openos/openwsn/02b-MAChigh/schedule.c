#include "openwsn.h"
#include "schedule.h"
#include "openserial.h"
#include "idmanager.h"
#include "random.h"

//=========================== variables =======================================

typedef struct {
   scheduleRow_t schedule[SCHEDULELENGTH];
   slotOffset_t  debugPrintRow;
} schedule_vars_t;

schedule_vars_t schedule_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//=== admin

void schedule_init() {
   uint8_t i;
   // for debug print
   schedule_vars.debugPrintRow                             = 0;
   //all slots OFF
   for (i=0;i<SCHEDULELENGTH;i++){
      schedule_vars.schedule[i].type                       = CELLTYPE_OFF;
      schedule_vars.schedule[i].shared                     = FALSE;
      schedule_vars.schedule[i].backoffExponent            = MINBE-1;
      schedule_vars.schedule[i].backoff                    = 0;
      schedule_vars.schedule[i].channelOffset              = 0;
      schedule_vars.schedule[i].neighbor.type              = ADDR_NONE;
      schedule_vars.schedule[i].neighbor.addr_64b[0]       = 0x14;
      schedule_vars.schedule[i].neighbor.addr_64b[1]       = 0x15;
      schedule_vars.schedule[i].neighbor.addr_64b[2]       = 0x92;
      schedule_vars.schedule[i].neighbor.addr_64b[3]       = 0x09;
      schedule_vars.schedule[i].neighbor.addr_64b[4]       = 0x02;
      schedule_vars.schedule[i].neighbor.addr_64b[5]       = 0x2c;
      schedule_vars.schedule[i].neighbor.addr_64b[6]       = 0x00;
      schedule_vars.schedule[i].numRx                      = 0;
      schedule_vars.schedule[i].numTx                      = 0;
      schedule_vars.schedule[i].numTxACK                   = 0;
      schedule_vars.schedule[i].asn.bytes0and1             = 0;
      schedule_vars.schedule[i].asn.bytes2and3             = 0;
      schedule_vars.schedule[i].asn.byte4                  = 0;
      
   }
   
   //slot 0 is advertisement slot
   i = 0;
   schedule_vars.schedule[0].type                          = CELLTYPE_ADV;
   schedule_vars.schedule[i].shared                        = FALSE;
   schedule_vars.schedule[i].backoff                       = 0;
   schedule_vars.schedule[i].channelOffset                 = 0;
   
   //slot 1 is shared TXRX anycast
   i = 1;
   schedule_vars.schedule[i].type                          = CELLTYPE_TXRX;
   schedule_vars.schedule[i].shared                        = TRUE;
   schedule_vars.schedule[i].backoff                       = 0;
   schedule_vars.schedule[i].channelOffset                 = 0;
   schedule_vars.schedule[i].neighbor.type                 = ADDR_ANYCAST;
   
   //slot 2 is SERIALRX
   i = 2;
   schedule_vars.schedule[i].type                          = CELLTYPE_SERIALRX;
   
   //slot 3 is MORESERIALRX
   i = 3;
   schedule_vars.schedule[i].type                          = CELLTYPE_MORESERIALRX;

   //slot 4 is MORESERIALRX
   i = 4;
   schedule_vars.schedule[i].type                          = CELLTYPE_MORESERIALRX;
   
   /*
   //slot 1: MASTER -> _2
   i = 1;
   switch (idmanager_getMyID(ADDR_16B)->addr_16b[1]) {
      case DEBUG_MOTEID_MASTER:
         // TX to _2
         schedule_vars.schedule[i].type                    = CELLTYPE_TX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_2;
         break;
      case DEBUG_MOTEID_2:
         // RX from MASTER
         schedule_vars.schedule[i].type                    = CELLTYPE_RX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_MASTER;
         break;
      case DEBUG_MOTEID_3:
         // OFF
         break;
   }
   //slot 2: _2 -> MASTER
   i = 2;
   switch (idmanager_getMyID(ADDR_16B)->addr_16b[1]) {
      case DEBUG_MOTEID_MASTER:
         // RX from _2
         schedule_vars.schedule[i].type                    = CELLTYPE_RX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_2;
         break;
      case DEBUG_MOTEID_2:
         // TX to MASTER
         schedule_vars.schedule[i].type                    = CELLTYPE_TX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_MASTER;
         break;
      case DEBUG_MOTEID_3:
         // OFF
         break;
   }
   
   //slot 3: MASTER -> _3
   i = 3;
   switch (idmanager_getMyID(ADDR_16B)->addr_16b[1]) {
      case DEBUG_MOTEID_MASTER:
         // TX to _3
         schedule_vars.schedule[i].type                    = CELLTYPE_TX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_3;
         break;
      case DEBUG_MOTEID_2:
         // OFF
         break;
      case DEBUG_MOTEID_3:
         // RX from MASTER
         schedule_vars.schedule[i].type                    = CELLTYPE_RX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_MASTER;
         break;
   }
   //slot 4: _3 -> MASTER
   i = 4;
   switch (idmanager_getMyID(ADDR_16B)->addr_16b[1]) {
      case DEBUG_MOTEID_MASTER:
         // RX from _3
         schedule_vars.schedule[i].type                    = CELLTYPE_RX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_3;
         break;
      case DEBUG_MOTEID_2:
         // OFF
         break;
      case DEBUG_MOTEID_3:
         // TX to MASTER
         schedule_vars.schedule[i].type                    = CELLTYPE_TX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_MASTER;
         break;
   }
   
   //slot 5: _2 -> _3
   i = 5;
   switch (idmanager_getMyID(ADDR_16B)->addr_16b[1]) {
      case DEBUG_MOTEID_MASTER:
         // OFF
         break;
      case DEBUG_MOTEID_2:
         // TX to _3
         schedule_vars.schedule[i].type                    = CELLTYPE_TX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_3;
         break;
      case DEBUG_MOTEID_3:
         // RX from _2
         schedule_vars.schedule[i].type                    = CELLTYPE_RX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_2;
         break;
   }
   //slot 6: _3 -> _2
   i = 6;
   switch (idmanager_getMyID(ADDR_16B)->addr_16b[1]) {
      case DEBUG_MOTEID_MASTER:
         // OFF
         break;
      case DEBUG_MOTEID_2:
         // RX from _3
         schedule_vars.schedule[i].type                    = CELLTYPE_RX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_3;
         break;
      case DEBUG_MOTEID_3:
         // TX to _2
         schedule_vars.schedule[i].type                    = CELLTYPE_TX;
         schedule_vars.schedule[i].shared                  = FALSE;
         schedule_vars.schedule[i].backoff                 = 0;
         schedule_vars.schedule[i].channelOffset           = 0;
         schedule_vars.schedule[i].neighbor.type           = ADDR_64B;
         schedule_vars.schedule[i].neighbor.addr_64b[7]    = DEBUG_MOTEID_2;
         break;
   }
   
   //slot 7: DATA to broadcast (for RPL DIOs)
   i = 7;
   schedule_vars.schedule[i].type                          = CELLTYPE_TXRX;
   schedule_vars.schedule[i].shared                        = FALSE;
   schedule_vars.schedule[i].backoff                       = 0;
   schedule_vars.schedule[i].channelOffset                 = 0;
   schedule_vars.schedule[i].neighbor.type                 = ADDR_64B;
   schedule_vars.schedule[i].neighbor.addr_64b[0]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[1]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[2]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[3]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[4]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[5]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[6]          = 0xff;
   schedule_vars.schedule[i].neighbor.addr_64b[7]          = 0xff;
   */
}

bool debugPrint_schedule() {
   debugScheduleRow_t temp;
   schedule_vars.debugPrintRow = (schedule_vars.debugPrintRow+1)%SCHEDULELENGTH;
   temp.row        = schedule_vars.debugPrintRow;
   temp.cellUsage  = schedule_vars.schedule[schedule_vars.debugPrintRow];
   openserial_printStatus(STATUS_SCHEDULE,
                          (uint8_t*)&temp,
                          sizeof(debugScheduleRow_t));
   return TRUE;
}

//=== from IEEE802154E

__monitor cellType_t schedule_getType(uint16_t slotOffset) {
   return schedule_vars.schedule[slotOffset].type;
}

__monitor bool schedule_getOkToSend(uint16_t slotOffset) {
   // decrement backoff of that slot
   if (schedule_vars.schedule[slotOffset].backoff>0) {
      schedule_vars.schedule[slotOffset].backoff--;
   }
   // check whether backoff has hit 0
   if (
       schedule_vars.schedule[slotOffset].shared==FALSE ||
       (
           schedule_vars.schedule[slotOffset].shared==TRUE &&
           schedule_vars.schedule[slotOffset].backoff==0
       )
      ) {
      return TRUE;
   } else {
      return FALSE;
   }
}

__monitor void schedule_getNeighbor(uint16_t slotOffset, open_addr_t* addrToWrite) {
   memcpy(addrToWrite,&schedule_vars.schedule[slotOffset].neighbor,sizeof(open_addr_t));
}

__monitor channelOffset_t schedule_getChannelOffset(uint16_t slotOffset) {
   return schedule_vars.schedule[slotOffset].channelOffset;
}

__monitor void schedule_indicateRx(uint16_t slotOffset, asn_t* asnTimestamp) {
   schedule_vars.schedule[slotOffset].numRx++;
   memcpy(&schedule_vars.schedule[slotOffset].asn, asnTimestamp, sizeof(asn_t));
}

__monitor void schedule_indicateTx(uint16_t slotOffset,
                                   asn_t*   asnTimestamp,
                                   bool     succesfullTx) {
   // update the slot's statistics
   if (schedule_vars.schedule[slotOffset].numTx==0xFF) {
      schedule_vars.schedule[slotOffset].numTx/=2;
      schedule_vars.schedule[slotOffset].numTxACK/=2;
   }
   schedule_vars.schedule[slotOffset].numTx++;
   if (succesfullTx==TRUE) {
      schedule_vars.schedule[slotOffset].numTxACK++;
   }
   memcpy(&schedule_vars.schedule[slotOffset].asn, asnTimestamp, sizeof(asn_t));
   
   // update this slot's backoff parameters
   if (succesfullTx==TRUE) {
      // reset backoffExponent
      schedule_vars.schedule[slotOffset].backoffExponent   = MINBE-1;
      // reset backoff
      schedule_vars.schedule[slotOffset].backoff           = 0;
   } else {
      // increase the backoffExponent
      if (schedule_vars.schedule[slotOffset].backoffExponent<MAXBE) {
         schedule_vars.schedule[slotOffset].backoffExponent++;
      }
      // set the backoff to a random value in [0..2^BE]
      schedule_vars.schedule[slotOffset].backoff =
         random_get16b()%(1<<schedule_vars.schedule[slotOffset].backoffExponent);
   }
}

//=========================== private =========================================