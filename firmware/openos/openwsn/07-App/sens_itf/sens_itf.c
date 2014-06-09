#include "openwsn.h"
#include "opencoap.h"
#include "openqueue.h"
#include "sens_itf.h"
#include "packetfunctions.h"

owerror_t sens_itf_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void sens_itf_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);

const uint8_t sens_itf_path0[]       = "sens";
coap_resource_desc_t sens_itf_vars;

void sens_itf_init() {
   
   // prepare the resource descriptor for the /l path
   sens_itf_vars.path0len            = sizeof(sens_itf_path0)-1;
   sens_itf_vars.path0val            = (uint8_t*)(&sens_itf_path0);
   sens_itf_vars.path1len            = 0;
   sens_itf_vars.path1val            = NULL;
   sens_itf_vars.componentID         = COMPONENT_SENS_ITF;
   sens_itf_vars.callbackRx          = &sens_itf_receive;
   sens_itf_vars.callbackSendDone    = &sens_itf_sendDone;
   
   // register with the CoAP module
   opencoap_register(&sens_itf_vars);
}

//=========================== private =========================================

/**
\brief Called when a CoAP message is received for this resource.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t sens_itf_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   owerror_t outcome;
   
   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         // reset packet payload
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         
         // add CoAP payload
         packetfunctions_reserveHeaderSize(msg,1+11);
         msg->payload[0]                  = COAP_PAYLOAD_MARKER;
         memcpy(&msg->payload[1],"hello world",11);
            
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;
         
         outcome                          = E_SUCCESS;
         break;
      
      case COAP_CODE_REQ_PUT:
         // reset packet payload
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CHANGED;
         
         outcome                          = E_SUCCESS;
         break;
         
      default:
         outcome                          = E_FAIL;
         break;
   }
   
   return outcome;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void sens_itf_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}