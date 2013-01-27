#include "openwsn.h"
#include "icmpv6router.h"
#include "icmpv6.h"
#include "openserial.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "idmanager.h"

//=========================== variables =======================================

typedef struct {
   bool        busySending;
   open_addr_t hisAddress;
   uint16_t    seq;
} icmpv6router_vars_t;

icmpv6router_vars_t icmpv6router_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void icmpv6router_init() {
   icmpv6router_vars.busySending = FALSE;
   icmpv6router_vars.seq         = 0;
}

void icmpv6router_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_ICMPv6ROUTER;
   if (msg->creator!=COMPONENT_ICMPv6ROUTER) {//that was a packet I had not created
      openserial_printError(COMPONENT_ICMPv6ROUTER,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   icmpv6router_vars.busySending = FALSE;
}

void icmpv6router_receive(OpenQueueEntry_t* msg) {
   open_addr_t temp_prefix;
   msg->owner = COMPONENT_ICMPv6ROUTER;
   //toss ICMPv6 header
   packetfunctions_tossHeader(msg,sizeof(ICMPv6_RA_ht));
   //record prefix
   if (  ((ICMPv6_64bprefix_option_ht*)(msg->payload))->option_type   == IANA_ICMPv6_RA_PREFIX_INFORMATION &&
         ((ICMPv6_64bprefix_option_ht*)(msg->payload))->option_length == 4                                 &&
         ((ICMPv6_64bprefix_option_ht*)(msg->payload))->prefix_length == 64                                &&
         msg->length>=sizeof(ICMPv6_64bprefix_option_ht) ) {
      temp_prefix.type = ADDR_PREFIX;
      memcpy(  &(temp_prefix.prefix[0]),
            &(((ICMPv6_64bprefix_option_ht*)(msg->payload))->prefix[0]),
            8);
      idmanager_setMyID(&temp_prefix);
   }
   //toss packet
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================