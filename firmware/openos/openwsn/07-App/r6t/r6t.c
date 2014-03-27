/**
\brief CoAP schedule manager application.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2013.
*/

#include "openwsn.h"
#include "r6t.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "scheduler.h"
#include "schedule.h"
#include "idmanager.h"

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
   owerror_t error
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

The CoAP payload contains the command_type (CREATE,READ,UPDATE,DELETE) number
of items to be processed.
A tuple including address,slotoffset,choffset,whether is shared or not and link
type.

According to the command, it returns the list of responses or the required
information.

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
   
   uint8_t              i;
   owerror_t            outcome;
   r6t_command_t*     link_command;
   r6t_command_t      getResponse;
   slotinfo_element_t*  link_element;
   slotinfo_element_t   getLink_elementResponse;
   open_addr_t          temp_addr;
   owerror_t            responses[R6T_MAXRESPONSES];
   
   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         
         outcome = E_SUCCESS;
         // parsing the options from header
         // assuming the following header: /6t/LinkComandType/targetSlot/targetAddress
         
         getResponse.type=(link_command_t)coap_options[1].pValue[0];
         
         // abort if not a READ_LINK request
         if (getResponse.type != READ_LINK){
            outcome               = E_FAIL;
            break;
         }
         
         getResponse.numelem = 1; //get is always for 1 element.
         getLink_elementResponse.slotOffset=coap_options[2].pValue[0];
         
         switch (coap_options[3].length) {
            case ADDR_16B:
               temp_addr.type=ADDR_16B;
               memcpy(&(temp_addr.addr_16b[0]), &(coap_options[3].pValue[0]),LENGTH_ADDR16b);
               schedule_getSlotInfo(getLink_elementResponse.slotOffset, &temp_addr, &getLink_elementResponse);
               outcome                 = E_SUCCESS;
               break;
            case ADDR_64B:
               temp_addr.type=ADDR_64B;
               memcpy(&(temp_addr.addr_64b[0]), &(coap_options[3].pValue[0]),LENGTH_ADDR64b);
               schedule_getSlotInfo(getLink_elementResponse.slotOffset, &temp_addr, &getLink_elementResponse);
               outcome                 = E_SUCCESS;
               break;
            case ADDR_128B:
            default:
               outcome                 = E_FAIL;
               break;  
         }
         
         coap_header->Code                = COAP_CODE_RESP_CONTENT;
         
         if (outcome==E_SUCCESS) {
            // write the payload in the response.
            packetfunctions_reserveHeaderSize(msg,1+sizeof(slotinfo_element_t));
            msg->payload[0] = COAP_PAYLOAD_MARKER;
            memcpy(&msg->payload[1],&getLink_elementResponse,sizeof(slotinfo_element_t));
         }
         
         break;
      
      case COAP_CODE_REQ_PUT:
         
         link_command = (r6t_command_t*) msg->payload; 
         
         //so parameters should be encoded in the URI
         switch (link_command->type){
            case READ_LINK:
               outcome=E_FAIL; 
               break;
            case CREATE_LINK:
            case UPDATE_LINK:
               // update should be POST according to REST architecture.
               outcome=E_FAIL; 
               if (link_command->numelem<R6T_MAXRESPONSES) {
                  for (i=0;i<link_command->numelem;i++) {
                     link_element=(slotinfo_element_t*) &(msg->payload[sizeof(r6t_command_t)+i*sizeof(slotinfo_element_t)]);
                     temp_addr.type=ADDR_64B;
                     memcpy(&(temp_addr.addr_64b[0]), &(link_element->address[0]),LENGTH_ADDR64b);
                     responses[i]=schedule_addActiveSlot(link_element->slotOffset,link_element->link_type,link_element->shared,link_element->channelOffset,&temp_addr,(link_command->type==UPDATE_LINK));
                  }
                  outcome=E_SUCCESS; 
               }
               break;
            case DELETE_LINK:
               // cannot delete with PUT/POST
               outcome=E_FAIL; 
               break;
            default:
               openserial_printError(COMPONENT_R6T,ERR_COMMAND_NOT_ALLOWED,
                  (errorparameter_t)0,
                  (errorparameter_t)0
               );
               outcome                    = E_FAIL;
               break; 
         }
         
         if (outcome==E_SUCCESS){
            
            // reset packet payload
            msg->payload                  = &(msg->packet[127]);
            msg->length                   = 0;
            
            // TODO: response?
            
            // set the CoAP header
            coap_header->Code             = COAP_CODE_RESP_CONTENT;
         }
         break;
      case COAP_CODE_REQ_DELETE:
         
         link_command = (r6t_command_t*) msg->payload; 
         switch (link_command->type){
            case DELETE_LINK:
               outcome=E_FAIL; 
               if (link_command->numelem<R6T_MAXRESPONSES){    
                  for(i=0;i<link_command->numelem;i++) {
                     link_element=(slotinfo_element_t*) &(msg->payload[sizeof(r6t_command_t)+i*sizeof(slotinfo_element_t)]);
                     temp_addr.type=ADDR_64B;
                     memcpy(&(temp_addr.addr_64b[0]), &(link_element->address[0]),LENGTH_ADDR64b);
                     //remove the required links.
                     responses[i]=schedule_removeActiveSlot(link_element->slotOffset,&temp_addr);
                  }
                  outcome=E_SUCCESS; 
               }
               break;
            default:
               openserial_printError(
                  COMPONENT_R6T,ERR_COMMAND_NOT_ALLOWED,
                  (errorparameter_t)0,
                  (errorparameter_t)0
               );
               outcome                          = E_FAIL;
               break;
         }
         break;
      default:
         outcome  = E_FAIL;
         break;
   }
   
   return outcome;
}

void r6t_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
