#include "openwsn.h"
#include "udplatency.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opentimers.h"
#include "openrandom.h"
#include "opencoap.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "neighbors.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} udplatency_vars_t;

udplatency_vars_t udplatency_vars;
uint16_t          seqNum;

//=========================== prototypes ======================================

void udplatency_timer(void);

//=========================== public ==========================================

void udplatency_init() {
   seqNum = 0;
   udplatency_vars.timerId    = opentimers_start(UDPLATENCYPERIOD,
                                            TIMER_PERIODIC,TIME_MS,
                                            udplatency_timer);
}

void udplatency_task() {
   OpenQueueEntry_t* pkt;
   open_addr_t * p;
   open_addr_t  q;

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
       opentimers_stop(udplatency_vars.timerId);
       return;
   }

   // prepare packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPLATENCY);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_UDPLATENCY,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
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
   packetfunctions_reserveHeaderSize(pkt, sizeof(asn_t));
   ieee154e_getAsn(pkt->payload);//gets asn from mac layer.
   
   packetfunctions_reserveHeaderSize(pkt,8);
   p=idmanager_getMyID(ADDR_64B);
   pkt->payload[0]    = p->addr_64b[0];
   pkt->payload[1]    = p->addr_64b[1];
   pkt->payload[2]    = p->addr_64b[2];
   pkt->payload[3]    = p->addr_64b[3];
   pkt->payload[4]    = p->addr_64b[4];
   pkt->payload[5]    = p->addr_64b[5];
   pkt->payload[6]    = p->addr_64b[6];
   pkt->payload[7]    = p->addr_64b[7];
   
   neighbors_getPreferredParentEui64(&q);
   if (q.type==ADDR_64B) {
      packetfunctions_reserveHeaderSize(pkt,8);
   
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
   packetfunctions_reserveHeaderSize(pkt,sizeof(seqNum));
   pkt->payload[0]    = (seqNum >> 8) & 0xff;
   pkt->payload[1]    = seqNum & 0xff;

   // send packet
   if ((openudp_send(pkt)) == E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }

   // increment seqNum
   seqNum++;

   // close timer when test finish
   if (seqNum > NUMPKTTEST) {
       opentimers_stop(udplatency_vars.timerId);
   }
}

void udplatency_timer() {
  scheduler_push_task(udplatency_task,TASKPRIO_COAP);
}

void udplatency_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_UDPLATENCY;
   if (msg->creator!=COMPONENT_UDPLATENCY) {
      openserial_printError(COMPONENT_UDPLATENCY,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

void udplatency_receive(OpenQueueEntry_t* msg) {
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================
