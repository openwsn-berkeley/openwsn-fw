/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:59.852916.
*/
/**
\brief RSVP component

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012
*/

#include "rsvp_obj.h"
#include "openwsn_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"

typedef struct{
   uint16_t rsvp_period;
   uint8_t rsvp_timer_id;
}rsvp_vars_t;

void rsvp_timer_cb(OpenMote* self);

rsvp_vars_t rsvp_vars;

void rsvp_init(){
   rsvp_vars.rsvp_period    = 0;
   rsvp_vars.rsvp_timer_id  = 0;
}

/**
 * this function does not have a msg as a parameter because we do not want applications know about
 * rsvp internals. I think that the msg should be created here and send transparently to the application requesting
 * bandwith. 
 */

void rsvp_qos_request(OpenMote* self, uint8_t bandwith, uint16_t refresh_period, open_addr_t dest) {
   OpenQueueEntry_t* pkt;
   
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_RSVP);
   if (pkt==NULL) {
// openserial_printError(self, 
//         COMPONENT_RSVP,ERR_NO_FREE_PACKET_BUFFER,
//         (errorparameter_t)0,
//         (errorparameter_t)0
//      );
 openqueue_freePacketBuffer(self, pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_RSVP;
   pkt->owner      = COMPONENT_RSVP;
   // RSVP header structure 
   
   pkt->l4_protocol = IANA_RSVP;
   //pkt->l4_length   = pkt->length;
   
 packetfunctions_reserveHeaderSize(self, pkt,sizeof(rsvp_path_msg_t));
   // packetfunctions_htons(self, msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
   // packetfunctions_htons(self, msg->l4_destination_port,&(msg->payload[2]));
   // packetfunctions_htons(self, msg->length,&(msg->payload[4]));
   // packetfunctions_calculateChecksum(self, msg,(uint8_t*)&(((udp_ht*)msg->payload)->checksum));
   // forwarding_send(self, msg);
   
   //rsvp_vars.timerId = opentimers_start(self, rsvp_vars.rsvp_period,
   //                                       TIMER_PERIODIC,TIME_MS,
   //                                       res_timer_cb);
}

void rsvp_timer_cb(OpenMote* self){
   
}
