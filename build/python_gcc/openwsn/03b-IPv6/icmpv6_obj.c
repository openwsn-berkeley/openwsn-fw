/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:40.431264.
*/
#include "openwsn_obj.h"
#include "icmpv6_obj.h"
#include "icmpv6echo_obj.h"
#include "icmpv6rpl_obj.h"
#include "forwarding_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void icmpv6_init(OpenMote* self) {
}

owerror_t icmpv6_send(OpenMote* self, OpenQueueEntry_t* msg) {
   msg->owner       = COMPONENT_ICMPv6;
   msg->l4_protocol = IANA_ICMPv6;
   return forwarding_send(self, msg);
}

void icmpv6_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_ICMPv6;
   switch (msg->l4_sourcePortORicmpv6Type) {
      case IANA_ICMPv6_ECHO_REQUEST:
      case IANA_ICMPv6_ECHO_REPLY:
 icmpv6echo_sendDone(self, msg, error);
         break;
      case IANA_ICMPv6_RPL:
 icmpv6rpl_sendDone(self, msg, error);
         break;
      default:
 openserial_printCritical(self, COMPONENT_ICMPv6,ERR_UNSUPPORTED_ICMPV6_TYPE,
                               (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
                               (errorparameter_t)0);
         // free the corresponding packet buffer
 openqueue_freePacketBuffer(self, msg);
         break;
   }
}

void icmpv6_receive(OpenMote* self, OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_ICMPv6;
   msg->l4_sourcePortORicmpv6Type = ((ICMPv6_ht*)(msg->payload))->type;
   switch (msg->l4_sourcePortORicmpv6Type) {
      case IANA_ICMPv6_ECHO_REQUEST:
      case IANA_ICMPv6_ECHO_REPLY:
 icmpv6echo_receive(self, msg);
         break;
      case IANA_ICMPv6_RPL:
 icmpv6rpl_receive(self, msg);
         break;
      default:
// openserial_printError(self, COMPONENT_ICMPv6,ERR_UNSUPPORTED_ICMPV6_TYPE,
//                               (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
//                               (errorparameter_t)1);
         // free the corresponding packet buffer
 openqueue_freePacketBuffer(self, msg);
         break;
   }
}
