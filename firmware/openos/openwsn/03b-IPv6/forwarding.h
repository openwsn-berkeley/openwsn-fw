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

enum {
   PCKTFORWARD     = 1,          
   PCKTSEND        = 2,
};

#define RPL_HOPBYHOP_HEADER_OPTION_TYPE  0x63

//=========================== typedef =========================================

/**
\brief RPL source routing header.

As defined in http://tools.ietf.org/html/rfc6554#section-3.
*/
PRAGMA(pack(1));
typedef struct {
   uint8_t    nextHeader;    ///< Header immediately following.
   uint8_t    HdrExtLen;     ///< In 8-octet units, excluding first 8.
   uint8_t    RoutingType;   ///< Set to 3 for "Source Routing Header".
   uint8_t    SegmentsLeft;  ///< Number of addresses still to visit.
   uint8_t    CmprICmprE;    ///< Number of prefix octets elided for all (CmprI) and last (CmprE) segment
   uint8_t    PadRes;        ///< Number of padding octets. Set to 0 if using EUI64.
   uint16_t   Reserved;      ///< Set to 0.
} rpl_routing_ht;
PRAGMA(pack());

//RPL hop by hop option header as described by RFC 6553 p.3
PRAGMA(pack(1));
typedef struct {
   uint8_t    optionType;    ///0x63.
   uint8_t    optionLen;     /////8-bit field indicating the length of the option, in octets, excluding the Option Type and Opt Data Len fields.
   uint8_t    flags;         //ORF00000.
   uint8_t    rplInstanceID;  //instanceid
   uint16_t   senderRank;    //sender rank
} rpl_hopbyhop_ht;
PRAGMA(pack());




//=========================== variables =======================================

//=========================== prototypes ======================================

void    forwarding_init();
owerror_t forwarding_send(OpenQueueEntry_t *msg);
void    forwarding_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void    forwarding_receive(OpenQueueEntry_t* msg, ipv6_header_iht ipv6_header);

/**
\}
\}
*/

#endif
