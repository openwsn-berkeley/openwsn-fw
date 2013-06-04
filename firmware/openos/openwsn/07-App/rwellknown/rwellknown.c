#include "openwsn.h"
#include "rwellknown.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"
//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} rwellknown_vars_t;

rwellknown_vars_t rwellknown_vars;

const uint8_t rwellknown_path0[]        = ".well-known";
const uint8_t rwellknown_path1[]        = "core";
const uint8_t rwellknown_testlink[]  = "</led>;if=\"actuator\";rt=\"ipso:light\";ct=\"0\"";

//=========================== prototypes ======================================

error_t rwellknown_receive(OpenQueueEntry_t* msg,
                           coap_header_iht*  coap_header,
                           coap_option_iht*  coap_options);
void    rwellknown_sendDone(OpenQueueEntry_t* msg,
                            error_t error);

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

error_t rwellknown_receive(OpenQueueEntry_t* msg,
                           coap_header_iht*  coap_header,
                           coap_option_iht*  coap_options) {
   error_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_GET) {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // add link descriptors to the packet
      opencoap_writeLinks(msg);
         
      // add return option
      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
                                         1;
      msg->payload[1]                  = COAP_MEDTYPE_APPLINKFORMAT;
      
      // set the CoAP header
       coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   } else {
      outcome                          = E_FAIL;
   }
   return outcome;
}

void rwellknown_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}