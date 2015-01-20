/**
\brief A CoAP resource which indicates the board its running on.
*/


#include "opendefs.h"
#include "rrt.h"
#include "sixtop.h"
#include "idmanager.h"
#include "openqueue.h"
#include "neighbors.h"
#include "packetfunctions.h"
#include "leds.h"
#include "openserial.h"

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
void rrt_setGETRespMsg(
   OpenQueueEntry_t* msg,
   uint8_t discovered   
);

void rrt_sendCoAPMsg(char actionMsg, uint8_t *ipv6mote);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void rrt_init() {
   
   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /rt path
   rrt_vars.desc.path0len             = sizeof(rrt_path0)-1;
   rrt_vars.desc.path0val             = (uint8_t*)(&rrt_path0);
   rrt_vars.desc.path1len             = 0;
   rrt_vars.desc.path1val             = NULL;
   rrt_vars.desc.componentID          = COMPONENT_RRT;
   rrt_vars.desc.callbackRx           = &rrt_receive;
   rrt_vars.desc.callbackSendDone     = &rrt_sendDone;

   rrt_vars.discovered                = 0; //if this mote has been discovered by ringmaster
   
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
   uint8_t mssgRecvd;
   uint8_t moteToSendTo[16];
   uint8_t actionToFwd;

   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         
         //=== reset packet payload (we will reuse this packetBuffer)
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         
         //=== prepare  CoAP response
         rrt_setGETRespMsg(msg, rrt_vars.discovered);
         
         
         // payload marker
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;
         
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;
         
         outcome                          = E_SUCCESS;
         break;
      case COAP_CODE_REQ_PUT:
      case COAP_CODE_REQ_POST:
         mssgRecvd = msg->payload[0];
         
         if (mssgRecvd == 'C') {
            rrt_vars.discovered = 1;
         } else if (mssgRecvd == 'B' && rrt_vars.discovered == 1) {
            //blink mote
            leds_error_toggle();

            //send packet back saying it did action B - blink
            rrt_sendCoAPMsg('B', NULL); //NULL for ringmaster
         } else if (mssgRecvd == 'F' && rrt_vars.discovered == 1) { //format - FB[ipv6]
            actionToFwd = msg->payload[1];
            memcpy(&moteToSendTo, &msg->payload[2], 16);
            rrt_sendCoAPMsg(actionToFwd, moteToSendTo);
         }

         // reset packet payload
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;

         packetfunctions_reserveHeaderSize(msg, 1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;

         //set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;

         outcome                          = E_SUCCESS;
         
         break;
      case COAP_CODE_REQ_DELETE:
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         
         //unregister the current mote as 'discovered' by ringmaster
         rrt_vars.discovered = 0; 
         
         // payload marker
         packetfunctions_reserveHeaderSize(msg,1);
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

void rrt_setGETRespMsg(OpenQueueEntry_t* msg, uint8_t registered) {
         if (registered == 0) {
             packetfunctions_reserveHeaderSize(msg,11);
             msg->payload[0]  =  'r';
             msg->payload[1]  =  'e';
             msg->payload[2]  =  'g';
             msg->payload[3]  =  'i';
             msg->payload[4]  =  's';
             msg->payload[5]  =  't';
             msg->payload[6]  =  'e';
             msg->payload[7]  =  'r';
             msg->payload[8]  =  'i';
             msg->payload[9]  =  'n';
             msg->payload[10] = 'g';

             rrt_sendCoAPMsg('D', NULL); //'D' stands for discovery, 0 for ringmaster

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
 * if mote is 0, then send to the ringmater, defined by ipAddr_ringmaster
**/
void rrt_sendCoAPMsg(char actionMsg, uint8_t *ipv6mote) {
      OpenQueueEntry_t* pkt;
      owerror_t outcome;
      uint8_t numOptions;

      pkt = openqueue_getFreePacketBuffer(COMPONENT_RRT);
      if (pkt == NULL) {
          openserial_printError(COMPONENT_RRT,ERR_BUSY_SENDING,
                                (errorparameter_t)0,
                                (errorparameter_t)0);
          openqueue_freePacketBuffer(pkt);
          return;
      }

      pkt->creator   = COMPONENT_RRT;
      pkt->owner      = COMPONENT_RRT;
      pkt->l4_protocol  = IANA_UDP;

      packetfunctions_reserveHeaderSize(pkt, 1);
      pkt->payload[0] = actionMsg;

      numOptions = 0;
      // location-path option
      packetfunctions_reserveHeaderSize(pkt,sizeof(rrt_path0)-1);
      memcpy(&pkt->payload[0],&rrt_path0,sizeof(rrt_path0)-1);
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
         (sizeof(rrt_path0)-1);
       numOptions++;
      // content-type option
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
         1;
      pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
      numOptions++;

      //metada
      pkt->l4_destination_port   = WKP_UDP_RINGMASTER; 
      pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RINGMASTER;
      pkt->l3_destinationAdd.type = ADDR_128B;
      
      // set destination address here
      if (!ipv6mote) {  //if mote ptr is NULL, then send to ringmaster
        memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_ringmaster, 16);
      } else {
        memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipv6mote[0], 16);
      }

      //send
      outcome = opencoap_send(
              pkt,
              COAP_TYPE_NON,
              COAP_CODE_REQ_PUT,
              numOptions,
              &rrt_vars.desc
              );
      

      if (outcome == E_FAIL) {
        openqueue_freePacketBuffer(pkt);
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