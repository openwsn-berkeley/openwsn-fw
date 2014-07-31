/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:45.924828.
*/
#include "openwsn_obj.h"
#include "opencoap_obj.h"
#include "openudp_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"
#include "openrandom_obj.h"
#include "packetfunctions_obj.h"
#include "idmanager_obj.h"
#include "opentimers_obj.h"
#include "scheduler_obj.h"

//=========================== defines =========================================

#define COAP_TOKEN 123 // TODO: make dynamic

//=========================== variables =======================================

// declaration of global variable _opencoap_vars_ removed during objectification.

//=========================== prototype =======================================

//=========================== public ==========================================

//===== from stack

/**
\brief Initialize this module.
*/
void opencoap_init(OpenMote* self) {
   // initialize the resource linked list
   (self->opencoap_vars).resources     = NULL;
   
   // initialize the messageID
   (self->opencoap_vars).messageID     = openrandom_get16b(self);
}

/**
\brief Indicate a CoAP messages was received.

A "CoAP message" is simply a UDP datagram received on the CoAP UDP port.

This function will call the appropriate resource, and send back its answer. The
received packetbuffer is reused to contain the response (or error code).

\param[in] msg The received CoAP message.
*/
void opencoap_receive(OpenMote* self, OpenQueueEntry_t* msg) {
   uint16_t                  temp_l4_destination_port;
   uint8_t                   i;
   uint8_t                   index;
   coap_option_t             last_option;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
   owerror_t                 outcome;
   // local variables passed to the handlers (with msg)
   coap_header_iht           coap_header;
   coap_option_iht           coap_options[MAX_COAP_OPTIONS];
   
   // take ownership over the received packet
   msg->owner                = COMPONENT_OPENCOAP;
   
   //=== step 1. parse the packet
   
   // parse the CoAP header and remove from packet
   index = 0;
   coap_header.Ver           = (msg->payload[index] & 0xc0) >> 6;
   coap_header.T             = (coap_type_t)((msg->payload[index] & 0x30) >> 4);
   coap_header.TKL           = (msg->payload[index] & 0x0f);
   index++;
   coap_header.Code          = (coap_code_t)(msg->payload[index]);
   index++;
   coap_header.messageID     = msg->payload[index]*256+msg->payload[index+1];
   index+=2;
   
   // reject unsupported header
   if (coap_header.Ver!=COAP_VERSION || coap_header.TKL>COAP_MAX_TKL) {
// openserial_printError(self, 
//         COMPONENT_OPENCOAP,ERR_WRONG_TRAN_PROTOCOL,
//         (errorparameter_t)0,
//         (errorparameter_t)coap_header.Ver
//      );
 openqueue_freePacketBuffer(self, msg);
      return;
   }
   
   // record the token
   memcpy(&coap_header.token[0], &msg->payload[index], coap_header.TKL);
   index += coap_header.TKL;
   
   // initialize the coap_options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      coap_options[i].type = COAP_OPTION_NONE;
   }
   
   // fill in the coap_options
   last_option = COAP_OPTION_NONE;
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      
      // detect when done parsing options
      if (msg->payload[index]==COAP_PAYLOAD_MARKER){
         // found the payload marker, done parsing options.
         index++; // skip marker and stop parsing options
         break;
      }
      
      // parse this option
      coap_options[i].type        = (coap_option_t)((uint8_t)last_option+(uint8_t)((msg->payload[index] & 0xf0) >> 4));
      last_option                 = coap_options[i].type;
      coap_options[i].length      = (msg->payload[index] & 0x0f);
      index++;
      coap_options[i].pValue      = &(msg->payload[index]);
      index                      += coap_options[i].length; //includes length as well
   }
   
   // remove the CoAP header+options
 packetfunctions_tossHeader(self, msg,index);
   
   //=== step 2. find the resource to handle the packet
   
   // find the resource this applies to
   found = FALSE;
   
   if (
         coap_header.Code>=COAP_CODE_REQ_GET &&
         coap_header.Code<=COAP_CODE_REQ_DELETE
      ) {
      // this is a request: target resource is indicated as COAP_OPTION_LOCATIONPATH option(s)
      // find the resource which matches
      
      // start with the first resource in the linked list
      temp_desc = (self->opencoap_vars).resources;
      
      // iterate until matching resource found, or no match
      while (found==FALSE) {
         if (
               coap_options[0].type==COAP_OPTION_NUM_URIPATH    &&
               coap_options[1].type==COAP_OPTION_NUM_URIPATH    &&
               temp_desc->path0len>0                            &&
               temp_desc->path0val!=NULL                        &&
               temp_desc->path1len>0                            &&
               temp_desc->path1val!=NULL
            ) {
            // resource has a path of form path0/path1
               
            if (
                  coap_options[0].length==temp_desc->path0len                               &&
                  memcmp(coap_options[0].pValue,temp_desc->path0val,temp_desc->path0len)==0 &&
                  coap_options[1].length==temp_desc->path1len                               &&
                  memcmp(coap_options[1].pValue,temp_desc->path1val,temp_desc->path1len)==0
               ) {
               found = TRUE;
            };
         
         } else if (
               coap_options[0].type==COAP_OPTION_NUM_URIPATH    &&
               temp_desc->path0len>0                            &&
               temp_desc->path0val!=NULL
            ) {
            // resource has a path of form path0
               
            if (
                  coap_options[0].length==temp_desc->path0len                               &&
                  memcmp(coap_options[0].pValue,temp_desc->path0val,temp_desc->path0len)==0
               ) {
               found = TRUE;
            };
         };
         
         // iterate to next resource, if not found
         if (found==FALSE) {
            if (temp_desc->next!=NULL) {
               temp_desc = temp_desc->next;
            } else {
               break;
            }
         }
      }
   
   } else {
      // this is a response: target resource is indicated by message ID
      // find the resource which matches
      
      // start with the first resource in the linked list
      temp_desc = (self->opencoap_vars).resources;
      
      // iterate until matching resource found, or no match
      while (found==FALSE) {
         
         if (coap_header.messageID==temp_desc->messageID) {
            found=TRUE;
            
            // call the resource's callback
            if (temp_desc->callbackRx!=NULL) {
               temp_desc->callbackRx(self, msg,&coap_header,&coap_options[0]);
            }
         }
         
         // iterate to next resource, if not found
         if (found==FALSE) {
            if (temp_desc->next!=NULL) {
               temp_desc = temp_desc->next;
            } else {
               break;
            }
         }
      };
      
      // free the received packet
 openqueue_freePacketBuffer(self, msg);
      
      // stop here: will will not respond to a response
      return;
   }
   
   //=== step 3. ask the resource to prepare response
   
   if (found==TRUE) {
      
      // call the resource's callback
      outcome = temp_desc->callbackRx(self, msg,&coap_header,&coap_options[0]);
   } else {
      // reset packet payload (DO NOT DELETE, we will reuse same buffer for response)
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      coap_header.Code                 = COAP_CODE_RESP_NOTFOUND;
   }
   
   if (outcome==E_FAIL) {
      // reset packet payload (DO NOT DELETE, we will reuse same buffer for response)
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      coap_header.Code                 = COAP_CODE_RESP_METHODNOTALLOWED;
   }
   
   //=== step 4. send that packet back
   
   // fill in packet metadata
   if (found==TRUE) {
      msg->creator                     = temp_desc->componentID;
   } else {
      msg->creator                     = COMPONENT_OPENCOAP;
   }
   msg->l4_protocol                    = IANA_UDP;
   temp_l4_destination_port            = msg->l4_destination_port;
   msg->l4_destination_port            = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type      = temp_l4_destination_port;
   
   // set destination address as the current source
   msg->l3_destinationAdd.type         = ADDR_128B;
   memcpy(&msg->l3_destinationAdd.addr_128b[0],&msg->l3_sourceAdd.addr_128b[0],LENGTH_ADDR128b);
   
   // fill in CoAP header
 packetfunctions_reserveHeaderSize(self, msg,4+coap_header.TKL);
   msg->payload[0]                  = (COAP_VERSION    << 6) |
                                      (COAP_TYPE_ACK   << 4) |
                                      (coap_header.TKL << 0);
   msg->payload[1]                  = coap_header.Code;
   msg->payload[2]                  = coap_header.messageID/256;
   msg->payload[3]                  = coap_header.messageID%256;
   memcpy(&msg->payload[4], &coap_header.token[0], coap_header.TKL);
   
   if (( openudp_send(self, msg))==E_FAIL) {
 openqueue_freePacketBuffer(self, msg);
   }
}

/**
\brief Indicates that the CoAP response has been sent.

\param[in] msg A pointer to the message which was sent.
\param[in] error The outcome of the send function.
*/
void opencoap_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   coap_resource_desc_t* temp_resource;
   
   // take ownership over that packet
   msg->owner = COMPONENT_OPENCOAP;
   
   // indicate sendDone to creator of that packet
   //=== mine
   if (msg->creator==COMPONENT_OPENCOAP) {
 openqueue_freePacketBuffer(self, msg);
      return;
   }
   //=== someone else's
   temp_resource = (self->opencoap_vars).resources;
   while (temp_resource!=NULL) {
      if (
         temp_resource->componentID==msg->creator &&
         temp_resource->callbackSendDone!=NULL
         ) {
         temp_resource->callbackSendDone(self, msg,error);
         return;
      }
      temp_resource = temp_resource->next;
   }
   
   // if you get here, no valid creator was found
   
// openserial_printError(self, 
//      COMPONENT_OPENCOAP,ERR_UNEXPECTED_SENDDONE,
//      (errorparameter_t)0,
//      (errorparameter_t)0
//   );
 openqueue_freePacketBuffer(self, msg);
}

//===== from CoAP resources

/**
\brief Writes the links to all the resources on this mote into the message.

\param[out] msg The messge to write the links to.

\post After this function returns, the msg contains 
*/
void opencoap_writeLinks(OpenMote* self, OpenQueueEntry_t* msg) {
   coap_resource_desc_t* temp_resource;
   
   // start with the first resource in the linked list
   temp_resource = (self->opencoap_vars).resources;
   
   // iterate through all resources
   while (temp_resource!=NULL) {
      
      // write ending '>'
 packetfunctions_reserveHeaderSize(self, msg,1);
      msg->payload[0] = '>';
      
      // write path1
      if (temp_resource->path1len>0) {
 packetfunctions_reserveHeaderSize(self, msg,temp_resource->path1len);
         memcpy(&msg->payload[0],temp_resource->path1val,temp_resource->path1len);
 packetfunctions_reserveHeaderSize(self, msg,1);
         msg->payload[0] = '/';
      }
      
      // write path0
 packetfunctions_reserveHeaderSize(self, msg,temp_resource->path0len);
      memcpy(msg->payload,temp_resource->path0val,temp_resource->path0len);
 packetfunctions_reserveHeaderSize(self, msg,2);
      msg->payload[1] = '/';
      
      // write opening '>'
      msg->payload[0] = '<';
      
      // write separator between links
      if (temp_resource->next!=NULL) {
 packetfunctions_reserveHeaderSize(self, msg,1);
         msg->payload[0] = ',';
      }
      
      // iterate to next resource
      temp_resource = temp_resource->next;
   }
}

/**
\brief Register a new CoAP resource.

This function is called by a CoAP resource when it starts, allowing it to
receive data sent to that resource.

Registration consists in adding a new resource at the end of the linked list
of resources.

\param[in] desc The description of the CoAP resource.
*/
void opencoap_register(OpenMote* self, coap_resource_desc_t* desc) {
   coap_resource_desc_t* last_elem;
   
   // since this CoAP resource will be at the end of the list, its next element
   // should point to NULL, indicating the end of the linked list.
   desc->next = NULL;
   
   // if this is the first resource, simply have resources point to it
   if ((self->opencoap_vars).resources==NULL) {
      (self->opencoap_vars).resources = desc;
      return;
   }
   
   // if not, add to the end of the resource linked list
   last_elem = (self->opencoap_vars).resources;
   while (last_elem->next!=NULL) {
      last_elem = last_elem->next;
   }
   last_elem->next = desc;
}

/**
\brief Send a CoAP request.

This function is called by a CoAP resource when it wants to send some data.
This function is NOT called for a response.

\param[in] msg The message to be sent. This messages should not contain the
   CoAP header.
\param[in] type The CoAP type of the message.
\param[in] code The CoAP code of the message.
\param[in] TKL  The Token Length of the message.
\param[out] descSender A pointer to the description of the calling CoAP
   resource.

\post After returning, this function will have written the messageID and TOKEN
   used in the descSender parameter.

\return The outcome of sending the packet.
*/
owerror_t opencoap_send(OpenMote* self, 
      OpenQueueEntry_t*      msg,
      coap_type_t            type,
      coap_code_t            code,
      uint8_t                TKL,
      coap_resource_desc_t*  descSender
   ) {
   
   // pick a new (global) messageID
   (self->opencoap_vars).messageID          = openrandom_get16b(self);
   
   // take ownership over the packet
   msg->owner                       = COMPONENT_OPENCOAP;
   
   // fill in packet metadata
   msg->l4_sourcePortORicmpv6Type   = WKP_UDP_COAP;
   
   // pre-pend CoAP header (version,type,TKL,code,messageID,Token)
 packetfunctions_reserveHeaderSize(self, msg,5);
   msg->payload[0]                  = (COAP_VERSION   << 6) |
                                      (type           << 4) |
                                      (TKL            << 0);
   msg->payload[1]                  = code;
   msg->payload[2]                  = ((self->opencoap_vars).messageID>>8) & 0xff;
   msg->payload[3]                  = ((self->opencoap_vars).messageID>>0) & 0xff;
   msg->payload[4]                  = COAP_TOKEN;
  
   // indicate the messageID used to the sender
   descSender->messageID            = (self->opencoap_vars).messageID;
   descSender->token                = COAP_TOKEN;
   
   return openudp_send(self, msg);
}

//=========================== private =========================================
