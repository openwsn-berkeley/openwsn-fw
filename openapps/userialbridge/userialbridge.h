#ifndef __USERIALBRIDGE_H
#define __USERIALBRIDGE_H

/**
\addtogroup AppUdp
\{
\addtogroup userialbridge
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
   uint16_t             counter;  ///< incrementing counter which is written into the packet
} userialbridge_vars_t;

//=========================== prototypes ======================================

void userialbridge_init(void);
void userialbridge_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void userialbridge_receive(OpenQueueEntry_t* msg);
void userialbridge_triggerData(uint8_t* buf, uint8_t bufLen);
/**
\}
\}
*/

#endif
