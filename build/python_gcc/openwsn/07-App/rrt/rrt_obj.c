/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:07.181880.
*/
/**
\brief A CoAP resource which indicates the board its running on.
*/

#include "openwsn_obj.h"
#include "rrt_obj.h"
#include "opencoap_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "openrandom_obj.h"
#include "board_obj.h"
#include "idmanager_obj.h"

//=========================== defines =========================================

const uint8_t rrt_path0[] = "rt";

//=========================== variables =======================================

// declaration of global variable _rrt_vars_ removed during objectification.

//=========================== prototypes ======================================

owerror_t rrt_receive(OpenMote* self, 
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void rrt_sendDone(OpenMote* self, 
   OpenQueueEntry_t* msg,
   owerror_t error
);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void rrt_init(OpenMote* self) {
   
   // do not run if DAGroot
   if( idmanager_getIsDAGroot(self)==TRUE) return; 
   
   // prepare the resource descriptor for the /i path
   (self->rrt_vars).desc.path0len             = sizeof(rrt_path0)-1;
   (self->rrt_vars).desc.path0val             = (uint8_t*)(&rrt_path0);
   (self->rrt_vars).desc.path1len             = 0;
   (self->rrt_vars).desc.path1val             = NULL;
   (self->rrt_vars).desc.componentID          = COMPONENT_RRT;
   (self->rrt_vars).desc.callbackRx           = &rrt_receive;
   (self->rrt_vars).desc.callbackSendDone     = &rrt_sendDone;
   
   // register with the CoAP module
 opencoap_register(self, &(self->rrt_vars).desc);
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
owerror_t rrt_receive(OpenMote* self, 
      OpenQueueEntry_t* msg,
      coap_header_iht* coap_header,
      coap_option_iht* coap_options
   ) {
   
   owerror_t outcome;
   
   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         
         //=== reset packet payload (we will reuse this packetBuffer)
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         
         //=== prepare  CoAP response
         
 packetfunctions_reserveHeaderSize(self, msg,6);
         msg->payload[0] = 'p';
         msg->payload[1] = 'o';
         msg->payload[2] = 'i';
         msg->payload[3] = 'p';
         msg->payload[4] = 'o';
         msg->payload[5] = 'i';
         
         // payload marker
 packetfunctions_reserveHeaderSize(self, msg,1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;
         
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
void rrt_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}