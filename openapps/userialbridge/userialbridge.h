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

#define USERIALBRIDGE_MAXPAYLEN 8

//=========================== variables =======================================

typedef struct {
   uint8_t  txbuf[USERIALBRIDGE_MAXPAYLEN];
   uint8_t  txbufLen;
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
