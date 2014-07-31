/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:08.859062.
*/
#ifndef __ICMPv6_H
#define __ICMPv6_H

/**
\addtogroup IPv6
\{
\addtogroup ICMPv6
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   uint8_t    type;
   uint8_t    code;
   uint16_t   checksum;
   // Below Identifier might need to be replaced by the identifier used by icmpv6rpl
   // uint16_t    identifier;
   // Below sequence_number might need to be removed
   // uint16_t    sequence_number;
} ICMPv6_ht;

typedef struct {
   uint8_t    type;
   uint8_t    code;
   uint16_t   checksum;
   uint8_t    hop_limit;
   uint8_t    flags;
   uint16_t   router_lifetime;
   uint32_t   reachable_time;
   uint32_t   retransmission_timer;
} ICMPv6_RA_ht;

typedef struct {
   uint8_t    option_type;
   uint8_t    option_length;
   uint8_t    prefix_length;
   uint8_t    flags;
   uint32_t   valid_lifetime;
   uint32_t   preferred_lifetime;
   uint32_t   unused;
   uint8_t    prefix[16]; // prefix container always 16B
} ICMPv6_64bprefix_option_ht;

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void icmpv6_init(OpenMote* self);
owerror_t icmpv6_send(OpenMote* self, OpenQueueEntry_t* msg);
void icmpv6_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void icmpv6_receive(OpenMote* self, OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
