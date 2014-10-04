/**
\brief A CoAP resource which indicates the board its running on.
*/

#include "openwsn.h"
#include "rrt.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t rrt_path0[] = "rt";

//=========================== variables =======================================

rrt_vars_t rrt_vars;

//=========================== prototypes ======================================

owerror_t     rrt_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void          rrt_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);
void setGETRespMsg(
   OpenQueueEntry_t* msg,
   bool discovered   
);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void rrt_init() {
   
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /i path
   rrt_vars.desc.path0len             = sizeof(rrt_path0)-1;
   rrt_vars.desc.path0val             = (uint8_t*)(&rrt_path0);
   rrt_vars.desc.path1len             = 0;
   rrt_vars.desc.path1val             = NULL;
   rrt_vars.desc.componentID          = COMPONENT_RRT;
   rrt_vars.desc.callbackRx           = &rrt_receive;
   rrt_vars.desc.callbackSendDone     = &rrt_sendDone;

   rrt_vars.discovered                = FALSE; //if this mote has been discovered by ringmaster
   
   // register with the CoAP module
   opencoap_register(&rrt_vars.desc);
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
owerror_t rrt_receive(
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
         setGETRespMsg(msg, rrt_vars.discovered);
         
         
         // payload marker
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;
         
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;
         
         outcome                          = E_SUCCESS;
         break;
      case COAP_CODE_REQ_PUT:
         // reset packet payload
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;

         packetfunctions_reserveHeaderSize(msg, 1);
         msg->payload[0] = 'y';

         //payload marker
         packetfunctions_reserveHeaderSize(msg, 1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;

         //set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CHANGED;

         outcome                          = E_SUCCESS;
         break;
      default:
         // return an error message
         outcome = E_FAIL;
   }
   
   return outcome;
}

void setGETRespMsg(OpenQueueEntry_t* msg, bool registered) {
         if (registered == FALSE) {
             packetfunctions_reserveHeaderSize(msg,11);
             msg->payload[0] = 'r';
             msg->payload[1] = 'e';
             msg->payload[2] = 'g';
             msg->payload[3] = 'i';
             msg->payload[4] = 's';
             msg->payload[5] = 't';
             msg->payload[6] = 'e';
             msg->payload[7] = 'r';
             msg->payload[8] = 'i';
             msg->payload[9] = 'n';
             msg->payload[10] = 'g';

             //send packet to local with 'D' here
             //sendDiscoveryToRingmaster();

         } else {
             packetfunctions_reserveHeaderSize(msg,10);
             msg->payload[0] = 'r';
             msg->payload[1] = 'e';
             msg->payload[2] = 'g';
             msg->payload[3] = 'i';
             msg->payload[4] = 's';
             msg->payload[5] = 't';
             msg->payload[6] = 'e';
             msg->payload[7] = 'r';
             msg->payload[8] = 'e';
             msg->payload[9] = 'd';
         }
}


/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void rrt_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
