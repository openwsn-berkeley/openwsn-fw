#include "openwsn.h"
#include "forwarding.h"
#include "iphc.h"
#include "openqueue.h"
#include "openserial.h"
#include "idmanager.h"
#include "packetfunctions.h"
#include "neighbors.h"
#include "icmpv6.h"
#include "openudp.h"
#include "opentcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void    getNextHop(open_addr_t* destination, open_addr_t* addressToWrite);
error_t fowarding_send_internal(OpenQueueEntry_t *msg);

//=========================== public ==========================================

void forwarding_init() {
}

error_t forwarding_send(OpenQueueEntry_t *msg) {
   msg->owner = COMPONENT_FORWARDING;
   return fowarding_send_internal(msg);
}

void forwarding_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_FORWARDING;
   if (msg->creator==COMPONENT_RADIO) {//that was a packet I had relayed
      openqueue_freePacketBuffer(msg);
   } else {//that was a packet coming from above
      switch(msg->l4_protocol) {
      case IANA_TCP:
         opentcp_sendDone(msg,error);
         break;
      case IANA_UDP:
         openudp_sendDone(msg,error);
         break;
      case IANA_ICMPv6:
         icmpv6_sendDone(msg,error);
         break;
      default:
         openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                               (errorparameter_t)msg->l4_protocol,
                               (errorparameter_t)0);
         // free the corresponding packet buffer
         openqueue_freePacketBuffer(msg);
      }
   }
}

void forwarding_receive(OpenQueueEntry_t* msg, ipv6_header_iht ipv6_header) {
   msg->owner = COMPONENT_FORWARDING;
   msg->l4_protocol            = ipv6_header.next_header;
   msg->l4_protocol_compressed = ipv6_header.next_header_compressed;
   if (idmanager_isMyAddress(&ipv6_header.dest) || packetfunctions_isBroadcastMulticast(&ipv6_header.dest)) {//for me
      memcpy(&(msg->l3_destinationORsource),&ipv6_header.src,sizeof(open_addr_t));
      switch(msg->l4_protocol) {
         case IANA_TCP:
            opentcp_receive(msg);
            break;
         case IANA_UDP:
            openudp_receive(msg);
            break;
         case IANA_ICMPv6:
            icmpv6_receive(msg);
            break;
         default:
            openserial_printError(COMPONENT_FORWARDING,ERR_WRONG_TRAN_PROTOCOL,
                                  (errorparameter_t)msg->l4_protocol,
                                  (errorparameter_t)1);
      }
   } else { //relay
      memcpy(&(msg->l3_destinationORsource),&ipv6_header.dest,sizeof(open_addr_t));//because initially contains source
      //TBC: source address gets changed!
      // change the creator to this components (should have been MAC)
      msg->creator = COMPONENT_FORWARDING;
      // resend as if from upper layer
      if (fowarding_send_internal(msg)==E_FAIL) {
         openqueue_freePacketBuffer(msg);
      }
   }
}

//=========================== private =========================================

error_t fowarding_send_internal(OpenQueueEntry_t *msg) {
   getNextHop(&(msg->l3_destinationORsource),&(msg->l2_nextORpreviousHop));
   if (msg->l2_nextORpreviousHop.type==ADDR_NONE) {
      openserial_printError(COMPONENT_FORWARDING,ERR_NO_NEXTHOP,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return E_FAIL;
   }
   return iphc_sendFromForwarding(msg);
}

void getNextHop(open_addr_t* destination128b, open_addr_t* addressToWrite64b) {
   uint8_t i;
   open_addr_t temp_prefix64btoWrite;
   if (packetfunctions_isBroadcastMulticast(destination128b)) {
      // IP destination is broadcast, send to 0xffffffffffffffff
      addressToWrite64b->type = ADDR_64B;
      for (i=0;i<8;i++) {
         addressToWrite64b->addr_64b[i] = 0xff;
      }
   } else if (neighbors_isStableNeighbor(destination128b)) {
       // IP destination is 1-hop neighbor, send directly
      packetfunctions_ip128bToMac64b(destination128b,&temp_prefix64btoWrite,addressToWrite64b);
   } else {
      // destination is remote, send to preferred parent
      neighbors_getPreferredParent(addressToWrite64b,ADDR_64B);
   }
}