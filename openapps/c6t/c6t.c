/**
\brief CoAP 6top application.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2013.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, July 2014
*/

#include "opendefs.h"
#include "c6t.h"
#include "sixtop.h"
#include "idmanager.h"
#include "openqueue.h"
#include "neighbors.h"

//=========================== defines =========================================

const uint8_t c6t_path0[] = "6t";

//=========================== variables =======================================

c6t_vars_t c6t_vars;

//=========================== prototypes ======================================

owerror_t c6t_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void    c6t_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void c6t_init() {
   if(idmanager_getIsDAGroot()==TRUE) return; 
   
   // prepare the resource descriptor for the /6t path
   c6t_vars.desc.path0len            = sizeof(c6t_path0)-1;
   c6t_vars.desc.path0val            = (uint8_t*)(&c6t_path0);
   c6t_vars.desc.path1len            = 0;
   c6t_vars.desc.path1val            = NULL;
   c6t_vars.desc.componentID         = COMPONENT_C6T;
   c6t_vars.desc.callbackRx          = &c6t_receive;
   c6t_vars.desc.callbackSendDone    = &c6t_sendDone;
   
   opencoap_register(&c6t_vars.desc);
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
owerror_t c6t_receive(
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

void c6t_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
