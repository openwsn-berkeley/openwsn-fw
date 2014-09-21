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

#define RPL_HOPBYHOP_HEADER_OPTION_TYPE  0x63

enum {
   PCKTFORWARD     = 1, // used by the node to indicate is forwarding a packet  -- either upstream or downstream
   PCKTSEND        = 2, // used by the node to indicate is sending a packet
};

enum {
   O_FLAG          = 0x80,
   R_FLAG          = 0x40,
   F_FLAG          = 0x20,
};

//=========================== typedef =========================================

/**
\brief RPL source routing header.

As defined in http://tools.ietf.org/html/rfc6554#section-3.
*/
BEGIN_PACK
typedef struct {
   uint8_t    nextHeader;    ///< Header immediately following.
   uint8_t    HdrExtLen;     ///< In 8-octet units, excluding first 8.
   uint8_t    RoutingType;   ///< Set to 3 for "Source Routing Header".
   uint8_t    SegmentsLeft;  ///< Number of addresses still to visit.
   uint8_t    CmprICmprE;    ///< Number of prefix octets elided for all (CmprI) and last (CmprE) segment
   uint8_t    PadRes;        ///< Number of padding octets. Set to 0 if using EUI64.
   uint16_t   Reserved;      ///< Set to 0.
} rpl_routing_ht;
END_PACK

//=========================== variables =======================================

//=========================== prototypes ======================================

void      forwarding_init(void);
owerror_t forwarding_send(OpenQueueEntry_t* msg);
void      forwarding_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void      forwarding_receive(
   OpenQueueEntry_t*    msg,
   ipv6_header_iht*     ipv6_header,
   ipv6_hopbyhop_iht*   ipv6_hop_header,
   rpl_option_ht*       rpl_option
);

/**
\}
\}
*/

#endif
