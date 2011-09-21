#ifndef __IEEE154ETIMER_H
#define __IEEE154ETIMER_H

/**
\addtogroup MAClow
\{
\addtogroup ieee154etimer
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

// this is a workaround from the fact that the interrupt pin for the radio is
// not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME() TACCTL2 |=  CCIS0;     \
                       TACCTL2 &= ~CCIS0;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void     ieee154etimer_init();
void     ieee154etimer_schedule(uint16_t offset);
void     ieee154etimer_cancel();
uint16_t ieee154etimer_getCapturedTime();

/**
\}
\}
*/

#endif
