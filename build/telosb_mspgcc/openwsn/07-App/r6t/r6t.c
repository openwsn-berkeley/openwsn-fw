/**
\brief CoAP 6top application

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2013.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, July 2014
*/

#include "openwsn.h"
#include "r6t.h"
#include "sixtop.h"
#include "idmanager.h"
#include "openqueue.h"
#include "neighbors.h"

//=========================== defines =========================================

const uint8_t r6t_path0[] = "6t";

//=========================== variables =======================================

r6t_vars_t r6t_vars;

//=========================== prototypes ======================================

owerror_t r6t_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void    r6t_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void r6t_init() {
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /6t path
   r6t_vars.desc.path0len            = sizeof(r6t_path0)-1;
   r6t_vars.desc.path0val            = (uint8_t*)(&r6t_path0);
   r6t_vars.desc.path1len            = 0;
   r6t_vars.desc.path1val            = NULL;
   r6t_vars.desc.componentID         = COMPONENT_R6T;
   r6t_vars.desc.callbackRx          = &r6t_receive;
   r6t_vars.desc.callbackSendDone    = &r6t_sendDone;
   
   opencoap_register(&r6t_vars.desc);
}

//=========================== private =========================================

/**
\brief Receives a command and a list of items to be used by the command.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t r6t_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   
   owerror_t            outcome;
   open_addr_t          neighbor;
   bool                 foundNeighbor;
   
   switch (coap_header->Code) {
      
      case COAP_CODE_REQ_PUT:
         // add a slot
         
         // reset packet payload
         msg->payload                  = &(msg->packet[127]);
         msg->length                   = 0;
         
         // get preferred parent
         foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
         if (foundNeighbor==FALSE) {
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_PRECONDFAILED;
            break;
         }
         
         // call sixtop
         sixtop_addCells(
            &neighbor,
            1
         );
         
         // set the CoAP header
         coap_header->Code             = COAP_CODE_RESP_CHANGED;
         
         outcome                       = E_SUCCESS;
         break;
      
      case COAP_CODE_REQ_DELETE:
         // delete a slot
         
         // reset packet payload
         msg->payload                  = &(msg->packet[127]);
         msg->length                   = 0;
         
         // get preferred parent
         foundNeighbor = neighbors_getPreferredParentEui64(&neighbor);
         if (foundNeighbor==FALSE) {
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_PRECONDFAILED;
            break;
         }
         
         // call sixtop
         sixtop_removeCell(
            &neighbor
         );
         
         // set the CoAP header
         coap_header->Code             = COAP_CODE_RESP_CHANGED;
         
         outcome                       = E_SUCCESS;
         break;
         
      default:
         outcome = E_FAIL;
         break;
   }
   
   return outcome;
}

void r6t_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
