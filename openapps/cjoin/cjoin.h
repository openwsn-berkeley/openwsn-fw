#ifndef __CJOIN_H
#define __CJOIN_H

/**
\addtogroup AppUdp
\{
\addtogroup cjoin
\{
*/
#include "opencoap.h"
//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   opentimer_id_t       startupTimerId;
   opentimer_id_t       retransmissionTimerId;
   uint8_t              lastPayload;
   bool                 isJoined;
} cjoin_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cjoin_init(void);
void cjoin_schedule(void);
bool cjoin_getIsJoined(void);
void cjoin_setIsJoined(bool newValue);

/**
\}
\}
*/

#endif
