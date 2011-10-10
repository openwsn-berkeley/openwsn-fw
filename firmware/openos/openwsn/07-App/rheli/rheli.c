#include "openwsn.h"
#include "rheli.h"
#include "opencoap.h"
#include "packetfunctions.h"
#include "heli.h"
#include "openqueue.h"

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rheli_vars_t;

rheli_vars_t rheli_vars;

const uint8_t rheli_path0[]        = "h";

//=========================== prototypes ======================================

error_t rheli_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options);
void    rheli_timer();
void    rheli_sendDone(OpenQueueEntry_t* msg,
                       error_t error);

//=========================== public ==========================================

void rheli_init() {
   // initialize the heli drivers
   heli_init();
   
   // prepare the resource descriptor
   rheli_vars.desc.path0len            = sizeof(rheli_path0)-1;
   rheli_vars.desc.path0val            = (uint8_t*)(&rheli_path0);
   rheli_vars.desc.path1len            = 0;
   rheli_vars.desc.path1val            = NULL;
   rheli_vars.desc.componentID         = COMPONENT_RHELI;
   rheli_vars.desc.callbackRx          = &rheli_receive;
   rheli_vars.desc.callbackTimer       = &rheli_timer;
   rheli_vars.desc.callbackSendDone    = &rheli_sendDone;
   
   opencoap_register(&rheli_vars.desc);
}

//=========================== private =========================================

error_t rheli_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options) {      
   error_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      
      // switch on the heli
      heli_on();
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_CHANGED;
      
      outcome                          = E_SUCCESS;
   } else {
      outcome                          = E_FAIL;
   }
   return outcome;
}

void rheli_timer() {
   // switch off the heli
   heli_off();
}

void rheli_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}