/**
\brief A simple application to test MAC connectivity.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
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
#include "sixtop.h"
#include "idmanager.h"
#include "neighbors.h"

#define LEN_PAYLOAD 100

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} macpong_vars_t;

macpong_vars_t macpong_vars;

//=========================== prototypes ======================================

void macpong_initSend(void);
void macpong_send(uint8_t payloadCtr);

//=========================== initialization ==================================

int mote_main(void) {
   board_init();
   scheduler_init();
   openwsn_init();
   if (idmanager_getMyID(ADDR_64B)->addr_64b[7]==0xbb) {
      idmanager_setIsDAGroot(TRUE);
   }
   scheduler_start();
   return 0; // this line should never be reached
}

void macpong_initSend(void) {
   if (idmanager_getIsDAGroot()==TRUE) {
      return;
   }
   if (ieee154e_isSynch()==TRUE && neighbors_getNumNeighbors()==1) {
      // send packet
      //poipoimacpong_send(0);   
      // cancel timer
      opentimers_stop(macpong_vars.timerId);
   }
}

void macpong_send(uint8_t payloadCtr) {
   OpenQueueEntry_t* pkt;
   uint8_t i;
   
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPRAND);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_IPHC,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      return;
   }
   pkt->creator                   = COMPONENT_IPHC;
   pkt->owner                     = COMPONENT_IPHC;
   
   neighbors_getNeighbor(&pkt->l2_nextORpreviousHop,ADDR_64B,0);
   packetfunctions_reserveHeaderSize(pkt,LEN_PAYLOAD);
   ((uint8_t*)pkt->payload)[0]    = payloadCtr;
   for (i=1;i<LEN_PAYLOAD;i++){
     ((uint8_t*)pkt->payload)[i]  = i;
   }
   sixtop_send(pkt);
}

//=========================== stubbing ========================================

//===== IPHC

void iphc_init(void) {
   macpong_vars.timerId    = opentimers_start(
      5000,
      TIMER_PERIODIC,TIME_MS,
      macpong_initSend
   );
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

void forwarding_init(void)        { return; }
void openbridge_init(void)        { return; }
void openbridge_triggerData(void) { return; }

//===== L4

void icmpv6_init(void)            { return; }

void icmpv6echo_init(void)        { return; }
void icmpv6echo_trigger(void)     { return; }

void icmpv6router_init(void)      { return; }
void icmpv6router_trigger(void)   { return; }

void icmpv6rpl_init(void)         { return; }
void icmpv6rpl_trigger(void)      { return; }

void opentcp_init(void)           { return; }

void openudp_init(void)           { return; }

void opencoap_init(void)          { return; }

//===== L7

void ohlone_init(void)            { return; }

void tcpecho_init(void)           { return; }

void tcpinject_init(void)         { return; }
void tcpinject_trigger(void)      { return; }

void tcpprint_init(void)          { return; }

void udpecho_init(void)           { return; }

void udpinject_init(void)         { return; }
void udpinject_trigger(void)      { return; }

void udpprint_init(void)          { return; }

void udprand_init(void)           { return; }
void udpstorm_init(void)          { return; }

void rinfo_init(void)             { return; }
void rleds__init(void)            { return; }
void r6t_init(void)               { return; }
void rwellknown_init(void)        { return; }
void rrt_init(void)               { return; }
