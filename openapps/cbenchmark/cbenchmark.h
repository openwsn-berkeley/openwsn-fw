#ifndef __CBENCHMARK_H
#define __CBENCHMARK_H

/**
\addtogroup AppUdp
\{
\addtogroup cbenchmark
\{
*/
#include "opendefs.h"
//=========================== define ==========================================
#define CBENCHMARK_PACKETTOKEN_LEN 5
//=========================== typedef =========================================

typedef struct {
    coap_resource_desc_t     desc;
    opentimers_id_t          timerId;
    uint8_t                  noResponse;
} cbenchmark_vars_t;

typedef struct {
    open_addr_t              dest;
    bool                     con;
    uint8_t                  numPackets;
    uint8_t                  token[CBENCHMARK_PACKETTOKEN_LEN];
    uint8_t                  payloadLen;
} cbenchmark_sendPacket_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cbenchmark_init(void);

/**
\}
\}
*/

#endif
