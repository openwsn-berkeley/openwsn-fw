#ifndef __OHLONE_H
#define __OHLONE_H

/**
\addtogroup AppTcp
\{
\addtogroup ohlone
\{
*/

#include "opentcp.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t*    pkt;
   BOOL                 sending;
   uint16_t             httpChunk;
   uint8_t              getRequest[TCP_DEFAULT_WINDOW_SIZE];
} ohlone_vars_t;

//=========================== prototypes ======================================

void ohlone_init();
BOOL ohlone_shouldIlisten();
void ohlone_receive(OpenQueueEntry_t* msg);
void ohlone_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void ohlone_connectDone(owerror_t error);
BOOL ohlone_debugPrint();

/**
\}
\}
*/

#endif
