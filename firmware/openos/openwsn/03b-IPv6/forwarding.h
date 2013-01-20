#ifndef __FORWARDING_H
#define __FORWARDING_H

/**
\addtogroup IPv6
\{
\addtogroup Forwarding
\{
*/

#include "iphc.h"

//=========================== define ==========================================

// RFC2460 page 11
#define SOURCEFWNXTHDR 43

enum {
   PCKTFORWARD     = 1,          
   PCKTSEND        = 2,
};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void    forwarding_init();
error_t forwarding_send(OpenQueueEntry_t *msg);
void    forwarding_sendDone(OpenQueueEntry_t* msg, error_t error);
void    forwarding_receive(OpenQueueEntry_t* msg, ipv6_header_iht ipv6_header);

/**
\}
\}
*/

#endif
