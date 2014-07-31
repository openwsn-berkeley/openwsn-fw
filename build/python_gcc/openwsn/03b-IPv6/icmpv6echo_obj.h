/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:55.197556.
*/
#ifndef __ICMPv6ECHO_H
#define __ICMPv6ECHO_H

/**
\addtogroup IPv6
\{
\addtogroup ICMPv6Echo
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   bool        busySending;
   open_addr_t hisAddress;
   uint16_t    seq;
} icmpv6echo_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void icmpv6echo_init(OpenMote* self);
void icmpv6echo_trigger(OpenMote* self);
void icmpv6echo_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);
void icmpv6echo_receive(OpenMote* self, OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
