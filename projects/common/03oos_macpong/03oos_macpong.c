/**
\brief A simple application to test MAC connectivity.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

// stack initialization
#include "opendefs.h"
#include "board.h"
#include "scheduler.h"
#include "openstack.h"
#include "opentimers.h"
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
   opentimers_id_t timerId;
   uint8_t         macpongCounter;
} macpong_vars_t;

macpong_vars_t macpong_vars;

//=========================== prototypes ======================================

void macpong_initSend(opentimers_id_t id);
void macpong_send(uint8_t payloadCtr);

//=========================== initialization ==================================

int mote_main(void) {
   board_init();
   scheduler_init();
   openstack_init();
   if (idmanager_getMyID(ADDR_64B)->addr_64b[7]==0x16) {
      idmanager_setIsDAGroot(TRUE);
   }
   scheduler_start();
   return 0; // this line should never be reached
}

void macpong_initSend(opentimers_id_t id) {
    bool timeToSend = FALSE;
    macpong_vars.macpongCounter = (macpong_vars.macpongCounter+1)%5;
    switch (macpong_vars.macpongCounter) {
        case 0:
            timeToSend = TRUE;
            break;
        default:
            break;
   }
   opentimers_scheduleIn(
        macpong_vars.timerId, // id
        1000,                 // duration
        TIME_MS,              // time_type
        TIMER_ONESHOT,        // timer_type
        macpong_initSend      // callback
   );
  
  
   if (idmanager_getIsDAGroot()==TRUE) {
      return;
   }
   if (ieee154e_isSynch()==TRUE && neighbors_getNumNeighbors()==1) {
      if (timeToSend){
          // send packet
           macpong_send(0);   
          // cancel timer
          opentimers_cancel(macpong_vars.timerId);
      }
   }
}

void macpong_send(uint8_t payloadCtr) {
   OpenQueueEntry_t* pkt;
   uint8_t i;
   
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UECHO);
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
   
   neighbors_getNeighborEui64(&pkt->l2_nextORpreviousHop,ADDR_64B,0);
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
    PORT_TIMER_WIDTH       reference;
    reference            = opentimers_getValue();
    macpong_vars.timerId = opentimers_create();
    opentimers_scheduleAbsolute(
        macpong_vars.timerId,  // timerId
        1000,                  // duration
        reference,             // reference
        TIME_MS,               // timetype
        macpong_initSend       // callback
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

//===== sniffer

void sniffer_setListeningChannel(uint8_t channel)       { return; }

//===== L3

void forwarding_init(void)                              { return; }
void openbridge_init(void)                              { return; }
void openbridge_triggerData(void)                       { return; }

//===== L4

void icmpv6_init(void)                                               { return; }
void icmpv6echo_init(void)                                           { return; }
void icmpv6echo_trigger(void)                                        { return; }
void icmpv6router_init(void)                                         { return; }
void icmpv6router_trigger(void)                                      { return; }
void icmpv6rpl_init(void)                                            { return; }
void icmpv6rpl_trigger(void)                                         { return; }
void icmpv6rpl_writeDODAGid(uint8_t* dodagid)                        { return; }
void icmpv6rpl_setDIOPeriod(uint16_t dioPeriod)                      { return; }
void icmpv6rpl_setDAOPeriod(uint16_t daoPeriod)                      { return; }
bool icmpv6rpl_getPreferredParentIndex(uint8_t* indexptr)            { 
    return FALSE; 
}
bool icmpv6rpl_getPreferredParentEui64(open_addr_t* addressToWrite)  { 
    return FALSE; 
}
bool icmpv6rpl_isPreferredParent(open_addr_t* address)               { 
    return FALSE; 
}
dagrank_t icmpv6rpl_getMyDAGrank(void)                               { 
    return 0; 
}
bool icmpv6rpl_daoSent(void) {
    return TRUE;
}
void icmpv6rpl_setMyDAGrank(dagrank_t rank)                          { return; }
void icmpv6rpl_killPreferredParent(void)                             { return; }
void icmpv6rpl_updateMyDAGrankAndParentSelection(void)               { return; }
void icmpv6rpl_indicateRxDIO(OpenQueueEntry_t* msg)                  { return; }
void icmpv6echo_setIsReplyEnabled(bool isEnabled)                    { return; }



void opentcp_init(void)                                 { return; }
void openudp_init(void)                                 { return; }
void opencoap_init(void)                                { return; }

//===== L7

void openapps_init(void)                                { return; }
void ohlone_init(void)                                  { return; }
void tcpecho_init(void)                                 { return; }
void tcpinject_init(void)                               { return; }
void tcpinject_trigger(void)                            { return; }
void tcpprint_init(void)                                { return; }
void c6t_init(void)                                     { return; }
void cinfo_init(void)                                   { return; }
void cleds__init(void)                                  { return; }
void cwellknown_init(void)                              { return; }
   // TCP
void techo_init(void)                                   { return; }
   // UDP
void uecho_init(void)                                   { return; }

