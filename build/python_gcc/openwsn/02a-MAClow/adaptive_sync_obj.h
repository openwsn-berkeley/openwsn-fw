/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:54.575116.
*/
#ifndef __ADAPTIVE_SYNC_H
#define __ADAPTIVE_SYNC_H

#include "board_obj.h"
#include "openwsn_obj.h"
#include "opentimers_obj.h"
/**
\addtogroup MAClow
\{
\addtogroup adpativesync
\{

\brief adaptive sync module

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, January 2014.
*/

//=========================== define ==========================================

typedef enum {
   S_NONE          = 0x00,
   S_FASTER        = 0x01,
   S_SLOWER        = 0x02,
} adaptive_sync_state_t;

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   open_addr_t               neighborID;
   uint16_t                  compensationSlots;       // compensation interval, in slots 
} compensationInfo_t;

typedef struct {
   adaptive_sync_state_t     clockState;
   PORT_RADIOTIMER_WIDTH     elapsedSlots;            // since last synchronizatino, this number of slots have elapsed.
   uint16_t                  compensationTimeout;     // decrease one every slot, when it reach zero, adjust currectly slot length by 2 tick(60us). 
   uint16_t                  compensateTicks;         // record how many ticks  are compensated 
   asn_t                     oldASN;                  // the asn when synchronized previous time
   compensationInfo_t        compensationInfo_vars;   // keep each time soures' compensation informatio( should be 9, since there would be more timesources)
   int16_t                   sumOfTC;                 // record the sum of ticks between two time point which need to calculate compensation period.
   uint16_t                  compensateThreshold;     // number of slots. calculate the compensation period only when elapsed slot number is greater than this threshold 
   bool                      driftChanged;            // drift is changed or not.
} adaptive_sync_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void adaptive_sync_init(OpenMote* self);
void adaptive_sync_preprocess(OpenMote* self, int16_t timeCorrection, open_addr_t timesource);
void adaptive_sync_calculateCompensatedSlots(OpenMote* self, int16_t timeCorrection);

void adaptive_sync_countCompensationTimeout(OpenMote* self);
void adaptive_sync_countCompensationTimeout_compoundSlots(OpenMote* self, uint16_t compoundSlots);
void adaptive_sync_driftChanged(OpenMote* self);

/**
\}
\}
*/
#endif

