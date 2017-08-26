#ifndef __TOHLONE_H
#define __TOHLONE_H

/**
\addtogroup AppTcp
\{
\addtogroup tohlone
\{
*/

#include "opentcp.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t*    pkt;
   bool                 sending;
   uint16_t             httpChunk;
   uint8_t              getRequest[TCP_DEFAULT_WINDOW_SIZE];
} tohlone_vars_t;

//=========================== prototypes ======================================

void tohlone_init(void);
bool tohlone_shouldIlisten(void);
void tohlone_receive(OpenQueueEntry_t* msg);
void tohlone_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void tohlone_connectDone(owerror_t error);
bool tohlone_debugPrint(void);

/**
\}
\}
*/

#endif
