#include "openwsn.h"
#include "rleds.h"
#include "opencoap.h"
#include "packetfunctions.h"
#include "leds.h"
#include "openqueue.h"

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rleds_vars_t;

rleds_vars_t rleds_vars;

const uint8_t rleds_path0[]        = "l";

//=========================== prototypes ======================================

owerror_t rleds_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options);
void    rleds_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void rleds__init() {
   // prepare the resource descriptor for the /.well-known/core path
   rleds_vars.desc.path0len            = sizeof(rleds_path0)-1;
   rleds_vars.desc.path0val            = (uint8_t*)(&rleds_path0);
   rleds_vars.desc.path1len            = 0;
   rleds_vars.desc.path1val            = NULL;
   rleds_vars.desc.componentID         = COMPONENT_RLEDS;
   rleds_vars.desc.callbackRx          = &rleds_receive;
   rleds_vars.desc.callbackSendDone    = &rleds_sendDone;
   
   opencoap_register(&rleds_vars.desc);
}

//=========================== private =========================================

owerror_t rleds_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options) {      
   owerror_t outcome;
   
   if        (coap_header->Code==COAP_CODE_REQ_GET) {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // add CoAP payload
      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[0] = COAP_PAYLOAD_MARKER;

      if (leds_error_isOn()==1) {
         msg->payload[1]               = '1';
      } else {
         msg->payload[1]               = '0';
      }
         
      // set the CoAP header
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   } else if (coap_header->Code==COAP_CODE_REQ_PUT) {
      
      // change the LED's state
      if (msg->payload[0]=='1') {
         leds_debug_on();
      } else if (msg->payload[0]=='2') {
         leds_debug_toggle();
      } else {
         leds_debug_off();
      }
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->Code                = COAP_CODE_RESP_CHANGED;
      
      outcome                          = E_SUCCESS;
   } else {
      outcome                          = E_FAIL;
   }
   return outcome;
}

void rleds_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}