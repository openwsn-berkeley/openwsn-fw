/**
\brief A protected CoAP resource, accessed over a dynamically-created OSCORE channel.
*/

#include "opendefs.h"
#include "cprotected.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t cprotected_path0[] = "resource1";

//=========================== variables =======================================

cprotected_vars_t cprotected_vars;

//=========================== prototypes ======================================

owerror_t cprotected_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
);
void cprotected_sendDone(
        OpenQueueEntry_t* msg,
        owerror_t error
);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void cprotected_init(void) {
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return;

   // prepare the resource descriptor
   cprotected_vars.desc.path0len             = sizeof(cprotected_path0)-1;
   cprotected_vars.desc.path0val             = (uint8_t*)(&cprotected_path0);
   cprotected_vars.desc.path1len             = 0;
   cprotected_vars.desc.path1val             = NULL;
   cprotected_vars.desc.componentID          = COMPONENT_CPROTECTED;
   cprotected_vars.desc.securityContext      = NULL;
   cprotected_vars.desc.discoverable         = TRUE;
   cprotected_vars.desc.callbackRx           = &cprotected_receive;
   cprotected_vars.desc.callbackSendDone     = &cprotected_sendDone;

   // register with the CoAP module
   opencoap_register(&cprotected_vars.desc);
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
owerror_t cprotected_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
) {
   owerror_t outcome;

   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         //=== reset packet payload (we will reuse this packetBuffer)
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;

         //=== prepare  CoAP response

         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;

         outcome                          = E_SUCCESS;
         break;
      default:
         // return an error message
         outcome = E_FAIL;
   }

   return outcome;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void cprotected_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
