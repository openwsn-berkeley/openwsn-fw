/**
\brief A CoAP resource accepting ACE access tokens.
*/

#include "opendefs.h"
#include "cauthz.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t cauthz_path0[] = "authz-info";

//=========================== variables =======================================

cauthz_vars_t cauthz_vars;

//=========================== prototypes ======================================

owerror_t     cauthz_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
);
void          cauthz_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void cauthz_init(void) {
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return;

   // prepare the resource descriptor for the /i path
   cauthz_vars.desc.path0len             = sizeof(cauthz_path0)-1;
   cauthz_vars.desc.path0val             = (uint8_t*)(&cauthz_path0);
   cauthz_vars.desc.path1len             = 0;
   cauthz_vars.desc.path1val             = NULL;
   cauthz_vars.desc.componentID          = COMPONENT_CAUTHZ;
   cauthz_vars.desc.securityContext      = NULL;
   cauthz_vars.desc.discoverable         = TRUE;
   cauthz_vars.desc.callbackRx           = &cauthz_receive;
   cauthz_vars.desc.callbackSendDone     = &cauthz_sendDone;

   // register with the CoAP module
   opencoap_register(&cauthz_vars.desc);
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
owerror_t cauthz_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
) {
   owerror_t outcome;

   switch (coap_header->Code) {
      case COAP_CODE_REQ_POST:
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
void cauthz_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
