#include "opendefs.h"
#include "ctraffic.h"
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

//=========================== defines =========================================

const uint8_t ctraffic_path0[]    = "traffic";
const uint8_t ctraffic_payload[]  = "OpenWSN";
static const uint8_t dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
}; 

//=========================== variables =======================================

ctraffic_vars_t ctraffic_vars;

//=========================== prototypes ======================================

owerror_t ctraffic_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void ctraffic_timer_cb(void);
void ctraffic_task_cb(void);
void ctraffic_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void ctraffic_init(void) {
   
   // register to OpenCoAP module
   ctraffic_vars.desc.path0len              = sizeof(ctraffic_path0)-1;
   ctraffic_vars.desc.path0val              = (uint8_t*)(&ctraffic_path0);
   ctraffic_vars.desc.path1len              = 0;
   ctraffic_vars.desc.path1val              = NULL;
   ctraffic_vars.desc.componentID           = COMPONENT_CSTORM;
   ctraffic_vars.desc.callbackRx            = &ctraffic_receive;
   ctraffic_vars.desc.callbackSendDone      = &ctraffic_sendDone;
   opencoap_register(&ctraffic_vars.desc);
   
   
   //start a periodic timer
   //comment : not running by default
   ctraffic_vars.period           = 65533; 
   
   ctraffic_vars.timerId                    = opentimers_start(
      ctraffic_vars.period,
      TIMER_PERIODIC,TIME_MS,
      ctraffic_timer_cb
   );
   /*
   //stop 
   //opentimers_stop(ctraffic_vars.timerId);
   */
}

//=========================== private =========================================

owerror_t ctraffic_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   return E_FAIL;
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
         msg->payload[1]          = (uint8_t)(ctraffic_vars.period >> 8);
         msg->payload[2]          = (uint8_t)(ctraffic_vars.period & 0xff);
         
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
         ctraffic_vars.period     = 0;
         ctraffic_vars.period    |= (msg->payload[0] << 8);
         ctraffic_vars.period    |= msg->payload[1];
         
         /*
         // stop and start again only if period > 0
         opentimers_stop(ctraffic_vars.timerId);
         
         if(ctraffic_vars.period > 0) {
            opentimers_setPeriod(ctraffic_vars.timerId,TIME_MS,ctraffic_vars.period);
            opentimers_restart(ctraffic_vars.timerId);
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
void ctraffic_timer_cb(){
   scheduler_push_task(ctraffic_task_cb,TASKPRIO_COAP);
}

void ctraffic_task_cb() {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              numOptions;
   
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(ctraffic_vars.timerId);
      return;
   }
   
   if(ctraffic_vars.period == 0) {
      // stop the periodic timer
      opentimers_stop(ctraffic_vars.timerId);
      return;
   }
   
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
   packetfunctions_reserveHeaderSize(pkt,sizeof(ctraffic_payload)-1);
   memcpy(&pkt->payload[0],ctraffic_payload,sizeof(ctraffic_payload)-1);
   
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
   packetfunctions_reserveHeaderSize(pkt,sizeof(ctraffic_path0)-1);
   memcpy(&pkt->payload[0],ctraffic_path0,sizeof(ctraffic_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_NUM_URIPATH-7) << 4 | (sizeof(ctraffic_path0)-1);
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
      COAP_CODE_REQ_POST,
      numOptions,
      &ctraffic_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void ctraffic_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

