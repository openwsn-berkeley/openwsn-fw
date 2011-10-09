#include "openwsn.h"
#include "netleds.h"
//openwsn stack
#include "opencoap.h"
#include "packetfunctions.h"
#include "leds.h"

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} netleds_vars_t;

netleds_vars_t netleds_vars;

const uint8_t rleds_path0[]        = "led";

//=========================== prototypes ======================================

void netleds_receive(OpenQueueEntry_t* msg,
                     coap_header_iht*  coap_header,
                     coap_option_iht*  coap_options);

//=========================== public ==========================================

void netleds_init() {
   // prepare the resource descriptor for the /.well-known/core path
   netleds_vars.desc.path0len   = sizeof(rleds_path0)-1;
   netleds_vars.desc.path0val   = (uint8_t*)(&rleds_path0);
   netleds_vars.desc.path1len   = 0;
   netleds_vars.desc.path1val   = NULL;
   netleds_vars.desc.callbackRx = &netleds_receive;
   
   opencoap_register(&netleds_vars.desc);
}

//=========================== private =========================================

void netleds_receive(OpenQueueEntry_t* msg,
                     coap_header_iht*  coap_header,
                     coap_option_iht*  coap_options) {
   
   if        (coap_header->Code==COAP_CODE_REQ_GET) {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // add CoAP payload
      packetfunctions_reserveHeaderSize(msg,1);
      if (led_errorIsOn()==TRUE) {
         msg->payload[0]               = '1';
      } else {
         msg->payload[0]               = '0';
      }
         
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
   } else if (coap_header->Code==COAP_CODE_REQ_PUT) {
      
      // change the LED's state
      if (msg->payload[0]=='1') {
         led_errorLedOn();
      } else {
         led_errorLedOff();
      }
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_CHANGED;
   }
}