#include "openwsn.h"
#include "rwellknown.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"

//=========================== variables =======================================

rwellknown_vars_t rwellknown_vars;

const uint8_t rwellknown_path0[]       = ".well-known";
const uint8_t rwellknown_path1[]       = "core";

//=========================== prototypes ======================================

owerror_t rwellknown_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);

void    rwellknown_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void rwellknown_init() {
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /.well-known/core path
   rwellknown_vars.desc.path0len            = sizeof(rwellknown_path0)-1;
   rwellknown_vars.desc.path0val            = (uint8_t*)(&rwellknown_path0);
   rwellknown_vars.desc.path1len            = sizeof(rwellknown_path1)-1;
   rwellknown_vars.desc.path1val            = (uint8_t*)(&rwellknown_path1);
   rwellknown_vars.desc.componentID         = COMPONENT_RWELLKNOWN;
   rwellknown_vars.desc.callbackRx          = &rwellknown_receive;
   rwellknown_vars.desc.callbackSendDone    = &rwellknown_sendDone;
   
   opencoap_register(&rwellknown_vars.desc);
}

//=========================== private =========================================

owerror_t rwellknown_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   owerror_t outcome;
   
   switch(coap_header->Code) {
      case COAP_CODE_REQ_GET:
         // reset packet payload
         msg->payload        = &(msg->packet[127]);
         msg->length         = 0;
         
         // have CoAP module write links to all resources
         opencoap_writeLinks(msg);
         
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0]     = COAP_PAYLOAD_MARKER;
            
         // add return option
         packetfunctions_reserveHeaderSize(msg,2);
         msg->payload[0]     = COAP_OPTION_NUM_CONTENTFORMAT << 4 | 1;
         msg->payload[1]     = COAP_MEDTYPE_APPLINKFORMAT;
         
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

void rwellknown_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}