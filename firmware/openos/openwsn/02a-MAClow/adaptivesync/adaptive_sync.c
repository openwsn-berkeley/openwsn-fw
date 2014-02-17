/**
\brief this file is used for the time synchronizatino between different hardware platform

\auther Tengfei Chang <tengfei.chang@gmail.com>, Janauray ,2014.
*/
#include "adaptive_sync.h"
#include "IEEE802154E.h"
#include "radio.h"
#include "openserial.h"
#include "leds.h"
#include "neighbors.h"
#include "debugpins.h"
#include "packetfunctions.h"

//=========================== define ==========================================

//=========================== type ============================================

typedef struct {
  open_addr_t neighborID;
  uint32_t     compensationSlots; // compensation interval, counted by slots 
}compensationInfo_t;

typedef struct {
  adaptive_sync_state_t clockState;
  PORT_RADIOTIMER_WIDTH lapsedSlots;         // since last synchronizatino, this number of slots have  lapsed.
  uint16_t              compensationTimeout; // decrease one every slot, when it reach zero, adjust currectly slot length by 2 tick(60us). 
  uint16_t              compensateTicks;     // record how many ticks  are compensated 
  uint16_t              timeCorrectionRecord;// record the sum of historical timeCorrection
  asn_t                 oldASN;              // the asn when synchronized previous time
  compensationInfo_t    compensationInfo_vars[9]; //keep each time soures' compensation information
} adaptive_sync_t;

//=========================== variables =======================================

adaptive_sync_t adaptive_sync_vars;

//=========================== prototypes ======================================

//=========================== prototypes ======================================

/**
\brief initial this module
*/
void adaptive_sync_init(){
  memset(&adaptive_sync_vars,0,sizeof(adaptive_sync_t));
}

/**
\brief calculated how many slots have lapsed since last synchronized.

\param[in] timesource The address of neighbor.

\param[in] timeCorrection time to be corrected

\param[in] syncMethod packet sync or ack sync

\returns the number of slots
*/
void adaptive_sync_recordLastASN(int16_t timeCorrection, uint8_t syncMethod, open_addr_t timesource){
    uint8_t array[5];
    ieee154e_getAsn(array);
    // check whether I am synchronized and also check whether it's the same neighbor synchronized to last time?
    if(ieee154e_isSynch() && packetfunctions_sameAddress(&timesource, &(adaptive_sync_vars.compensationInfo_vars[0].neighborID)))
    {
      // reset compensationtTicks when synchronization happened
      adaptive_sync_vars.compensateTicks = 0;
      // if I am synchronized, only calculated compensation interval when timeCorrection excess LIMITLARGETIMECORRECTION
      if(timeCorrection > LIMITLARGETIMECORRECTION || timeCorrection < -LIMITLARGETIMECORRECTION)
      {
        adaptive_sync_calculateCompensatedSlots(timeCorrection, syncMethod);
        adaptive_sync_vars.timeCorrectionRecord = 0;
      }
      else
      {
        adaptive_sync_vars.timeCorrectionRecord += timeCorrection;
        return;
      }
    }
    else
    {
      if(packetfunctions_sameAddress(&timesource, &(adaptive_sync_vars.compensationInfo_vars[0].neighborID)) == FALSE && ieee154e_isSynch() == TRUE)
        leds_debug_toggle();
      // this is the first time for synchronizing to current neighbor, reset variables, 
      memset(&adaptive_sync_vars,0,sizeof(adaptive_sync_t));
    }
    // update oldASN variable by currect asn
    memcpy(&(adaptive_sync_vars.oldASN.bytes0and1), &array[0], sizeof(uint16_t));
    memcpy(&(adaptive_sync_vars.oldASN.bytes2and3), &array[2], sizeof(uint16_t));
    memcpy(&(adaptive_sync_vars.oldASN.byte4), &array[4], sizeof(uint8_t));
    
    memcpy(&(adaptive_sync_vars.compensationInfo_vars[0].neighborID), &timesource, sizeof(open_addr_t));
}

/**
\brief calcluated the compensation interval, counted by number of slots.

\para[in] address The address of neighbor.

\returns compensationSlots the number of slots. 
*/
void adaptive_sync_calculateCompensatedSlots(int16_t timeCorrection, uint8_t syncMethod){
    uint16_t TC;
    adaptive_sync_vars.lapsedSlots = ieee154e_asnDiff(&adaptive_sync_vars.oldASN);
    // determine the clock's state: faster or slower?
    switch(syncMethod)
    {
    case S_PACKET_SYNC:
      if(timeCorrection > 0)
      {
        if(adaptive_sync_vars.clockState == S_NONE)
          adaptive_sync_vars.clockState = S_FASTER;
      }
      else
      {
        TC = -timeCorrection;
        if(adaptive_sync_vars.clockState == S_NONE)
          adaptive_sync_vars.clockState = S_SLOWER;
      }
      break;
    case S_ACK_SYNC:
      if(timeCorrection > 0)
      {
        if(adaptive_sync_vars.clockState == S_NONE)
          adaptive_sync_vars.clockState = S_SLOWER;
      }
      else
      {
        TC = -timeCorrection;
        if(adaptive_sync_vars.clockState == S_NONE)
          adaptive_sync_vars.clockState = S_FASTER;
      }
      break;
    default:
      while(1); // should not reach here
    }// calculate the compensation interval, uint: slots/x ticks. 2ticks is only the case of openmotestm platform, usually it should be one
    adaptive_sync_vars.compensationInfo_vars[0].compensationSlots = COMPENSATE_ACCURACY * adaptive_sync_vars.lapsedSlots / (TC + adaptive_sync_vars.compensateTicks + adaptive_sync_vars.timeCorrectionRecord);
    adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
}
/**
\brief minus compensationTimeout at each slot
*/
void adaptive_sync_countCompensationTimeout() {
  // if clockState is not set yet, don't compensate.
  if(adaptive_sync_vars.clockState == S_NONE) return;

    adaptive_sync_vars.compensationTimeout--;
    // when compensationTimeout, adjust current slot length
    if(adaptive_sync_vars.compensationTimeout == 0)
    {
      switch(adaptive_sync_vars.clockState)
      {
      case S_SLOWER:
        radio_setTimerPeriod(TsSlotDuration - COMPENSATE_ACCURACY);
        adaptive_sync_vars.compensateTicks += COMPENSATE_ACCURACY;
        break;
      case S_FASTER:
        radio_setTimerPeriod(TsSlotDuration + COMPENSATE_ACCURACY);
        adaptive_sync_vars.compensateTicks += COMPENSATE_ACCURACY;
        break;
      case S_NONE:
        while(1);
      default:
        while(1);
      }
      // tf: for debugging
      GPIOC->ODR ^= 0X1000;
      // reload compensationTimeout
      adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
    }
}
/**
\brief minus compensationTimeout at each slot, when compundSlots are scheduled.(e.g. SERIALRX slots)
*/
void adaptive_sync_countCompensationTimeout_compoundSlots(uint16_t compoundSlots)
{
  uint8_t  temp_quotient;
  uint16_t temp_reminder, restTimeout;
  // the rest slots to trigger next compensation
  restTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots - adaptive_sync_vars.compensationTimeout;
    // if clockState is not set yet, don't compensate.
  if(adaptive_sync_vars.clockState == S_NONE) return;
  // calculate necessary variables for compensation
  temp_quotient = (compoundSlots + restTimeout) / adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
  temp_reminder = (compoundSlots + restTimeout) % adaptive_sync_vars.compensationInfo_vars[0].compensationSlots;
  
    // when quotient > 0, I need to do compensation by adjusting current slot length
    if(temp_quotient > 0)
    {
      switch(adaptive_sync_vars.clockState)
      {
      case S_SLOWER:
        radio_setTimerPeriod(TsSlotDuration*(compoundSlots + 1) - temp_quotient * COMPENSATE_ACCURACY);
        adaptive_sync_vars.compensateTicks += temp_quotient * COMPENSATE_ACCURACY;
        break;
      case S_FASTER:
        radio_setTimerPeriod(TsSlotDuration*(compoundSlots + 1) + temp_quotient * COMPENSATE_ACCURACY);
        adaptive_sync_vars.compensateTicks += temp_quotient * COMPENSATE_ACCURACY;
        break;
      case S_NONE:
        while(1);
      default:
        while(1);
      }
      for(uint8_t i=0;i<temp_quotient;i++)
        // tf: for debugging
        GPIOC->ODR ^= 0X1000;
    }
    // reload compensationTimeout
    adaptive_sync_vars.compensationTimeout = adaptive_sync_vars.compensationInfo_vars[0].compensationSlots - temp_reminder;    
}