#include "opendefs.h"
#include "cwellknown.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"

//=========================== variables =======================================

cwellknown_vars_t cwellknown_vars;

const uint8_t cwellknown_path0[]       = ".well-known";
const uint8_t cwellknown_path1[]       = "core";

//=========================== prototypes ======================================

owerror_t cwellknown_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options,
   uint8_t*          response_options
);

void    cwellknown_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void cwellknown_init() {
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /.well-known/core path
   cwellknown_vars.desc.path0len            = sizeof(cwellknown_path0)-1;
   cwellknown_vars.desc.path0val            = (uint8_t*)(&cwellknown_path0);
   cwellknown_vars.desc.path1len            = sizeof(cwellknown_path1)-1;
   cwellknown_vars.desc.path1val            = (uint8_t*)(&cwellknown_path1);
   cwellknown_vars.desc.componentID         = COMPONENT_CWELLKNOWN;
   cwellknown_vars.desc.discoverable        = FALSE;
   cwellknown_vars.desc.callbackRx          = &cwellknown_receive;
   cwellknown_vars.desc.callbackSendDone    = &cwellknown_sendDone;
   
   opencoap_register(&cwellknown_vars.desc);
}

//=========================== private =========================================

owerror_t cwellknown_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options,
      uint8_t*          response_options
   ) {
   owerror_t outcome;
   
   switch(coap_header->Code) {
      case COAP_CODE_REQ_GET:
         // reset packet payload
         msg->payload        = &(msg->packet[127]);
         msg->length         = 0;
         
         // have CoAP module write links to all resources
         opencoap_writeLinks(msg,COMPONENT_CWELLKNOWN);
         
         // add return option
         response_options[0] = COAP_MEDTYPE_APPLINKFORMAT;
         opencoap_setOption(coap_options, COAP_OPTION_NUM_CONTENTFORMAT, 1, response_options);
         
         // set the CoAP header
         coap_header->Code   = COAP_CODE_RESP_CONTENT;
         
         outcome             = E_SUCCESS;
         
         break;
      default:
         outcome             = E_FAIL;
         break;
   }
   
   return outcome;
}

void cwellknown_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
