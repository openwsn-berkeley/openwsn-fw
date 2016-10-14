/**
\brief An example CoAP application.
*/

#include "opendefs.h"
#include "cexample.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "idmanager.h"
#include "IEEE802154E.h"
#include "sixtop.h"
#include "icmpv6rpl.h"
#include <stdio.h>



//=========================== defines =========================================

/// info for traffic generation
#define  PAYLOADLEN           40

const uint32_t cexample_timeout = QUEUE_TIMEOUT_DEFAULT; //3 * 15 * SUPERFRAME_LENGTH; //3 slotframes
const char cexample_path0[] = "cexample";

//=========================== variables =======================================

cexample_vars_t cexample_vars;

//=========================== prototypes ======================================

owerror_t cexample_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);
void    cexample_start(void);
void    cexample_timer_start(opentimer_id_t id);
void    cexample_timer_cb(opentimer_id_t id);
void    cexample_task_cb(void);
void    cexample_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void cexample_init() {

   // prepare the resource descriptor for the /ex path
   cexample_vars.desc.path0len             = sizeof(cexample_path0) - 1;
   cexample_vars.desc.path0val             = (uint8_t*)(&cexample_path0);
   cexample_vars.desc.path1len             = 0;
   cexample_vars.desc.path1val             = NULL;
   cexample_vars.desc.componentID          = COMPONENT_CEXAMPLE;
   cexample_vars.desc.callbackRx           = &cexample_receive;
   cexample_vars.desc.callbackSendDone     = &cexample_sendDone;
   cexample_vars.seqnum                    = ((uint32_t)openrandom_get16b() <<16) | ((uint32_t)openrandom_get16b());

   //the track depends on the mode of the application
#if (TRACK_MGMT == TRACK_MGMT_NO)
   cexample_vars.track = sixtop_get_trackbesteffort();
#endif

#if (TRACK_MGMT == TRACK_MGMT_SHARED)
   cexample_vars.track = sixtop_get_trackcommon();
#endif

#if (TRACK_MGMT == TRACK_MGMT_ISOLATION) || (TRACK_MGMT == TRACK_MGMT_6P_ISOLATION)

   //I am the owner of this track (8 bytes address)
   memcpy(&(cexample_vars.track.owner), idmanager_getMyID(ADDR_64B), sizeof(open_addr_t));
   cexample_vars.track.instance            = (uint16_t)TRACK_CEXAMPLE;
#endif


#if (TRACK_MGMT != TRACK_MGMT_NO)  && (TRACK_MGMT != TRACK_MGMT_SHARED)  && (TRACK_MGMT != TRACK_MGMT_ISOLATION)  && (TRACK_MGMT != TRACK_MGMT_6P_ISOLATION)
   THIS IS AN ERROR
#endif

   opencoap_register(&cexample_vars.desc);

   //DAGroot: no packet is generated, only the reception part is activated
   if (idmanager_getIsDAGroot())
      return;

   //starts to generate packets when I am synchronized
   uint64_t  next = openrandom_get16b();
   while (next > 2 * CEXAMPLE_PERIOD)
      next -= CEXAMPLE_PERIOD;

   cexample_vars.timerId    = opentimers_start(
         next,
         TIMER_ONESHOT,
         TIME_MS,
         cexample_timer_start);


}




//=========================== private =========================================

owerror_t cexample_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {

   //extracts the sequence number
   uint32_t seqnum = packetfunctions_ntohl(&(msg->payload[0]));

   //a frame was received
   open_addr_t dest_64b, prefix, src_64b;
   packetfunctions_ip128bToMac64b(&(msg->l3_destinationAdd), &prefix, &dest_64b);
   packetfunctions_ip128bToMac64b(&(msg->l3_sourceAdd), &prefix, &src_64b);
   openserial_statDataRx(seqnum, &(msg->l2_track), &src_64b, &dest_64b);

   //nothing to respond
   return E_SUCCESS;
}


//starts generating the packet only once I am synchronized
void cexample_timer_start(opentimer_id_t id){

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
       return;
   }

   cexample_vars.timerId    = opentimers_start(
         CEXAMPLE_PERIOD,
         TIMER_PERIODIC,TIME_MS,
         cexample_timer_cb);

}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void cexample_timer_cb(opentimer_id_t id){
   scheduler_push_task(cexample_task_cb,TASKPRIO_COAP);
}

void cexample_task_cb() {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              i;

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(cexample_vars.timerId);
      return;
   }

   //increment the sequence number
   (cexample_vars.seqnum)++;

   //stat for data packet generation
/*   open_addr_t dest_128b, dest_64b, prefix;
   dest_128b.type = ADDR_128B;
   icmpv6rpl_getRPLDODAGid(&(dest_128b.addr_128b[0]));
   packetfunctions_ip128bToMac64b(&dest_128b, &prefix, &dest_64b);
*/


   // don't run if not synch
   if (ieee154e_isSynch() == FALSE)
      return;


   // create a CoAP packet
   pkt = openqueue_getFreePacketBuffer_with_timeout(COMPONENT_CEXAMPLE, cexample_timeout);
   if (pkt==NULL) {
      return;
   }
   // take ownership over that packet
   pkt->creator                   = COMPONENT_CEXAMPLE;
   pkt->owner                     = COMPONENT_CEXAMPLE;


   // CoAP payload - seqnum are the first two bytes
   packetfunctions_reserveHeaderSize(pkt,PAYLOADLEN);
   pkt->payload[0]                = (cexample_vars.seqnum & 0xff000000) >> 24;
   pkt->payload[1]                = (cexample_vars.seqnum & 0x00ff0000) >> 16;
   pkt->payload[2]                = (cexample_vars.seqnum & 0x0000ff00) >> 8;
   pkt->payload[3]                = (cexample_vars.seqnum & 0x000000ff);

   //garbage for the remaining bytes
   for (i=4;i<PAYLOADLEN;i++)
       pkt->payload[i]             = i;

   //coap packet
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = COAP_PAYLOAD_MARKER;
   
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                = (COAP_OPTION_NUM_CONTENTFORMAT - COAP_OPTION_NUM_URIPATH) << 4
                                    | 1;
   pkt->payload[1]                = COAP_MEDTYPE_APPOCTETSTREAM;

   // location-path option
   packetfunctions_reserveHeaderSize(pkt, sizeof(cexample_path0)-1);
   memcpy(&pkt->payload[0],cexample_path0, sizeof(cexample_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                = ((COAP_OPTION_NUM_URIPATH) << 4) | (sizeof(cexample_path0)-1);
   
   // metadata
   pkt->l2_track                  = cexample_vars.track;
   pkt->l4_destination_port       = WKP_UDP_COAP;

   // set PKT destination
   pkt->l3_destinationAdd.type = ADDR_128B;
   icmpv6rpl_getRPLDODAGid(&(pkt->l3_destinationAdd.addr_128b[0]));

   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      1,
      &cexample_vars.desc
   );
   
   //stats
   openserial_statDataGen(cexample_vars.seqnum, pkt);
         //&(cexample_vars.track), idmanager_getMyID(ADDR_64B), &dest_64b);



   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void cexample_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
