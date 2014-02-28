#ifndef __ADAPTIVE_SYNC_H
#define __ADAPTIVE_SYNC_H

#include "board_info.h"
#include "openwsn.h"
/**
\addtogroup MAClow
\{
\addtogroup adpativesync
\{

\brief adaptive sync module

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, Janaury 2014.
*/

//=========================== define ==========================================

typedef enum {
  S_NONE   = 0x00,
  S_FASTER = 0x01,
  S_SLOWER = 0x02,
}adaptive_sync_state_t;

typedef enum {
  S_PACKET_SYNC = 0x00,
  S_ACK_SYNC    = 0x01,
}adaptive_sync_methods_t;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== prototypes ======================================

void adaptive_sync_init();
void adaptive_sync_recordLastASN(int16_t timeCorrection, uint8_t syncMethod, open_addr_t timesource);
void adaptive_sync_calculateCompensatedSlots(int16_t timeCorrection, uint8_t syncMethod);

void adaptive_sync_countCompensationTimeout();
void adaptive_sync_countCompensationTimeout_compoundSlots(uint16_t compoundSlots);

/**
\}
\}
*/
#endif