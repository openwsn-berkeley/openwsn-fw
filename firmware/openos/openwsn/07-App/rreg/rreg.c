/**
\brief A CoAP resource which registers the CoAP resources with a CoAP
   resource directory.
*/

#include "openwsn.h"
#include "rreg.h"
#include "opentimers.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"
#include "board.h"
#include "scheduler.h"

//=========================== variables =======================================

#define RREGPERIOD           30000

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t  timerId;
} rreg_vars_t;

rreg_vars_t rreg_vars;

const uint8_t rreg_path0[]    = "r";

//=========================== prototypes ======================================

owerror_t rreg_receive(OpenQueueEntry_t* msg,
                     coap_header_iht*  coap_header,
                     coap_option_iht*  coap_options);
void    rreg_timer();
void    rreg_sendDone(OpenQueueEntry_t* msg,
                      owerror_t error);
uint8_t hexToAscii(uint8_t hex);

//=========================== public ==========================================

void rreg_init() {
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return; 
 
   // prepare the resource descriptor for the /r path
   rreg_vars.desc.path0len             = sizeof(rreg_path0)-1;
   rreg_vars.desc.path0val             = (uint8_t*)(&rreg_path0);
   rreg_vars.desc.path1len             = 0;
   rreg_vars.desc.path1val             = NULL;
   rreg_vars.desc.componentID          = COMPONENT_RREG;
   rreg_vars.desc.callbackRx           = &rreg_receive;
   rreg_vars.desc.callbackSendDone     = &rreg_sendDone;
   
   // register with the CoAP module
   opencoap_register(&rreg_vars.desc);
   
   // start a timer to register to the CoAP Resource Directory every RREGPERIOD
   rreg_vars.timerId    = opentimers_start(
      RREGPERIOD,
      TIMER_PERIODIC,
      TIME_MS,
      rreg_timer
   );
}

//=========================== private =========================================

/**
\brief Called when a CoAP message is received for this resource.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the packet could be parsed successfully.
*/
owerror_t rreg_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   
   owerror_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      // we received the order to register now
      
      // schedule task to execute timer function next
      scheduler_push_task(rreg_timer,TASKPRIO_COAP);
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->Code                = COAP_CODE_RESP_VALID;
      
      outcome = E_SUCCESS;
   } else if (coap_header->T==COAP_TYPE_ACK) {
      // we received the ACK from the CoAP RD
      
      // it worked!
   } else {
      outcome = E_FAIL;
   }
   
   return outcome;
}

/**
\brief The timer elapsed; time to register with the CoAP RD.
*/
void rreg_timer() {
   OpenQueueEntry_t*    pkt;
   uint8_t              temp8b;
   owerror_t            outcome;
   uint8_t              numOptions;
   
   //=== request a packetBuffer
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_RREG);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_RREG,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return;
   }
   
   // take ownership over that packet
   pkt->creator    = COMPONENT_RREG;
   pkt->owner      = COMPONENT_RREG;
   
   //=== create CoAP message
   
   // write CoAP payload
   opencoap_writeLinks(pkt);
   
   numOptions = 0;
   
   // URI-query
   packetfunctions_reserveHeaderSize(pkt,sizeof(rreg_uriquery)-1+2);
   memcpy(&pkt->payload[0],&rreg_uriquery,sizeof(rreg_uriquery)-1);
   temp8b = idmanager_getMyID(ADDR_16B)->addr_16b[1];
   pkt->payload[sizeof(rreg_uriquery)-1] = hexToAscii((temp8b>>4) & 0x0f);
   pkt->payload[sizeof(rreg_uriquery)-0] = hexToAscii((temp8b>>0) & 0x0f);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_NUM_URIQUERY-COAP_OPTION_NUM_URIPATH) << 4 |
      sizeof(rreg_uriquery)-1+2;
   numOptions++;
   
   // URI-path
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0] = 'r';
   pkt->payload[1] = 'd';
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_NUM_URIPATH) << 4 |
      2;
   numOptions++;
   
   // add content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
      1;
   pkt->payload[1]                = COAP_MEDTYPE_APPLINKFORMAT;
   numOptions++;
   
   //=== send CoAP message
   
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type    = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_ipsoRD,16);
   
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_CON,
      COAP_CODE_REQ_POST,
      numOptions,
      &rreg_vars.desc
   );
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void rreg_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

/**
\brief converts a hex number into its ASCII representation.

For example: 0x0a -> 'a'

\param[in] hex A number between 0x00 and 0x0f.

\return The ASCII representation of the number, between '0' and 'f'.
*/
port_INLINE uint8_t hexToAscii(uint8_t hex) {
   if (hex<0x0a) {
      return '0'+(hex-0x00);
   } else {
      return 'a'+(hex-0x0a);
   }
}