#ifndef __CJOIN_H
#define __CJOIN_H

/**
\addtogroup AppUdp
\{
\addtogroup cjoin
\{
*/
#include "opendefs.h"
//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
    coap_resource_desc_t desc;
    opentimer_id_t           startupTimerId;
    opentimer_id_t           retransmissionTimerId;
    bool                     isJoined;
    asn_t                    joinAsn;
    uint8_t                  joinKey[16];
} cjoin_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cjoin_init(void);
void cjoin_schedule(void);
bool cjoin_getIsJoined(void);
void cjoin_setIsJoined(bool newValue);
void cjoin_setJoinKey(uint8_t *key, uint8_t len);
bool debugPrint_joined(void);



/**
\}
\}
*/

#endif
