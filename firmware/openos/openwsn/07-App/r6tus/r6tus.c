/**
\brief CoAP schedule manager application.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, Feb. 2013.

*/

#include "openwsn.h"
#include "r6tus.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "scheduler.h"
#include "schedule.h"
#include "idmanager.h"

//=========================== defines =========================================

#define TSCH_GET_OPTIONS 4 //how many params in a get request.

const uint8_t r6tus_path0[] = "6tus";

//=========================== variables =======================================

r6tus_vars_t r6tus_vars;

//=========================== prototypes ======================================

owerror_t r6tus_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);

void    r6tus_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void r6tus_init() {
   
   if(idmanager_getIsDAGroot()==TRUE) return; 
   // prepare the resource descriptor for the /r6tus path
   r6tus_vars.desc.path0len            = sizeof(r6tus_path0)-1;
   r6tus_vars.desc.path0val            = (uint8_t*)(&r6tus_path0);
   r6tus_vars.desc.path1len            = 0;
   r6tus_vars.desc.path1val            = NULL;
   r6tus_vars.desc.componentID         = COMPONENT_R6TUS;
   r6tus_vars.desc.callbackRx          = &r6tus_receive;
   r6tus_vars.desc.callbackSendDone    = &r6tus_sendDone;
   
   opencoap_register(&r6tus_vars.desc);
}

//=========================== private =========================================

/**
\brief Receives a command and a list of items to be used by the command.

the coap payload contains, command_type (CREATE,READ,UPDATE,DELETE) number of
items to be processed.
A tuple including address,slotoffset,choffset,whether is shared or not and link
type.

According to the command it returns the list of responses or the required
information.
*/
owerror_t r6tus_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options) {
                        
   uint8_t              i;
   owerror_t              outcome;
   r6tus_command_t*     link_command;
   r6tus_command_t      getResponse;
   slotinfo_element_t*  link_element;
   slotinfo_element_t   getLink_elementResponse;
   open_addr_t          temp_addr;
   owerror_t              responses[R6TUS_MAXRESPONSES];
   //assuming data comes in binary format.
    
   if (coap_header->Code==COAP_CODE_REQ_GET) {
      outcome = E_SUCCESS;    
      // parsing the options from header
      // assuming the following header: /6tus/LinkComandType/targetSlot/targetAddress
      // if (coap_header->OC != TSCH_GET_OPTIONS) {
         //option[0] is 6tus
         getResponse.type=(link_command_t)coap_options[1].pValue[0];
         if (getResponse.type != READ_LINK){
            //fail if this is not a READ REQUEST
            outcome                    = E_FAIL;
            coap_header->Code          = COAP_CODE_RESP_CONTENT;
            //return as this is not the right request.
            return outcome;
         }
         
         getResponse.numelem = 1; //get is always for 1 element.
         getLink_elementResponse.slotOffset=coap_options[2].pValue[0];
         
         switch (coap_options[3].length){
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
               // not supported
               outcome                 = E_FAIL;
               break;
            default:
               outcome                 = E_FAIL;
               break;  
         }
      
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
      // By using the same link_element we don't need to write the packet.
      // It returns the same payload  but with the correct values.
      if (outcome==E_SUCCESS) {
         // write the payload in the response.
         packetfunctions_reserveHeaderSize(msg,sizeof(slotinfo_element_t));
         memcpy(&msg->payload[0],&getLink_elementResponse,sizeof(slotinfo_element_t));
      }
   } else if (coap_header->Code==COAP_CODE_REQ_PUT) {     
      link_command = (r6tus_command_t*) msg->payload; 
      //parsing all cases at post as we want params. once tested we can decide. GET does not accept params
      //so params should be encoded in the url.
      switch (link_command->type){
         case READ_LINK:
            outcome=E_FAIL; 
            //cannot put READ operation
            break;
         case CREATE_LINK:
         case UPDATE_LINK: //update should be post according to REST architecture.
            outcome=E_FAIL; 
            if (link_command->numelem<R6TUS_MAXRESPONSES) {
               for (i=0;i<link_command->numelem;i++) {
                  link_element=(slotinfo_element_t*) &(msg->payload[sizeof(r6tus_command_t)+i*sizeof(slotinfo_element_t)]);
                  temp_addr.type=ADDR_64B;
                  memcpy(&(temp_addr.addr_64b[0]), &(link_element->address[0]),LENGTH_ADDR64b);
                  responses[i]=schedule_addActiveSlot(link_element->slotOffset,link_element->link_type,link_element->shared,link_element->channelOffset,&temp_addr,(link_command->type==UPDATE_LINK));
               }
               outcome=E_SUCCESS; 
            }
            break;
         case DELETE_LINK:
            outcome=E_FAIL; 
            //cannot delete with PUT/POST
            break;
         default:
            openserial_printError(COMPONENT_R6TUS,ERR_COMMAND_NOT_ALLOWED,
               (errorparameter_t)0,
               (errorparameter_t)0
            );
            //who clears the packet??
            //error. Print error and send error msg.
            outcome                    = E_FAIL;
            break; 
      }
      //response of the post
      if (outcome==E_SUCCESS){
         
         // reset packet payload
         msg->payload                  = &(msg->packet[127]);
         msg->length                   = 0;
         //copy the response.
         
         //packetfunctions_reserveHeaderSize(msg,link_command->numelem); 
         //memcpy(&(msg->payload[0]), &(responses[0]),link_command->numelem);
         
         // set the CoAP header
         coap_header->Code             = COAP_CODE_RESP_CONTENT;
      }
   } else if (coap_header->Code==COAP_CODE_REQ_DELETE) {  
      link_command = (r6tus_command_t*) msg->payload; 
      switch (link_command->type){
         case DELETE_LINK:
            outcome=E_FAIL; 
            if (link_command->numelem<R6TUS_MAXRESPONSES){    
               for(i=0;i<link_command->numelem;i++) {
                  link_element=(slotinfo_element_t*) &(msg->payload[sizeof(r6tus_command_t)+i*sizeof(slotinfo_element_t)]);
                  temp_addr.type=ADDR_64B;
                  memcpy(&(temp_addr.addr_64b[0]), &(link_element->address[0]),LENGTH_ADDR64b);
                  //remove the required links.
                  responses[i]=schedule_removeActiveSlot(link_element->slotOffset,&temp_addr);
               }
               outcome=E_SUCCESS; 
            }
            break;
         default:
            openserial_printError(COMPONENT_R6TUS,ERR_COMMAND_NOT_ALLOWED,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
            //who clears the packet??
            //error. Print error and send error msg.        
            outcome                          = E_FAIL;
            break;
      }
   }
   return outcome;
}


void r6tus_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
