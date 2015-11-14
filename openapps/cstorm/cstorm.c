#include "opendefs.h"
#include "cstorm.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "schedule.h"

//=========================== defines =========================================

const uint8_t cstorm_path0[]    = "storm";
const uint8_t cstorm_payload[]  = "OpenWSN";
static const uint8_t dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
}; 

#define PACKET_PER_SLOTFRAME  1
#define SLOTDURATION_MS      15 // 15ms per slot

//=========================== variables =======================================

cstorm_vars_t cstorm_vars;

//=========================== prototypes ======================================

owerror_t cstorm_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void cstorm_timer_cb(opentimer_id_t id);
void cstorm_task_cb(void);
void cstorm_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void cstorm_init(void) {
   
   // register to OpenCoAP module
   cstorm_vars.desc.path0len              = sizeof(cstorm_path0)-1;
   cstorm_vars.desc.path0val              = (uint8_t*)(&cstorm_path0);
   cstorm_vars.desc.path1len              = 0;
   cstorm_vars.desc.path1val              = NULL;
   cstorm_vars.desc.componentID           = COMPONENT_CSTORM;
   cstorm_vars.desc.discoverable          = TRUE;
   cstorm_vars.desc.callbackRx            = &cstorm_receive;
   cstorm_vars.desc.callbackSendDone      = &cstorm_sendDone;
   cstorm_vars.busySending                = FALSE;
   cstorm_vars.packetId[0]                = 0;
   cstorm_vars.packetId[1]                = 0;
   opencoap_register(&cstorm_vars.desc);
   
   //start a periodic timer
   cstorm_vars.period           = SLOTFRAME_LENGTH * SLOTDURATION_MS / PACKET_PER_SLOTFRAME;
//   cstorm_vars.period           = SLOTFRAME_LENGTH * SLOTDURATION_MS / (1+openrandom_get16b()%6); 
   cstorm_vars.timerId                    = opentimers_start(
      cstorm_vars.period,
      TIMER_PERIODIC,TIME_MS,
      cstorm_timer_cb
   );
   
//   if (
//       idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x06 && \
//       idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x07 && \
//       idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x08 && \
//       idmanager_getMyID(ADDR_64B)->addr_64b[7] != 0x09  
//   ) {
//       opentimers_stop(cstorm_vars.timerId);
//   }
}

uint16_t cstorm_getPeriod() {
    return cstorm_vars.period;
}

//=========================== private =========================================

owerror_t cstorm_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   owerror_t outcome;
   
   switch (coap_header->Code) {
      
      case COAP_CODE_REQ_GET:
         
         // reset packet payload
         msg->payload             = &(msg->packet[127]);
         msg->length              = 0;
         
         // add CoAP payload
         packetfunctions_reserveHeaderSize(msg, 3);
         msg->payload[0]          = COAP_PAYLOAD_MARKER;
         
         // return as big endian
         msg->payload[1]          = (uint8_t)(cstorm_vars.period >> 8);
         msg->payload[2]          = (uint8_t)(cstorm_vars.period & 0xff);
         
         // set the CoAP header
         coap_header->Code        = COAP_CODE_RESP_CONTENT;
         
         outcome                  = E_SUCCESS;
         break;
      
      case COAP_CODE_REQ_PUT:
         
         if (msg->length!=2) {
            outcome               = E_FAIL;
            coap_header->Code     = COAP_CODE_RESP_BADREQ;
         }
         
         // read the new period
         cstorm_vars.period     = 0;
         cstorm_vars.period    |= (msg->payload[0] << 8);
         cstorm_vars.period    |= msg->payload[1];
         
         /*
         // stop and start again only if period > 0
         opentimers_stop(cstorm_vars.timerId);
         
         if(cstorm_vars.period > 0) {
            opentimers_setPeriod(cstorm_vars.timerId,TIME_MS,cstorm_vars.period);
            opentimers_restart(cstorm_vars.timerId);
         }
         */
         
         // reset packet payload
         msg->payload             = &(msg->packet[127]);
         msg->length              = 0;
         
         // set the CoAP header
         coap_header->Code        = COAP_CODE_RESP_CHANGED;
         
         outcome                  = E_SUCCESS;
         break;
      
      default:
         outcome = E_FAIL;
         break;
   }
   
   return outcome;
}

/**
\note timer fired, but we don't want to execute task in ISR mode instead, push
   task to scheduler with CoAP priority, and let scheduler take care of it.
*/
void cstorm_timer_cb(opentimer_id_t id){
   scheduler_push_task(cstorm_task_cb,TASKPRIO_COAP);
}

void cstorm_task_cb() {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              numOptions;
   uint8_t              asn[5];
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(cstorm_vars.timerId);
      return;
   }
   
   if(cstorm_vars.period == 0) {
      // stop the periodic timer
      opentimers_stop(cstorm_vars.timerId);
      return;
   }
   
//   if(cstorm_vars.busySending == TRUE) {
//       return;
//   }
   
   // if you get here, send a packet
   
   // get a packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CSTORM);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_CSTORM,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   
   // take ownership over that packet
   pkt->creator    = COMPONENT_CSTORM;
   pkt->owner      = COMPONENT_CSTORM;
   
   //The contents of the message are written in reverse order : the payload first
   //packetfunctions_reserveHeaderSize moves the index pkt->payload
   
   // add payload
   packetfunctions_reserveHeaderSize(pkt,sizeof(cstorm_payload)-1);
   memcpy(&pkt->payload[0],cstorm_payload,sizeof(cstorm_payload)-1);
   
   cstorm_vars.packetId[1]++;
   if (cstorm_vars.packetId[1]==0){
       cstorm_vars.packetId[0]++;
   }
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0] = cstorm_vars.packetId[0];
   pkt->payload[1] = cstorm_vars.packetId[1];
   
   // add asn for calculate latency
   ieee154e_getAsn(asn);
   packetfunctions_reserveHeaderSize(pkt,5);
   memcpy(&pkt->payload[0],asn,5);
   
//   printf("Mote: %d, generates packet %d at ASN: %d\n",
//          idmanager_getMyID(ADDR_64B)->addr_64b[7],
//          cstorm_vars.packetId[0]*256+cstorm_vars.packetId[1],
//          ((((asn[4]*256)+asn[3])*256+asn[2])*256+asn[1])*256+asn[0]);
   
   //set the TKL byte as a counter of Options
   //TODO: This is not conform with RFC7252, but yes with current dissector WS v1.10.6
   numOptions = 0;
   
   //Bigger Options last in message, first in the code (as it is in reverse order) 
   //Deltas are calculated between too consecutive lengthes.
   
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0] = (COAP_OPTION_NUM_CONTENTFORMAT-COAP_OPTION_NUM_URIPATH) << 4 | 1; 
   pkt->payload[1] = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(cstorm_path0)-1);
   memcpy(&pkt->payload[0],cstorm_path0,sizeof(cstorm_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_NUM_URIPATH-7) << 4 | (sizeof(cstorm_path0)-1);
   numOptions++;
   
   // length of uri-port option added directly by opencoap_send
   packetfunctions_reserveHeaderSize(pkt,11);
   numOptions++;
   
   // metadata
   pkt->l4_destination_port = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&dst_addr,16);
   
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      numOptions,
      &cstorm_vars.desc
   );
   
   cstorm_vars.busySending = TRUE;
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
      cstorm_vars.busySending = FALSE;
   }
}

void cstorm_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
   cstorm_vars.busySending = FALSE;

}

void cstorm_generateNewTraffic() {
//   cstorm_vars.period           = SLOTFRAME_LENGTH * SLOTDURATION_MS / PACKET_PER_SLOTFRAME; 
   // generate next packet with random interval
   cstorm_vars.period = SLOTFRAME_LENGTH * SLOTDURATION_MS / (3+openrandom_get16b()%3); 
   // set cstorm packet generating timer
   opentimers_setPeriod(
      cstorm_vars.timerId,
      TIME_MS,
      cstorm_vars.period
   );
   opentimers_restart(cstorm_vars.timerId);
}

void cstorm_stop() {
   cstorm_vars.period = 0xffff;
   opentimers_stop(cstorm_vars.timerId);
}

