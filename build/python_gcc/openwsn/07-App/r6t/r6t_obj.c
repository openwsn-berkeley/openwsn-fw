/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:02.055219.
*/
/**
\brief CoAP 6top application

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2013.
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, July 2014
*/

#include "openwsn_obj.h"
#include "r6t_obj.h"
#include "sixtop_obj.h"
#include "idmanager_obj.h"
#include "openqueue_obj.h"
#include "neighbors_obj.h"

//=========================== defines =========================================

const uint8_t r6t_path0[] = "6t";

//=========================== variables =======================================

// declaration of global variable _r6t_vars_ removed during objectification.

//=========================== prototypes ======================================

owerror_t r6t_receive(OpenMote* self, 
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void r6t_sendDone(OpenMote* self, 
   OpenQueueEntry_t* msg,
   owerror_t         error
);

//=========================== public ==========================================

void r6t_init(OpenMote* self) {
   if( idmanager_getIsDAGroot(self)==TRUE) return; 
   
   // prepare the resource descriptor for the /6t path
   (self->r6t_vars).desc.path0len            = sizeof(r6t_path0)-1;
   (self->r6t_vars).desc.path0val            = (uint8_t*)(&r6t_path0);
   (self->r6t_vars).desc.path1len            = 0;
   (self->r6t_vars).desc.path1val            = NULL;
   (self->r6t_vars).desc.componentID         = COMPONENT_R6T;
   (self->r6t_vars).desc.callbackRx          = &r6t_receive;
   (self->r6t_vars).desc.callbackSendDone    = &r6t_sendDone;
   
 opencoap_register(self, &(self->r6t_vars).desc);
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
owerror_t r6t_receive(OpenMote* self, 
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
         foundNeighbor = neighbors_getPreferredParentEui64(self, &neighbor);
         if (foundNeighbor==FALSE) {
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_PRECONDFAILED;
            break;
         }
         
         // call sixtop
 sixtop_addCells(self, 
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
         foundNeighbor = neighbors_getPreferredParentEui64(self, &neighbor);
         if (foundNeighbor==FALSE) {
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_PRECONDFAILED;
            break;
         }
         
         // call sixtop
 sixtop_removeCell(self, 
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

void r6t_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}
