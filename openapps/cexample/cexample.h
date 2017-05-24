#ifndef __CEXAMPLE_H
#define __CEXAMPLE_H

/**
\addtogroup AppUdp
\{
\addtogroup cexample
\{
*/
#include "opencoap.h"

//=========================== define ==========================================


//=========================== typedef =========================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t       timerId;
   track_t              track;
   uint32_t             seqnum;  //uniquely identifies this packet
} cexample_vars_t;


//=========================== variables =======================================

static const uint8_t ipAddr_unistra[] = {0x7, 0xd1, 0x02, 0x94, 0x12, 0x5d, 0x03, 0xe9, \
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9d};

//clarinet.u-strasbg.fr (2001:660:4701:1001::157)


//=========================== prototypes ======================================


void cexample_init(void);
void cexample_timer_start(opentimer_id_t id);

/**
\}
\}
*/

#endif
