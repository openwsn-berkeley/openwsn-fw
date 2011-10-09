#include "openwsn.h"
#include "rreg.h"
//openwsn stack
#include "opencoap.h"
#include "packetfunctions.h"

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rreg_vars_t;

rreg_vars_t rreg_vars;

const uint8_t rreg_path0[]        = "reg";

//=========================== prototypes ======================================

void rreg_receive(OpenQueueEntry_t* msg,
                  coap_header_iht*  coap_header,
                  coap_option_iht*  coap_options);

//=========================== public ==========================================

void rreg_init() {
   // prepare the resource descriptor for the /.well-known/core path
   rreg_vars.desc.path0len   = sizeof(rreg_path0)-1;
   rreg_vars.desc.path0val   = (uint8_t*)(&rreg_path0);
   rreg_vars.desc.path1len   = 0;
   rreg_vars.desc.path1val   = NULL;
   rreg_vars.desc.callbackRx = &rreg_receive;
   
   opencoap_register(&rreg_vars.desc);
}

//=========================== private =========================================

void rreg_receive(OpenQueueEntry_t* msg,
                   coap_header_iht*  coap_header,
                   coap_option_iht*  coap_options) {
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_VALID;
      
      // TODO schedule to send a registration
   }
}