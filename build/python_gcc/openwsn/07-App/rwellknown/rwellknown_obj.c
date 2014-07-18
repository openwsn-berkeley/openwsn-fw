/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:09.314117.
*/
#include "openwsn_obj.h"
#include "rwellknown_obj.h"
#include "opencoap_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "idmanager_obj.h"

//=========================== variables =======================================

rwellknown_vars_t rwellknown_vars;

const uint8_t rwellknown_path0[]       = ".well-known";
const uint8_t rwellknown_path1[]       = "core";

//=========================== prototypes ======================================

owerror_t rwellknown_receive(OpenMote* self, 
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);

void rwellknown_sendDone(OpenMote* self, 
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void rwellknown_init(OpenMote* self) {
   if( idmanager_getIsDAGroot(self)==TRUE) return; 
   
   // prepare the resource descriptor for the /.well-known/core path
   rwellknown_vars.desc.path0len            = sizeof(rwellknown_path0)-1;
   rwellknown_vars.desc.path0val            = (uint8_t*)(&rwellknown_path0);
   rwellknown_vars.desc.path1len            = sizeof(rwellknown_path1)-1;
   rwellknown_vars.desc.path1val            = (uint8_t*)(&rwellknown_path1);
   rwellknown_vars.desc.componentID         = COMPONENT_RWELLKNOWN;
   rwellknown_vars.desc.callbackRx          = &rwellknown_receive;
   rwellknown_vars.desc.callbackSendDone    = &rwellknown_sendDone;
   
 opencoap_register(self, &rwellknown_vars.desc);
}

//=========================== private =========================================

owerror_t rwellknown_receive(OpenMote* self, 
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
 opencoap_writeLinks(self, msg);
         
 packetfunctions_reserveHeaderSize(self, msg,1);
         msg->payload[0]     = COAP_PAYLOAD_MARKER;
            
         // add return option
 packetfunctions_reserveHeaderSize(self, msg,2);
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

void rwellknown_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}