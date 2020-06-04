#ifndef __USERIALBRIDGE_H
#define __USERIALBRIDGE_H

/**
\addtogroup AppUdp
\{
\addtogroup userialbridge
\{
*/

#include "config.h"
#include "openserial.h"
#include "udp.h"


//=========================== define ==========================================

//=========================== typedef =========================================

#define USERIALBRIDGE_MAXPAYLEN 32

//=========================== variables =======================================

typedef struct {
    uint8_t txbuf[USERIALBRIDGE_MAXPAYLEN];
    uint8_t txbufLen;
} userialbridge_vars_t;

//=========================== prototypes ======================================

void userialbridge_init(void);

void userialbridge_sendDone(OpenQueueEntry_t *msg, owerror_t error);

void userialbridge_triggerData(void);
/**
\}
\}
*/

#endif
