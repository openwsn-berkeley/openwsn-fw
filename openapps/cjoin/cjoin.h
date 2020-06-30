#ifndef OPENWSN_CJOIN_H
#define OPENWSN_CJOIN_H

/**
\addtogroup AppUdp
\{
\addtogroup cjoin
\{
*/

#include "opendefs.h"
#include "coap.h"
#include "oscore.h"
//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
    coap_resource_desc_t desc;
    opentimers_id_t timerId;
    bool isJoined;
    oscore_security_context_t context;
    uint8_t medType;
} cjoin_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cjoin_init(void);

void cjoin_schedule(void);

bool cjoin_getIsJoined(void);

void cjoin_setIsJoined(bool newValue);

void cjoin_setJoinKey(uint8_t *key, uint8_t len);

/**
\}
\}
*/

#endif /* OPENWSN_CJOIN_H */
