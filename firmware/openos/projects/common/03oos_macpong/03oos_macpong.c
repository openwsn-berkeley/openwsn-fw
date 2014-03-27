/**
\brief A simple application to test MAC connectivity.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, November 2012.
*/

// stack initialization
#include "openwsn.h"
#include "board.h"
#include "scheduler.h"
#include "openwsn.h"
// needed for spoofing
#include "openqueue.h"
#include "opentimers.h"
#include "IEEE802154E.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "res.h"
#include "idmanager.h"
#include "neighbors.h"

#define MAX_PAYLOAD 100

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} macpong_vars_t;

macpong_vars_t macpong_vars;

//=========================== prototypes ======================================

void macpong_initSend();
void macpong_send(uint8_t payloadCtr);

//=========================== initialization ==================================

int mote_main() {
   board_init();
   scheduler_init();
   openwsn_init();
   scheduler_start();
   return 0; // this line should never be reached
}

void macpong_initSend() {
   if (ieee154e_isSynch()==TRUE && neighbors_getNumNeighbors()==1) {
      // send packet
      macpong_send(0);   
      // cancel timer
      opentimers_stop(macpong_vars.timerId);
   }
}

void macpong_send(uint8_t payloadCtr) {
   OpenQueueEntry_t* pkt;
   uint8_t i;
   
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPRAND);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_IPHC,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_IPHC;
   pkt->owner                       = COMPONENT_IPHC;
   
   neighbors_getNeighbor(&pkt->l2_nextORpreviousHop,ADDR_64B,0);
   packetfunctions_reserveHeaderSize(pkt,MAX_PAYLOAD);
   for (i=0;i<MAX_PAYLOAD;i++){
     ((uint8_t*)pkt->payload)[i]      = i;
   }
   res_send(pkt);
}

//=========================== spoofing ========================================

//===== IPHC

void iphc_init() {
   if (idmanager_getIsDAGroot()==FALSE) {
      macpong_vars.timerId    = opentimers_start(5000,
                                                 TIMER_PERIODIC,TIME_MS,
                                                 macpong_initSend);
   }
}

void iphc_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_IPHC;
   openqueue_freePacketBuffer(msg);
}

void iphc_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_IPHC;
   macpong_send(++msg->payload[0]);
   openqueue_freePacketBuffer(msg);
}

//===== L3

void forwarding_init()       { return; }
void openbridge_init()       { return; }
void openbridge_triggerData(){ return; }

//===== L4

void icmpv6_init()           { return; }

void icmpv6echo_init()       { return; }
void icmpv6echo_trigger()    { return; }

void icmpv6router_init()     { return; }
void icmpv6router_trigger()  { return; }

void icmpv6rpl_init()        { return; }
void icmpv6rpl_trigger()     { return; }

void opentcp_init()          { return; }

void openudp_init()          { return; }

void opencoap_init()         { return; }

//===== L7

void ohlone_init()           { return; }

void tcpecho_init()          { return; }

void tcpinject_init()        { return; }
void tcpinject_trigger()     { return; }

void tcpprint_init()         { return; }

void udpecho_init()          { return; }

void udpinject_init()        { return; }
void udpinject_trigger()     { return; }

void udpprint_init()         { return; }

void udprand_init()          { return; }

void rinfo_init()            { return; }
void rleds__init()           { return; }
void r6t_init()              { return; }
void rwellknown_init()       { return; }
