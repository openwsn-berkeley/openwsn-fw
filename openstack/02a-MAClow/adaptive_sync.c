/**
\brief this file is used for the time synchronizatino between different hardware platform

\author Tengfei Chang <tengfei.chang@gmail.com>, January ,2014.
*/
#include "opendefs.h"
#include "adaptive_sync.h"
#include "IEEE802154E.h"
#include "radio.h"
#include "openserial.h"
#include "leds.h"
#include "neighbors.h"
#include "debugpins.h"
#include "packetfunctions.h"
#include "sixtop.h"
#include "scheduler.h"
#include "openqueue.h"
#include "openrandom.h"

//=========================== define ==========================================

#define BASIC_COMPENSATION_THRESHOLD  58

//=========================== type ============================================

//=========================== variables =======================================

adaptive_sync_vars_t adaptive_sync_vars;

//=========================== public ==========================================

/**
\brief initial this module
*/
void adaptive_sync_init() {
   // reset local variables
   memset(&adaptive_sync_vars,0x00,sizeof(adaptive_sync_vars_t));
   
   // default local variables
   adaptive_sync_vars.clockState              = S_NONE;
   adaptive_sync_vars.sumOfTC                 = 0;
   adaptive_sync_vars.compensateThreshold     = BASIC_COMPENSATION_THRESHOLD;
   adaptive_sync_vars.driftChanged            = FALSE;
} 

/**
\brief Calculate how many slots have elapsed since last synchronization.

\param[in] timeCorrection    The time correction being applied.
\param[in] timesource        The address of the neighbor with which I just
   communicated, which triggered a time correction.
*/
void adaptive_sync_indicateTimeCorrection(int16_t timeCorrection, open_addr_t timesource){
   uint8_t array[5];
   
   // stop calculating compensation period when compensateThreshold exceeds KATIMEOUT and drift is not changed
   if(
         adaptive_sync_vars.compensateThreshold  > MAXKAPERIOD &&
         adaptive_sync_vars.driftChanged        == FALSE
      ) {
      if(timeCorrection > LIMITLARGETIMECORRECTION) {
         //once I get a large time correction, it means previous calcluated drift is not accurate yet. The clock drift is changed.
         adaptive_sync_driftChanged();
      }
      return;
   }
   
   // check whether I am synchronized and also check whether it's the same neighbor synchronized to last time?
   if(
         adaptive_sync_vars.driftChanged == FALSE &&
         ieee154e_isSynch()                       &&
         packetfunctions_sameAddress(&timesource, &(adaptive_sync_vars.compensationInfo_vars.neighborID))
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
            sixtop_setKaPeriod(adaptive_sync_vars.compensateThreshold);
            // update oldASN
            ieee154e_getAsn(array);
            adaptive_sync_vars.oldASN.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
            adaptive_sync_vars.oldASN.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
            adaptive_sync_vars.oldASN.byte4                = array[4]; 
         } else {
            // record the timeCorrection, if not calculate.
            adaptive_sync_vars.sumOfTC                    += timeCorrection;
         }
   } else {
      adaptive_sync_vars.compensateThreshold               = BASIC_COMPENSATION_THRESHOLD;
      sixtop_setKaPeriod(adaptive_sync_vars.compensateThreshold);
      
      // when I joined the network, or changed my time parent, reset adaptive_sync relative variables
      adaptive_sync_vars.clockState                        = S_NONE;
      adaptive_sync_vars.elapsedSlots                      = 0;
      adaptive_sync_vars.compensationTimeout               = 0;
      adaptive_sync_vars.compensateTicks                   = 0;
      adaptive_sync_vars.sumOfTC                           = 0;
      
      // update oldASN
      ieee154e_getAsn(array);
      adaptive_sync_vars.oldASN.bytes0and1                 = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
      adaptive_sync_vars.oldASN.bytes2and3                 = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
      adaptive_sync_vars.oldASN.byte4                      = array[4]; 
      
      // record this neighbor as my time source
      memcpy(&(adaptive_sync_vars.compensationInfo_vars.neighborID), &timesource, sizeof(open_addr_t));
   }
}

/**
\brief Calculate the compensation interval, in number of slots.

\param[in] timeCorrection time to be corrected

\returns compensationSlots the number of slots. 
*/
void adaptive_sync_calculateCompensatedSlots(int16_t timeCorrection) {
   bool     isFirstSync;              // is this the first sync after joining network?
   uint16_t totalTimeCorrectionTicks; // how much error in ticks since last synchronization.
   
   if(adaptive_sync_vars.clockState == S_NONE) {
      isFirstSync = TRUE;
   } else {
      isFirstSync = FALSE;
   }
   adaptive_sync_vars.elapsedSlots = ieee154e_asnDiff(&adaptive_sync_vars.oldASN);
   
   if(isFirstSync) {
      if(timeCorrection > 1) {
         adaptive_sync_vars.clockState = S_FASTER;
         adaptive_sync_vars.compensationInfo_vars.compensationSlots       = SYNC_ACCURACY*adaptive_sync_vars.elapsedSlots;
         adaptive_sync_vars.compensationInfo_vars.compensationSlots      /= timeCorrection;
      } else {
         if(timeCorrection < -1) {
            adaptive_sync_vars.clockState = S_SLOWER;
            adaptive_sync_vars.compensationInfo_vars.compensationSlots    = SYNC_ACCURACY*adaptive_sync_vars.elapsedSlots;
            adaptive_sync_vars.compensationInfo_vars.compensationSlots   /= (-timeCorrection);
         } else {
            //timeCorrection = {-1,1}, it's not accurate when timeCorrection belongs to {-1,1}
            //nothing is needed to do with this case.
         }
      }
   } else {
      if(adaptive_sync_vars.clockState == S_SLOWER) {
         totalTimeCorrectionTicks                                    = adaptive_sync_vars.compensateTicks;
         totalTimeCorrectionTicks                                   -= timeCorrection+adaptive_sync_vars.sumOfTC;
      } else {
         totalTimeCorrectionTicks                                    = adaptive_sync_vars.compensateTicks;
         totalTimeCorrectionTicks                                   += timeCorrection+adaptive_sync_vars.sumOfTC;
      }
      if(totalTimeCorrectionTicks == 0) {
         // totalTimeCorrectionTicks should be always positive if drift of clock is constant. if totalTimeCorrectionTIcks become zero, it means the drift changed for some reasons. 
         adaptive_sync_driftChanged();
      } else {
         adaptive_sync_vars.compensationInfo_vars.compensationSlots  = SYNC_ACCURACY*adaptive_sync_vars.elapsedSlots;
         adaptive_sync_vars.compensationInfo_vars.compensationSlots /= totalTimeCorrectionTicks;
      }
   }
   
   adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars.compensationSlots;
}

/**
\brief update compensationTimeout at the beginning of each slot and adjust current slot length when the elapsed slots rearch to compensation interval.

Once compensationTimeout == 0, extend or shorten current slot length for one tick.
*/
void adaptive_sync_countCompensationTimeout() {
   uint16_t newSlotDuration;
   
   newSlotDuration  = TsSlotDuration;
   
   // if clockState is not set yet, don't compensate.
   if (adaptive_sync_vars.clockState == S_NONE) {
      return;
   }
   
   if (adaptive_sync_vars.compensationTimeout == 0) {
      return; // should not happen
   }
   
   adaptive_sync_vars.compensationTimeout--;
   
   // when compensationTimeout, adjust current slot length
   if(adaptive_sync_vars.compensationTimeout == 0) {
      if(adaptive_sync_vars.clockState == S_SLOWER) {
         newSlotDuration                    -= SYNC_ACCURACY;
         adaptive_sync_vars.compensateTicks += SYNC_ACCURACY;
      } else { // clock is fast
         newSlotDuration                    += SYNC_ACCURACY;
         adaptive_sync_vars.compensateTicks += SYNC_ACCURACY;
      }
      // update current slot duration and reload compensationTimeout
      radio_setTimerPeriod(newSlotDuration);
      adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars.compensationSlots;
#ifdef OPENSIM
      debugpins_debug_set();
      debugpins_debug_clr();
#endif
   }
}

/**
\brief update compensationTimeout when compound slots are scheduled and adjust the slot when the elapsed slots rearch to compensation interval(e.g. SERIALRX slots)

\param[in] compoundSlots how many slots will be elapsed before wakeup next time.
*/
void adaptive_sync_countCompensationTimeout_compoundSlots(uint16_t compoundSlots) {
   uint16_t counter;
   uint8_t  compensateTicks;
   uint16_t newSlotDuration;
   
   newSlotDuration  = TsSlotDuration*(compoundSlots+1);
   
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
         adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars.compensationSlots;
      }
      counter--;
   }
   
   // when compensateTicks > 0, I need to do compensation by adjusting current slot length
   if(compensateTicks > 0) {
      if(adaptive_sync_vars.clockState == S_SLOWER) {
         newSlotDuration                    -= compensateTicks*SYNC_ACCURACY;
         adaptive_sync_vars.compensateTicks += compensateTicks*SYNC_ACCURACY;
      } else { // clock is fast
         newSlotDuration                    += compensateTicks*SYNC_ACCURACY;
         adaptive_sync_vars.compensateTicks += compensateTicks * SYNC_ACCURACY;
      }
      radio_setTimerPeriod(newSlotDuration);
#ifdef OPENSIM
      debugpins_debug_set();
      debugpins_debug_clr();
#endif
   }
}

/**
\brief set driftChanged to true.
*/
void adaptive_sync_driftChanged() {
#ifndef NOADAPTIVESYNC
   adaptive_sync_vars.driftChanged = TRUE;
#endif
}
