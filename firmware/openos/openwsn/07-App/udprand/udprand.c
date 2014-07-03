#include "openwsn.h"
#include "udprand.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opentimers.h"
#include "openrandom.h"
#include "opencoap.h"
#include "scheduler.h"
#include "idmanager.h"
#include "IEEE802154E.h"

//=========================== defines =========================================

/// inter-packet period (in mseconds)
#define UDPRANDPERIOD     30000

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} udprand_vars_t;

udprand_vars_t udprand_vars;

//=========================== prototypes ======================================

void udprand_timer(void);

//=========================== public ==========================================

void udprand_init() {
   udprand_vars.timerId    = opentimers_start(openrandom_get16b()%UDPRANDPERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          udprand_timer);
}

void udprand_task(){
    OpenQueueEntry_t* pkt;
   
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
    
    // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(udprand_vars.timerId);
      return;
   }
   
   //prepare packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPRAND);
   if (pkt==NULL) {
//      openserial_printError(COMPONENT_UDPRAND,ERR_NO_FREE_PACKET_BUFFER,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_UDPRAND;
   pkt->owner                       = COMPONENT_UDPRAND;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RAND;
   pkt->l4_destination_port         = WKP_UDP_RAND;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motedata,16);

   //START OF TELEMATICS CODE
   pkt->l2_security = TRUE;
   pkt->l2_securityLevel = 5;
   pkt->l2_keyIdMode = 3;
   neighbors_getPreferredParentEui64(&(pkt->l2_keySource));
   pkt->l2_keyIndex = 1;

   	   //VARIABLE PAYLOAD TEST
//   uint8_t payloadLength,i;
//   payloadLength = 5;
//   packetfunctions_reserveHeaderSize(pkt, payloadLength*sizeof(uint8_t));
//   for(i=0; i<payloadLength;i++){
//   pkt->payload[i] = i*2;
//  }
   //END OF TELEMATICS CODE

   packetfunctions_reserveHeaderSize(pkt,2);
   ((uint8_t*)pkt->payload)[0]      = openrandom_get16b()%0xff;
   ((uint8_t*)pkt->payload)[1]      = openrandom_get16b()%0xff;
   //send packet
   if ((openudp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void udprand_timer() {
  scheduler_push_task(udprand_task,TASKPRIO_COAP);
}

void udprand_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_UDPRAND;
   if (msg->creator!=COMPONENT_UDPRAND) {
//      openserial_printError(COMPONENT_UDPRAND,ERR_UNEXPECTED_SENDDONE,
//                            (errorparameter_t)0,
//                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

void udprand_receive(OpenQueueEntry_t* msg) {
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================
