/**
\brief this file is used for the time synchronizatino between different hardware platform

\auther Tengfei Chang <tengfei.chang@gmail.com>, January ,2014.
*/
#include "openwsn.h"
#include "adaptive_sync.h"
#include "IEEE802154E.h"
#include "radio.h"
#include "openserial.h"
#include "leds.h"
#include "neighbors.h"
#include "debugpins.h"
#include "packetfunctions.h"
#include "res.h"
#include "scheduler.h"
#include "openqueue.h"
#include "openrandom.h"

//=========================== define ==========================================   

//=========================== type ============================================

//=========================== variables =======================================

adaptive_sync_vars_t adaptive_sync_vars;

//=========================== prototypes ======================================

void adaptive_sync_timer_cb();
void timer_adaptive_sync_fired();
bool adaptive_sync_driftChanged();

//=========================== public ==========================================

/**
\brief initial this module
*/
void adaptive_sync_init(){  
   memset(&adaptive_sync_vars,0x00,sizeof(adaptive_sync_vars_t));
   adaptive_sync_vars.clockState              = S_NONE;
   adaptive_sync_vars.timerPeriod             = 872+(openrandom_get16b()&0xff);
   adaptive_sync_vars.timerId                 = opentimers_start(
      adaptive_sync_vars.timerPeriod,
      TIMER_PERIODIC,
      TIME_MS,
      adaptive_sync_timer_cb
   );
   adaptive_sync_vars.sumOfTC                 = 0;
   adaptive_sync_vars.compensateThreshold     = 58; // 58 slots == 872 ms
   adaptive_sync_vars.adaptiveProcessStarted  = FALSE;
}

/**
\brief Calculated how many slots have elapsed since last synchronized.

\param[in] timeCorrection time to be corrected
\param[in] syncMethod packet sync or ack sync
\param[in] timesource The address of neighbor.

\return the number of slots
*/
void adaptive_sync_recordLastASN(int16_t timeCorrection, open_addr_t timesource){
   uint8_t array[5];
   
   // stop calculating compensation period when compensateThreshold exceeds KATIMEOUT 
   if(adaptive_sync_vars.compensateThreshold > KATIMEOUT) {
     return;
   }
   
   // check whether I am synchronized and also check whether it's the same neighbor synchronized to last time?
   if(
         ieee154e_isSynch() &&
         packetfunctions_sameAddress(&timesource, &(adaptive_sync_vars.compensationInfo_vars[0].neighborID))
      ) {
        // only calcluate when asnDiff > compensateThresholdThreshold. (this is used for guaranteeing accuracy )
        if(ieee154e_asnDiff(&adaptive_sync_vars.oldASN) > adaptive_sync_vars.compensateThreshold) {
          // calculate compensation interval
          adaptive_sync_calculateCompensatedSlots(timeCorrection);
          // reset compensationtTicks and sumOfTC after calculation
          adaptive_sync_vars.compensateTicks             = 0;
          adaptive_sync_vars.sumOfTC                     = 0;
          // update threshold
          adaptive_sync_vars.compensateThreshold        *= 2;
          // update oldASN
          ieee154e_getAsn(array);
          adaptive_sync_vars.oldASN.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
          adaptive_sync_vars.oldASN.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
          adaptive_sync_vars.oldASN.byte4                = array[4]; 
        } else {
          // record the timeCorrection, if not calculate.
          adaptive_sync_vars.sumOfTC                    += timeCorrection;
        }
        // following condition is used to detect the change of drift.
        if (adaptive_sync_driftChanged() == TRUE) {
          // reset adaptive_sync timer period
          adaptive_sync_vars.timerPeriod                 = 872+(openrandom_get16b()&0xff);
          opentimers_setPeriod(
              adaptive_sync_vars.timerId,
              TIME_MS,
              adaptive_sync_vars.timerPeriod
          );
          adaptive_sync_vars.sumOfTC                     = 0;
          adaptive_sync_vars.compensateThreshold         = 58;
          // update oldASN
          ieee154e_getAsn(array);
          adaptive_sync_vars.oldASN.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
          adaptive_sync_vars.oldASN.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
          adaptive_sync_vars.oldASN.byte4                = array[4]; 
          // start adaptive process
          adaptive_sync_vars.adaptiveProcessStarted      = TRUE;
        }
   } else {
     // when I joined the network, or changed my time parent, reset adaptive_sync relative variables
     adaptive_sync_vars.clockState               = S_NONE;
     adaptive_sync_vars.elapsedSlots             = 0;
     adaptive_sync_vars.compensationTimeout      = 0;
     adaptive_sync_vars.compensateTicks          = 0;
     adaptive_sync_vars.adaptiveProcessStarted   = TRUE;
     // update oldASN
     ieee154e_getAsn(array);
     adaptive_sync_vars.oldASN.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
     adaptive_sync_vars.oldASN.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
     adaptive_sync_vars.oldASN.byte4                = array[4]; 
     // record this neighbor as my time source
     memcpy(&(adaptive_sync_vars.compensationInfo_vars[0].neighborID), &timesource, sizeof(open_addr_t));
   }
}

/**
\brief Calculate the compensation interval, in number of slots.

\param[in] timeCorrection time to be corrected
\param[in] syncMethod syncrhonized using packet or ack.

\returns compensationSlots the number of slots. 
*/
void adaptive_sync_calculateCompensatedSlots(int16_t timeCorrection) {
   bool isFirstSync; // is this the first sync after joining network?
   if(adaptive_sync_vars.clockState == S_NONE) {
     isFirstSync = TRUE;
   } else {
     isFirstSync = FALSE;
   }
   adaptive_sync_vars.elapsedSlots = ieee154e_asnDiff(&adaptive_sync_vars.oldASN);
   
   if(isFirstSync) {
     if(timeCorrection > 0) {
       adaptive_sync_vars.clockState = S_FASTER;
       adaptive_sync_vars.compensationInfo_vars[0].compensationSlots = 2*adaptive_sync_vars.elapsedSlots/timeCorrection;
     } else {
       if(timeCorrection < 0) {
         adaptive_sync_vars.clockState = S_SLOWER;
         adaptive_sync_vars.compensationInfo_vars[0].compensationSlots = 2*adaptive_sync_vars.elapsedSlots/(-timeCorrection);
       } else {
         //timeCorrection == 0, no drift is detected
       }
     }
   } else {
     if(adaptive_sync_vars.clockState == S_SLOWER) {
       adaptive_sync_vars.compensationInfo_vars[0].compensationSlots = 2*adaptive_sync_vars.elapsedSlots/(adaptive_sync_vars.compensateTicks-(timeCorrection+adaptive_sync_vars.sumOfTC));
     } else {
       adaptive_sync_vars.compensationInfo_vars[0].compensationSlots = 2*adaptive_sync_vars.elapsedSlots/(adaptive_sync_vars.compensateTicks+(timeCorrection+adaptive_sync_vars.sumOfTC));
     }
   }
     
   adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
}

/**
\brief Counts compensationTimeout at each slot by minus one.

Once compensationTimeout == 0, adjust correct slot length.
*/
void adaptive_sync_countCompensationTimeout() {
   // if clockState is not set yet, don't compensate.
   if(adaptive_sync_vars.clockState == S_NONE) {
     return;
   }
   if(adaptive_sync_vars.compensationTimeout == 0) {
     return; // should not happen
   }
   adaptive_sync_vars.compensationTimeout--;
   // when compensationTimeout, adjust current slot length
   if(adaptive_sync_vars.compensationTimeout == 0) {
     if(adaptive_sync_vars.clockState == S_SLOWER) {
       radio_setTimerPeriod(TsSlotDuration-2);
       adaptive_sync_vars.compensateTicks += 2;
#ifdef OPENSIM
   debugpins_debug_set();
   debugpins_debug_clr();
#endif
     } else { // clock is fast
       radio_setTimerPeriod(TsSlotDuration+2);
       adaptive_sync_vars.compensateTicks += 2;
#ifdef OPENSIM
   debugpins_debug_set();
   debugpins_debug_clr();
#endif
     }
     // reload compensationTimeout
     adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
//     GPIOC->ODR ^= 0X1000;
   }
}

/**
\brief Count compensationTimeout at each slot by minus one when compundSlots are scheduled (e.g. SERIALRX slots)

\param[in] compoundSlots how many slots will be elapsed before wakeup next time.
*/
void adaptive_sync_countCompensationTimeout_compoundSlots(uint16_t compoundSlots) {
   uint16_t counter;
   uint8_t  compensateTicks;
 
   // if clockState is not set yet, don't compensate.
   if(adaptive_sync_vars.clockState == S_NONE) {
     return;
   }
   if(adaptive_sync_vars.compensationTimeout == 0) {
     return; // should not happen
   }
   if(compoundSlots < 1) {
     // return, if this is not a compoundSlot
     return;
   }
   counter          = compoundSlots; 
   compensateTicks  = 0;
   while(counter > 0) {
      adaptive_sync_vars.compensationTimeout--;
      if (adaptive_sync_vars.compensationTimeout == 0) {
         compensateTicks += 1;
         adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
//         GPIOC->ODR ^= 0X1000;
      }
      counter--;
   }
   
   // when compensateTicks > 0, I need to do compensation by adjusting current slot length
   if(compensateTicks > 0) {
     if(adaptive_sync_vars.clockState == S_SLOWER) {
       radio_setTimerPeriod(TsSlotDuration*(compoundSlots+1)-compensateTicks*2);
       adaptive_sync_vars.compensateTicks += compensateTicks*2;
#ifdef OPENSIM
   debugpins_debug_set();
   debugpins_debug_clr();
#endif
     } else { // clock is fast
       radio_setTimerPeriod(TsSlotDuration*(compoundSlots+1)+compensateTicks*2);
       adaptive_sync_vars.compensateTicks += compensateTicks * 2;
#ifdef OPENSIM
   debugpins_debug_set();
   debugpins_debug_clr();
#endif
     }
   }
}

//=========================== private =========================================

void adaptive_sync_timer_cb() {
   scheduler_push_task(timer_adaptive_sync_fired,TASKPRIO_ADAPTIVE_SYNC);
}

/**
\brief Timer handlers which triggers scheduling a packet to calculate drift.

This function is called in task context by the scheduler after the
adaptive_sync timer has fired. This timer is set to fire in one second after
mote joined network.
*/
void timer_adaptive_sync_fired() {
   OpenQueueEntry_t* pkt;
   open_addr_t       neighAddr;

   if(adaptive_sync_vars.adaptiveProcessStarted == FALSE) {
      return;
   }
//   GPIOC->ODR ^=  0X0100;
    // re-schedule a long term packet to synchronization
   if(adaptive_sync_vars.timerPeriod*2 < KATIMEOUT*15) {   
     adaptive_sync_vars.timerPeriod             = adaptive_sync_vars.timerPeriod*2;
   } else {
     adaptive_sync_vars.timerPeriod             = 872+(openrandom_get16b()&0xff);
     adaptive_sync_vars.adaptiveProcessStarted  = FALSE;
   }
    // re-schedule one packet for sending
   opentimers_setPeriod(
       adaptive_sync_vars.timerId,
       TIME_MS,
       adaptive_sync_vars.timerPeriod
   );
   
   memset(&neighAddr,0x00,sizeof(open_addr_t));
   
   if(neighbors_getPreferredParentEui64(&neighAddr)==FALSE) {
      // can't find preferredParent
      return;
   }
   
   // if I get here, I will send a packet
   
   // get a free packet buffer
   pkt = openqueue_getFreePacketBuffer(COMPONENT_RES);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_RES,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)1,
         (errorparameter_t)0
      );
      // no free packet buffer
      return;
   }
   
   // declare the packet belong to res. (this packet practically is ka packet, just being sent early)
   pkt->creator = COMPONENT_RES;
   pkt->owner   = COMPONENT_RES;
   
   // some l2 information about this packet
   pkt->l2_frameType = IEEE154_TYPE_DATA;
   memcpy(&(pkt->l2_nextORpreviousHop),&(neighAddr),sizeof(open_addr_t));
   
   res_send(pkt);
   
}

bool adaptive_sync_driftChanged() {
   bool driftChanged = FALSE; 
   // add some methods here to determine whether the drift is changed.
   
   return driftChanged;
}