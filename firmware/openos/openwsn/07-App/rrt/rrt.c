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

void sendMsgToRingmaster(char actionMsg);

void sendMsgToMote(char actionMsg, uint8_t mote);

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
   uint8_t mssgRecvd;
   uint8_t nextMoteIfNeeded;
   uint8_t actionToFwd;

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
      case COAP_CODE_REQ_POST:
         mssgRecvd = msg->payload[0];
         
         printf("message received: %i\n", mssgRecvd);

         if (mssgRecvd == 'C') {
            rrt_vars.discovered = TRUE;
         } else if (mssgRecvd == 'B') {
            //blink mote
            printf("Mote performed blink\n");
            //send packet back saying it did action B - blink
            //TODO - call blink method here

            sendMsgToRingmaster('B');
         } else if (mssgRecvd == 'F') {
            nextMoteIfNeeded = msg->payload[1];
            actionToFwd = msg->payload[2];
            sendMsgToMote(actionToFwd, nextMoteIfNeeded);
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

         rrt_vars.discovered = FALSE;
         
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

             sendMsgToRingmaster('D'); //'D' stands for discovery

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

void sendMsgToRingmaster(char actionMsg) {
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
         sizeof(rrt_path0)-1;
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
      memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_ringmaster, 16);
      //send
      outcome = opencoap_send(pkt,
                              COAP_TYPE_CON,
                              COAP_CODE_REQ_PUT,
                              numOptions,
                              &rrt_vars.desc);
      

      if (outcome == E_FAIL) {
        openqueue_freePacketBuffer(pkt);
      }
}

void sendMsgToMote(char actionMsg, uint8_t mote) {
      OpenQueueEntry_t* pkt;
      owerror_t outcome;
      uint8_t numOptions;

      printf("sending mssg to mote: %i\n", mote);

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
         sizeof(rrt_path0)-1;
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
      memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_simMotes, 16);
      pkt->l3_destinationAdd.addr_128b[15] = (mote - 0x30);
      //send
      outcome = openudp_send(pkt);
      

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
