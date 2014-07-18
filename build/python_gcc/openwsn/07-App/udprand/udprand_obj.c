/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:14.665560.
*/
#include "openwsn_obj.h"
#include "udprand_obj.h"
#include "openudp_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#include "opentimers_obj.h"
#include "openrandom_obj.h"
#include "opencoap_obj.h"
#include "scheduler_obj.h"
#include "idmanager_obj.h"
#include "IEEE802154E_obj.h"

//=========================== defines =========================================

/// inter-packet period (in mseconds)
#define UDPRANDPERIOD     30000

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
} udprand_vars_t;

udprand_vars_t udprand_vars;

//=========================== prototypes ======================================

void udprand_timer(OpenMote* self);

//=========================== public ==========================================

void udprand_init(OpenMote* self) {
   udprand_vars.timerId    = opentimers_start(self, openrandom_get16b(self)%UDPRANDPERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          udprand_timer);
}

void udprand_task(OpenMote* self){
    OpenQueueEntry_t* pkt;
   
   // don't run if not synch
   if ( ieee154e_isSynch(self) == FALSE) return;
    
    // don't run on dagroot
   if ( idmanager_getIsDAGroot(self)) {
 opentimers_stop(self, udprand_vars.timerId);
      return;
   }
   
   //prepare packet
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_UDPRAND);
   if (pkt==NULL) {
 openserial_printError(self, COMPONENT_UDPRAND,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_UDPRAND;
   pkt->owner                       = COMPONENT_UDPRAND;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RAND;
   pkt->l4_destination_port         = WKP_UDP_RAND;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motedata,16);
 packetfunctions_reserveHeaderSize(self, pkt,2);
   ((uint8_t*)pkt->payload)[0]      = openrandom_get16b(self)%0xff;
   ((uint8_t*)pkt->payload)[1]      = openrandom_get16b(self)%0xff;
   //send packet
   if (( openudp_send(self, pkt))==E_FAIL) {
 openqueue_freePacketBuffer(self, pkt);
   }
}

void udprand_timer(OpenMote* self) {
 scheduler_push_task(self, udprand_task,TASKPRIO_COAP);
}

void udprand_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_UDPRAND;
   if (msg->creator!=COMPONENT_UDPRAND) {
 openserial_printError(self, COMPONENT_UDPRAND,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
 openqueue_freePacketBuffer(self, msg);
}

void udprand_receive(OpenMote* self, OpenQueueEntry_t* msg) {
 openqueue_freePacketBuffer(self, msg);
}

//=========================== private =========================================
