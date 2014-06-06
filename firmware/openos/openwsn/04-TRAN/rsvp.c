/**
\brief RSVP component

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012
*/



#include "rsvp.h"
#include "openwsn.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

typedef struct{
   uint16_t rsvp_period;
   uint8_t rsvp_timer_id;
}rsvp_vars_t;

void rsvp_timer_cb();

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

void rsvp_qos_request(uint8_t bandwith, uint16_t refresh_period, open_addr_t dest){
   
      OpenQueueEntry_t* pkt;
      owerror_t           outcome;
      uint8_t           i,j;
     
      pkt = openqueue_getFreePacketBuffer(COMPONENT_RSVP);
      if (pkt==NULL) {
         openserial_printError(COMPONENT_RSVP,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         openqueue_freePacketBuffer(pkt);
         return;
      }
      // take ownership over that packet
      pkt->creator    = COMPONENT_RSVP;
      pkt->owner      = COMPONENT_RSVP;
      // RSVP header structure 
    
      pkt->l4_protocol = IANA_RSVP;
    //  pkt->l4_length   = pkt->length;
    
      packetfunctions_reserveHeaderSize(pkt,sizeof(rsvp_path_msg_t));
      
//       packetfunctions_htons(msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
//       packetfunctions_htons(msg->l4_destination_port,&(msg->payload[2]));
//       packetfunctions_htons(msg->length,&(msg->payload[4]));
//       packetfunctions_calculateChecksum(msg,(uint8_t*)&(((udp_ht*)msg->payload)->checksum));
//       forwarding_send(msg);

   
   
   
   
   
   
   
   //rsvp_vars.timerId = opentimers_start(rsvp_vars.rsvp_period,
   //                                       TIMER_PERIODIC,TIME_MS,
   //                                       res_timer_cb);
   i = i;
   j = j;
   outcome = outcome;
}

void rsvp_timer_cb(){
   
}