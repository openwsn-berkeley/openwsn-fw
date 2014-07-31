/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:15.269438.
*/
#include "openwsn_obj.h"
#include "udplatency_obj.h"
#include "openudp_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#include "opentimers_obj.h"
#include "openrandom_obj.h"
#include "opencoap_obj.h"
#include "scheduler_obj.h"
#include "IEEE802154E_obj.h"
#include "idmanager_obj.h"
#include "neighbors_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} udplatency_vars_t;

udplatency_vars_t udplatency_vars;
uint16_t          seqNum;

//=========================== prototypes ======================================

void udplatency_timer(OpenMote* self);

//=========================== public ==========================================

void udplatency_init(OpenMote* self) {
   seqNum = 0;
   udplatency_vars.timerId    = opentimers_start(self, UDPLATENCYPERIOD,
                                            TIMER_PERIODIC,TIME_MS,
                                            udplatency_timer);
}

void udplatency_task(OpenMote* self) {
   OpenQueueEntry_t* pkt;
   open_addr_t * p;
   open_addr_t  q;

   // don't run if not synch
   if ( ieee154e_isSynch(self) == FALSE) return;

   // don't run on dagroot
   if ( idmanager_getIsDAGroot(self)) {
 opentimers_stop(self, udplatency_vars.timerId);
       return;
   }

   // prepare packet
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_UDPLATENCY);
   if (pkt==NULL) {
// openserial_printError(self, COMPONENT_UDPLATENCY,ERR_NO_FREE_PACKET_BUFFER,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_UDPLATENCY;
   pkt->owner                       = COMPONENT_UDPLATENCY;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_LATENCY;
   pkt->l4_destination_port         = WKP_UDP_LATENCY;
   pkt->l3_destinationAdd.type      = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motedata,16);
   
   // the payload contains the 64bit address of the sender + the ASN
 packetfunctions_reserveHeaderSize(self, pkt, sizeof(asn_t));
 ieee154e_getAsn(self, pkt->payload);//gets asn from mac layer.
   
 packetfunctions_reserveHeaderSize(self, pkt,8);
   p= idmanager_getMyID(self, ADDR_64B);
   pkt->payload[0]    = p->addr_64b[0];
   pkt->payload[1]    = p->addr_64b[1];
   pkt->payload[2]    = p->addr_64b[2];
   pkt->payload[3]    = p->addr_64b[3];
   pkt->payload[4]    = p->addr_64b[4];
   pkt->payload[5]    = p->addr_64b[5];
   pkt->payload[6]    = p->addr_64b[6];
   pkt->payload[7]    = p->addr_64b[7];
   
 neighbors_getPreferredParentEui64(self, &q);
   if (q.type==ADDR_64B) {
 packetfunctions_reserveHeaderSize(self, pkt,8);
   
   // copy my preferred parent so we can build the topology
      pkt->payload[0] = q.addr_64b[0];
      pkt->payload[1] = q.addr_64b[1];
      pkt->payload[2] = q.addr_64b[2];
      pkt->payload[3] = q.addr_64b[3];
      pkt->payload[4] = q.addr_64b[4];
      pkt->payload[5] = q.addr_64b[5];
      pkt->payload[6] = q.addr_64b[6];
      pkt->payload[7] = q.addr_64b[7];
   }

   // insert Sequence Number
 packetfunctions_reserveHeaderSize(self, pkt,sizeof(seqNum));
   pkt->payload[0]    = (seqNum >> 8) & 0xff;
   pkt->payload[1]    = seqNum & 0xff;

   // send packet
   if (( openudp_send(self, pkt)) == E_FAIL) {
 openqueue_freePacketBuffer(self, pkt);
   }

   // increment seqNum
   seqNum++;

   // close timer when test finish
   if (seqNum > NUMPKTTEST) {
 opentimers_stop(self, udplatency_vars.timerId);
   }
}

void udplatency_timer(OpenMote* self) {
 scheduler_push_task(self, udplatency_task,TASKPRIO_COAP);
}

void udplatency_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_UDPLATENCY;
   if (msg->creator!=COMPONENT_UDPLATENCY) {
// openserial_printError(self, COMPONENT_UDPLATENCY,ERR_UNEXPECTED_SENDDONE,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
   }
 openqueue_freePacketBuffer(self, msg);
}

void udplatency_receive(OpenMote* self, OpenQueueEntry_t* msg) {
 openqueue_freePacketBuffer(self, msg);
}

//=========================== private =========================================
