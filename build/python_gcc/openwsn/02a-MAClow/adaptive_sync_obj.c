/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:20.270736.
*/
/**
\brief this file is used for the time synchronizatino between different hardware platform

\author Tengfei Chang <tengfei.chang@gmail.com>, January ,2014.
*/
#include "openwsn_obj.h"
#include "adaptive_sync_obj.h"
#include "IEEE802154E_obj.h"
#include "radio_obj.h"
#include "openserial_obj.h"
#include "leds_obj.h"
#include "neighbors_obj.h"
#include "debugpins_obj.h"
#include "packetfunctions_obj.h"
#include "sixtop_obj.h"
#include "scheduler_obj.h"
#include "openqueue_obj.h"
#include "openrandom_obj.h"

//=========================== define ==========================================

#define BASIC_COMPENSATION_THRESHOLD  58

//=========================== type ============================================

//=========================== variables =======================================

// declaration of global variable _adaptive_sync_vars_ removed during objectification.

//=========================== public ==========================================

/**
\brief initial this module
*/
void adaptive_sync_init(OpenMote* self){  
   memset(&(self->adaptive_sync_vars),0x00,sizeof(adaptive_sync_vars_t));
   (self->adaptive_sync_vars).clockState              = S_NONE;
   (self->adaptive_sync_vars).sumOfTC                 = 0;
   (self->adaptive_sync_vars).compensateThreshold     = BASIC_COMPENSATION_THRESHOLD;
   (self->adaptive_sync_vars).driftChanged            = FALSE;
} 

/**
\brief Calculated how many slots have elapsed since last synchronized.

\param[in] timeCorrection time to be corrected
\param[in] timesource The address of neighbor.

\return the number of slots
*/
void adaptive_sync_preprocess(OpenMote* self, int16_t timeCorrection, open_addr_t timesource){
   uint8_t array[5];
   
   // stop calculating compensation period when compensateThreshold exceeds KATIMEOUT and drift is not changed
   if(
         (self->adaptive_sync_vars).compensateThreshold  > MAXKAPERIOD &&
         (self->adaptive_sync_vars).driftChanged        == FALSE
      ) {
     if(timeCorrection > LIMITLARGETIMECORRECTION) {
       //once I get a large time correction, it means previous calcluated drift is not accurate yet. The clock drift is changed.
 adaptive_sync_driftChanged(self);
     }
     return;
   }
   
   // check whether I am synchronized and also check whether it's the same neighbor synchronized to last time?
   if(
         (self->adaptive_sync_vars).driftChanged == FALSE &&
 ieee154e_isSynch(self)                       &&
 packetfunctions_sameAddress(self, &timesource, &((self->adaptive_sync_vars).compensationInfo_vars.neighborID))
      ) {
        // only calcluate when asnDiff > compensateThresholdThreshold. (this is used for guaranteeing accuracy )
        if( ieee154e_asnDiff(self, &(self->adaptive_sync_vars).oldASN) > (self->adaptive_sync_vars).compensateThreshold) {
          // calculate compensation interval
 adaptive_sync_calculateCompensatedSlots(self, timeCorrection);
          // reset compensationtTicks and sumOfTC after calculation
          (self->adaptive_sync_vars).compensateTicks             = 0;
          (self->adaptive_sync_vars).sumOfTC                     = 0;
          // update threshold
          (self->adaptive_sync_vars).compensateThreshold        *= 2;
 sixtop_setKaPeriod(self, (self->adaptive_sync_vars).compensateThreshold);
          // update oldASN
 ieee154e_getAsn(self, array);
          (self->adaptive_sync_vars).oldASN.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
          (self->adaptive_sync_vars).oldASN.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
          (self->adaptive_sync_vars).oldASN.byte4                = array[4]; 
        } else {
          // record the timeCorrection, if not calculate.
          (self->adaptive_sync_vars).sumOfTC                    += timeCorrection;
        }
   } else {
     (self->adaptive_sync_vars).compensateThreshold      = BASIC_COMPENSATION_THRESHOLD;
 sixtop_setKaPeriod(self, (self->adaptive_sync_vars).compensateThreshold);
     // when I joined the network, or changed my time parent, reset adaptive_sync relative variables
     (self->adaptive_sync_vars).clockState               = S_NONE;
     (self->adaptive_sync_vars).elapsedSlots             = 0;
     (self->adaptive_sync_vars).compensationTimeout      = 0;
     (self->adaptive_sync_vars).compensateTicks          = 0;
     (self->adaptive_sync_vars).sumOfTC                  = 0;
     // update oldASN
 ieee154e_getAsn(self, array);
     (self->adaptive_sync_vars).oldASN.bytes0and1        = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
     (self->adaptive_sync_vars).oldASN.bytes2and3        = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
     (self->adaptive_sync_vars).oldASN.byte4             = array[4]; 
     // record this neighbor as my time source
     memcpy(&((self->adaptive_sync_vars).compensationInfo_vars.neighborID), &timesource, sizeof(open_addr_t));
   }
}

/**
\brief Calculate the compensation interval, in number of slots.

\param[in] timeCorrection time to be corrected

\returns compensationSlots the number of slots. 
*/
void adaptive_sync_calculateCompensatedSlots(OpenMote* self, int16_t timeCorrection) {
   bool     isFirstSync;              // is this the first sync after joining network?
   uint16_t totalTimeCorrectionTicks; // how much error in ticks since last synchronization.
   if((self->adaptive_sync_vars).clockState == S_NONE) {
     isFirstSync = TRUE;
   } else {
     isFirstSync = FALSE;
   }
   (self->adaptive_sync_vars).elapsedSlots = ieee154e_asnDiff(self, &(self->adaptive_sync_vars).oldASN);
   
   if(isFirstSync) {
     if(timeCorrection > 1) {
       (self->adaptive_sync_vars).clockState = S_FASTER;
       (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots  = SYNC_ACCURACY*(self->adaptive_sync_vars).elapsedSlots;
       (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots /= timeCorrection;
     } else {
       if(timeCorrection < -1) {
         (self->adaptive_sync_vars).clockState = S_SLOWER;
         (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots  = SYNC_ACCURACY*(self->adaptive_sync_vars).elapsedSlots;
         (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots /= (-timeCorrection);
       } else {
         //timeCorrection = {-1,1}, it's not accurate when timeCorrection belongs to {-1,1}
         //nothing is needed to do with this case.
       }
     }
   } else {
     if((self->adaptive_sync_vars).clockState == S_SLOWER) {
       totalTimeCorrectionTicks                                       = (self->adaptive_sync_vars).compensateTicks;
       totalTimeCorrectionTicks                                      -= timeCorrection+(self->adaptive_sync_vars).sumOfTC;
     } else {
       totalTimeCorrectionTicks                                       = (self->adaptive_sync_vars).compensateTicks;
       totalTimeCorrectionTicks                                      += timeCorrection+(self->adaptive_sync_vars).sumOfTC;
     }
     if(totalTimeCorrectionTicks == 0) {
       // totalTimeCorrectionTicks should be always positive if drift of clock is constant. if totalTimeCorrectionTIcks become zero, it means the drift changed for some reasons. 
 adaptive_sync_driftChanged(self);
     } else {
       (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots  = SYNC_ACCURACY*(self->adaptive_sync_vars).elapsedSlots;
       (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots /= totalTimeCorrectionTicks;
     }
   }
     
   (self->adaptive_sync_vars).compensationTimeout = (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots;
}

/**
\brief update compensationTimeout at the beginning of each slot and adjust current slot length when the elapsed slots rearch to compensation interval.

Once compensationTimeout == 0, extend or shorten current slot length for one tick.
*/
void adaptive_sync_countCompensationTimeout(OpenMote* self) {
   uint16_t newSlotDuration;
   newSlotDuration  = TsSlotDuration;
   // if clockState is not set yet, don't compensate.
   if((self->adaptive_sync_vars).clockState == S_NONE) {
     return;
   }
   if((self->adaptive_sync_vars).compensationTimeout == 0) {
     return; // should not happen
   }
   (self->adaptive_sync_vars).compensationTimeout--;
   // when compensationTimeout, adjust current slot length
   if((self->adaptive_sync_vars).compensationTimeout == 0) {
     if((self->adaptive_sync_vars).clockState == S_SLOWER) {
       newSlotDuration                    -= SYNC_ACCURACY;
       (self->adaptive_sync_vars).compensateTicks += SYNC_ACCURACY;
     } else { // clock is fast
       newSlotDuration                    += SYNC_ACCURACY;
       (self->adaptive_sync_vars).compensateTicks += SYNC_ACCURACY;
     }
     // update current slot duration and reload compensationTimeout
 radio_setTimerPeriod(self, newSlotDuration);
     (self->adaptive_sync_vars).compensationTimeout = (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots;
#ifdef OPENSIM
 debugpins_debug_set(self);
 debugpins_debug_clr(self);
#endif
   }
}

/**
\brief update compensationTimeout when compound slots are scheduled and adjust the slot when the elapsed slots rearch to compensation interval(e.g. SERIALRX slots)

\param[in] compoundSlots how many slots will be elapsed before wakeup next time.
*/
void adaptive_sync_countCompensationTimeout_compoundSlots(OpenMote* self, uint16_t compoundSlots) {
   uint16_t counter;
   uint8_t  compensateTicks;
   uint16_t newSlotDuration;
   newSlotDuration  = TsSlotDuration*(compoundSlots+1);
   // if clockState is not set yet, don't compensate.
   if((self->adaptive_sync_vars).clockState == S_NONE) {
     return;
   }
   if((self->adaptive_sync_vars).compensationTimeout == 0) {
     return; // should not happen
   }
   if(compoundSlots < 1) {
     // return, if this is not a compoundSlot
     return;
   }
   counter          = compoundSlots; 
   compensateTicks  = 0;
   while(counter > 0) {
      (self->adaptive_sync_vars).compensationTimeout--;
      if ((self->adaptive_sync_vars).compensationTimeout == 0) {
         compensateTicks += 1;
         (self->adaptive_sync_vars).compensationTimeout = (self->adaptive_sync_vars).compensationInfo_vars.compensationSlots;
      }
      counter--;
   }
   
   // when compensateTicks > 0, I need to do compensation by adjusting current slot length
   if(compensateTicks > 0) {
     if((self->adaptive_sync_vars).clockState == S_SLOWER) {
       newSlotDuration                    -= compensateTicks*SYNC_ACCURACY;
       (self->adaptive_sync_vars).compensateTicks += compensateTicks*SYNC_ACCURACY;
     } else { // clock is fast
       newSlotDuration                    += compensateTicks*SYNC_ACCURACY;
       (self->adaptive_sync_vars).compensateTicks += compensateTicks * SYNC_ACCURACY;
     }
 radio_setTimerPeriod(self, newSlotDuration);
#ifdef OPENSIM
 debugpins_debug_set(self);
 debugpins_debug_clr(self);
#endif
   }
}

/**
\brief set driftChanged to true.
*/
void adaptive_sync_driftChanged(OpenMote* self) {
#ifndef NOADAPTIVESYNC
   (self->adaptive_sync_vars).driftChanged = TRUE;
#endif
}

