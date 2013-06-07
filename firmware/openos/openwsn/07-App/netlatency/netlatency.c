#include "openwsn.h"
#include "netlatency.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opentimers.h"
#include "openrandom.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "neighbors.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} netlatency_vars_t;

netlatency_vars_t netlatency_vars;
uint8_t           sn;

//=========================== prototypes ======================================

void netlatency_timer();

//=========================== public ==========================================

void netlatency_init() {
   // 16b client addr
   open_addr_t* ipAddr_netLat_Cli;
   ipAddr_netLat_Cli->type        = ADDR_16B;
   ipAddr_netLat_Cli->addr_16b[0] = 0x08;
   ipAddr_netLat_Cli->addr_16b[1] = 0xb6;

   // reset sequence number
   sn                             = 0;

   // run only on client
   if (!(idmanager_getMyID(ADDR_16B)->addr_16b[0] == 0x08 && idmanager_getMyID(ADDR_16B)->addr_16b[1] == 0xb6)) return;
 
   netlatency_vars.timerId    = opentimers_start(NETLATENCYPERIOD,
                                            TIMER_PERIODIC,TIME_MS,
                                            netlatency_timer);
}

void netlatency_task() {
   OpenQueueEntry_t* pkt;

   //don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // prepare packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_NETLATENCY);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_NETLATENCY,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_NETLATENCY;
   pkt->owner                       = COMPONENT_NETLATENCY;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_NET_LATENCY;
   pkt->l4_destination_port         = WKP_NET_LATENCY;
   pkt->l3_destinationAdd.type      = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_netLat_Serv, 16);

   // the payload contains the ASN
   packetfunctions_reserveHeaderSize(pkt, sizeof(asn_t));
   ieee154e_getAsn(pkt->payload); // gets asn from mac layer.

   // insert seqNum
   packetfunctions_reserveHeaderSize(pkt, 1);
   memcpy(&pkt->payload[0], (uint8_t*)&sn, sizeof(sn));

   // send packet
   if ((openudp_send(pkt))==E_FAIL) {
       openserial_printError(COMPONENT_NETLATENCY,ERR_BUSY_SENDING,
                             (errorparameter_t)0,
                             (errorparameter_t)0);
       openqueue_freePacketBuffer(pkt);
   }

   // increment seqNum
   sn++;

   // close timer when test finish
   if (sn > NUMPKTTEST) {
       opentimers_stop(netlatency_vars.timerId);
   }
}

void netlatency_timer() {
   scheduler_push_task(netlatency_task, TASKPRIO_COAP);
}

void netlatency_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_NETLATENCY;
   if (msg->creator!=COMPONENT_NETLATENCY) {
      openserial_printError(COMPONENT_NETLATENCY,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

void netlatency_receive(OpenQueueEntry_t* msg) {
   uint8_t*  rcvdAsn;
   uint16_t  latency;

   // retrieve seqNum
   memcpy((uint8_t*)&sn, msg->payload, 1);

   // retrieve asn
   memcpy(rcvdAsn, &msg->payload[1], sizeof(asn_t));

   // calculate asn latency
   latency = ieee154e_asnDiff((asn_t*)rcvdAsn);

   // print info(debug)
   openserial_printInfo(COMPONENT_NETLATENCY,ERR_RCVD_LATENCY,
                        (errorparameter_t)sn,
                        (errorparameter_t)latency);

   openqueue_freePacketBuffer(msg);
}

bool debugPrint_netlatency() {
   openserial_printStatus(STATUS_NETLATENCY, (uint8_t*)&sn, sizeof(sn));
   return TRUE;
}

//=========================== private =========================================
