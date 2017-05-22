#include "opendefs.h"
#include "opencoap.h"
#include "openqueue.h"
#include "openserial.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "opentimers.h"
#include "scheduler.h"

//=========================== defines =========================================

//=========================== variables =======================================

opencoap_vars_t opencoap_vars;

//=========================== prototype =======================================

owerror_t opencoap_decodeOptions(OpenQueueEntry_t* msg, coap_option_iht* options, uint8_t* payload_index);
owerror_t opencoap_encodeOptions(OpenQueueEntry_t* msg, coap_option_iht* options);

//=========================== public ==========================================

//===== from stack

/**
\brief Initialize this module.
*/
void opencoap_init() {
   // initialize the resource linked list
   opencoap_vars.resources     = NULL;
   
   // initialize the messageID
   opencoap_vars.messageID     = openrandom_get16b();

   // register at UDP stack
   opencoap_vars.desc.port              = WKP_UDP_COAP;
   opencoap_vars.desc.callbackReceive   = &opencoap_receive;
   opencoap_vars.desc.callbackSendDone  = opencoap_sendDone;
   openudp_register(&opencoap_vars.desc);
}

/**
\brief Indicate a CoAP messages was received.

A "CoAP message" is simply a UDP datagram received on the CoAP UDP port.

This function will call the appropriate resource, and send back its answer. The
received packetbuffer is reused to contain the response (or error code).

\param[in] msg The received CoAP message.
*/
void opencoap_receive(OpenQueueEntry_t* msg) {
   uint16_t                  temp_l4_destination_port;
   uint8_t                   i;
   uint8_t                   index;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
   owerror_t                 outcome = 0;
   coap_type_t               response_type;
   // local variables passed to the handlers (with msg)
   coap_header_iht           coap_header;
   coap_option_iht           coap_options[MAX_COAP_OPTIONS];
   uint8_t                   response_options[COAP_RESPONSE_OPTION_SIZE];
   uint8_t                   option_count;
   uint8_t                   option_index;
   
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
      openserial_printError(
         COMPONENT_OPENCOAP,ERR_WRONG_TRAN_PROTOCOL,
         (errorparameter_t)0,
         (errorparameter_t)coap_header.Ver
      );
      openqueue_freePacketBuffer(msg);
      return;
   }
   
   // record the token
   memcpy(&coap_header.token[0], &msg->payload[index], coap_header.TKL);
   index += coap_header.TKL;

   // parse options
   opencoap_decodeOptions(msg, &coap_options[0], &index);

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
      temp_desc = opencoap_vars.resources;

      option_count = opencoap_getOption(&coap_options[0], COAP_OPTION_NUM_URIPATH, &option_index);
      // iterate until matching resource found, or no match
      for (found=FALSE; found==FALSE && temp_desc!=NULL; temp_desc=temp_desc->next) {
         if (temp_desc->path0len > 0 && temp_desc->path0val != NULL) {
            // path0 needed
            if (
               option_count >= 1                                                                         &&
               coap_options[option_index].length == temp_desc->path0len                                  &&
               memcmp(coap_options[option_index].pValue, temp_desc->path0val, temp_desc->path0len) == 0
            ) {
               // path0 matches
               if (temp_desc->path1len > 0 && temp_desc->path1val != NULL) {
                  // path1 needed
                  if (
                     option_count >= 2                                                                          &&
                     coap_options[option_index+1].length == temp_desc->path1len                                 &&
                     memcmp(coap_options[option_index+1].pValue, temp_desc->path1val, temp_desc->path1len) == 0
                  ) {
                     // path1 matches
                     found = TRUE;
                     break;
                  }
               } else {
                  found = TRUE;
                  break;
               }
            }
         }
      }
   
   } else {
      // this is a response: target resource is indicated by token, and message ID
      // if an ack for a confirmable message, or a reset
      // find the resource which matches
      
      // start with the first resource in the linked list
      temp_desc = opencoap_vars.resources;
      
      // iterate until matching resource found, or no match
      while (found==FALSE) {
         
         if (
                coap_header.TKL==temp_desc->last_request.TKL                                       &&
                memcmp(&coap_header.token[0],&temp_desc->last_request.token[0],coap_header.TKL)==0
            ) {
                
            if (coap_header.T==COAP_TYPE_ACK || coap_header.T==COAP_TYPE_RES) {
                if (coap_header.messageID==temp_desc->last_request.messageID) {
                    found=TRUE;
                }
            } else {
                found=TRUE;
            }
            
            // call the resource's callback
            if (found==TRUE && temp_desc->callbackRx!=NULL) {
               temp_desc->callbackRx(msg,&coap_header,&coap_options[0],&response_options[0]);
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
      openqueue_freePacketBuffer(msg);
      
      // stop here: will will not respond to a response
      return;
   }
   
   //=== step 3. ask the resource to prepare response
   
   if (found==TRUE) {
      
      // call the resource's callback
      outcome = temp_desc->callbackRx(msg,&coap_header,&coap_options[0],&response_options[0]);
   } else {
      // reset packet payload (DO NOT DELETE, we will reuse same buffer for response)
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      coap_header.Code                 = COAP_CODE_RESP_NOTFOUND;
   }

   // add options to the response
   opencoap_encodeOptions(msg, &coap_options[0]);
   
   if (outcome==E_FAIL) {
      // reset packet payload (DO NOT DELETE, we will reuse same buffer for response)
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      coap_header.Code                 = COAP_CODE_RESP_METHODNOTALLOWED;
   }

   if (coap_header.T == COAP_TYPE_CON) {
       response_type = COAP_TYPE_ACK;
   } else {
       response_type = COAP_TYPE_NON;
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
   packetfunctions_reserveHeaderSize(msg,4+coap_header.TKL);
   msg->payload[0]                  = (COAP_VERSION    << 6) |
                                      (response_type   << 4) |
                                      (coap_header.TKL << 0);
   msg->payload[1]                  = coap_header.Code;
   msg->payload[2]                  = coap_header.messageID/256;
   msg->payload[3]                  = coap_header.messageID%256;
   memcpy(&msg->payload[4], &coap_header.token[0], coap_header.TKL);
   
   if ((openudp_send(msg))==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

/**
\brief Indicates that the CoAP response has been sent.

\param[in] msg A pointer to the message which was sent.
\param[in] error The outcome of the send function.
*/
void opencoap_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   coap_resource_desc_t* temp_resource;
   
   // take ownership over that packet
   msg->owner = COMPONENT_OPENCOAP;
   
   // indicate sendDone to creator of that packet
   //=== mine
   if (msg->creator==COMPONENT_OPENCOAP) {
      openqueue_freePacketBuffer(msg);
      return;
   }
   //=== someone else's
   temp_resource = opencoap_vars.resources;
   while (temp_resource!=NULL) {
      if (
         temp_resource->componentID==msg->creator &&
         temp_resource->callbackSendDone!=NULL
         ) {
         temp_resource->callbackSendDone(msg,error);
         return;
      }
      temp_resource = temp_resource->next;
   }
   
   // if you get here, no valid creator was found
   
   openserial_printError(
      COMPONENT_OPENCOAP,ERR_UNEXPECTED_SENDDONE,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
   openqueue_freePacketBuffer(msg);
}

//===== from CoAP resources

/**
\brief Writes the links to all the resources on this mote into the message.

\param[out] msg The messge to write the links to.
\param[in] componentID The componentID calling this function.

\post After this function returns, the msg contains 
*/
void opencoap_writeLinks(OpenQueueEntry_t* msg, uint8_t componentID) {
   coap_resource_desc_t* temp_resource;
   
   // start with the first resource in the linked list
   temp_resource = opencoap_vars.resources;
   
   // iterate through all resources
   while (temp_resource!=NULL) {
      
      if (  
            (temp_resource->discoverable==TRUE) &&
            (
               ((componentID==COMPONENT_CWELLKNOWN) && (temp_resource->path1len==0))
               || 
               ((componentID==temp_resource->componentID) && (temp_resource->path1len!=0))
            )
         ) {
          
         // write ending '>'
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0] = '>';
         
         // write path1
         if (temp_resource->path1len>0) {
            packetfunctions_reserveHeaderSize(msg,temp_resource->path1len);
            memcpy(&msg->payload[0],temp_resource->path1val,temp_resource->path1len);
            packetfunctions_reserveHeaderSize(msg,1);
            msg->payload[0] = '/';
         }
         
         // write path0
         packetfunctions_reserveHeaderSize(msg,temp_resource->path0len);
         memcpy(msg->payload,temp_resource->path0val,temp_resource->path0len);
         packetfunctions_reserveHeaderSize(msg,2);
         msg->payload[1] = '/';
         
         // write opening '>'
         msg->payload[0] = '<';
         
         // write separator between links
         if (temp_resource->next!=NULL) {
            packetfunctions_reserveHeaderSize(msg,1);
            msg->payload[0] = ',';
         }
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
void opencoap_register(coap_resource_desc_t* desc) {
   coap_resource_desc_t* last_elem;
   
   // since this CoAP resource will be at the end of the list, its next element
   // should point to NULL, indicating the end of the linked list.
   desc->next = NULL;
   
   // if this is the first resource, simply have resources point to it
   if (opencoap_vars.resources==NULL) {
      opencoap_vars.resources = desc;
      return;
   }
   
   // if not, add to the end of the resource linked list
   last_elem = opencoap_vars.resources;
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
\param[in] TKL  The Token Length of the message, sanitized to a max of COAP_MAX_TKL (8).
\param[out] descSender A pointer to the description of the calling CoAP
   resource.

\post After returning, this function will have written the messageID and TOKEN
   used in the descSender parameter.

\return The outcome of sending the packet.
*/
owerror_t opencoap_send(
      OpenQueueEntry_t*      msg,
      coap_type_t            type,
      coap_code_t            code,
      uint8_t                TKL,
      coap_resource_desc_t*  descSender
   ) {
   uint16_t token;
   uint8_t tokenPos=0;
   coap_header_iht* request;
   
   // increment the (global) messageID
   if (opencoap_vars.messageID++ == 0xffff) {
      opencoap_vars.messageID = 0;
   }
   
   // take ownership over the packet
   msg->owner                       = COMPONENT_OPENCOAP;
   
   // fill in packet metadata
   msg->l4_sourcePortORicmpv6Type   = WKP_UDP_COAP;
   
   // update the last_request header
   request                          = &descSender->last_request;
   request->T                       = type;
   request->Code                    = code;
   request->messageID               = opencoap_vars.messageID;
   request->TKL                     = TKL<COAP_MAX_TKL ? TKL : COAP_MAX_TKL;
   
   while (tokenPos<request->TKL) {
       token = openrandom_get16b();
       memcpy(&request->token[tokenPos],&token,2);
       tokenPos+=2;
   }
   
   // pre-pend CoAP header (version,type,TKL,code,messageID,Token)
   packetfunctions_reserveHeaderSize(msg,4+request->TKL);
   msg->payload[0]                  = (COAP_VERSION   << 6) |
                                      (type           << 4) |
                                      (request->TKL   << 0);
   msg->payload[1]                  = code;
   msg->payload[2]                  = (request->messageID>>8) & 0xff;
   msg->payload[3]                  = (request->messageID>>0) & 0xff;

   memcpy(&msg->payload[4],&token,request->TKL);
   
   return openudp_send(msg);
}

//=========================== private =========================================

owerror_t opencoap_decodeOptions(OpenQueueEntry_t* msg, coap_option_iht* options, uint8_t* payload_index) {
   uint8_t       index = *payload_index;
   uint8_t       skip_bytes;
   uint8_t       i;
   uint8_t       delta, length;
   coap_option_t last_option;

   // initialize the coap_options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      options[i].type = COAP_OPTION_NONE;
      options[i].inResponse = 0;
   }

   // fill in the coap_options
   last_option = COAP_OPTION_NONE;
   for (i=0;i<MAX_COAP_OPTIONS;i++) {

      // detect when done parsing options
      if (msg->payload[index] == COAP_PAYLOAD_MARKER) {
         // found the payload marker, done parsing options.
         index++; // skip marker and stop parsing options
         break;
      }
      if (msg->length <= index) {
         // end of message, no payload
         break;
      }

      // parse this option
      delta = (msg->payload[index] & 0xf0) >> 4;
      skip_bytes = 0;
      if (delta < 13) {
         options[i].type = (coap_option_t)((uint8_t)last_option+delta);
      } else if (delta == 13) {
         options[i].type = (coap_option_t)(13+(uint8_t)last_option+(uint8_t)(msg->payload[index+1]));
         skip_bytes += 1;
      } else if (delta == 14) {
         skip_bytes += 2;
         // can't handle large option numbers
      } else {
         // error
         return E_FAIL;
      }
      length = msg->payload[index] & 0x0f;
      if (length < 13) {
         options[i].length = length;
      } else if (length == 13) {
         options[i].length = msg->payload[index+skip_bytes+1] + 13;
         skip_bytes += 1;
      } else if (length == 14) {
         skip_bytes += 2;
         // can't handle large option lengths
      } else {
         // error
         return E_FAIL;
      }
      index += skip_bytes;
      index++;
      options[i].pValue      = &(msg->payload[index]);
      index                  += options[i].length; //includes length as well
      last_option            = options[i].type;
   }

   // remove the CoAP header+options
   packetfunctions_tossHeader(msg, index);

   *payload_index = index;
   return E_SUCCESS;
}

owerror_t opencoap_encodeOptions(OpenQueueEntry_t* msg, coap_option_iht* options) {
   uint8_t i, found_index;
   coap_option_t last_type = COAP_OPTION_NONE;
   uint8_t found_type; // find the smallest type in the options
   uint8_t byte_count = 0;
   uint8_t* ptr;

   // first calculate how many bytes we have to reserve for the options
   while (1) {
      found_type = 0xff;
      for(i=0; i<MAX_COAP_OPTIONS; i++) {
         if (
               options[i].inResponse == 1              &&
               (uint8_t)(options[i].type) < found_type
          ) {
            found_index = i;
            found_type = (uint8_t)(options[i].type);
         }
      }
      if (found_type == 0xff) {
         break;
      }
      options[found_index].inResponse = 2;

      byte_count++; // normal option delta and length field
      if (found_type - last_type >= 13) {
         byte_count++; // extended delta
      }
      if (options[found_index].length >= 13) {
         byte_count++; // extended length
      }

      byte_count += options[found_index].length;
      last_type = found_type;
   }

   packetfunctions_reserveHeaderSize(msg, byte_count);
   ptr = msg->payload;

   last_type = COAP_OPTION_NONE;
   // now we can insert the options in increasing order
   while (1) {
      found_type = 0xff;
      for (i=0; i<MAX_COAP_OPTIONS; i++) {
         if (
            options[i].inResponse == 2              &&
            (uint8_t)(options[i].type) < found_type
         ) {
            found_index = i;
            found_type = (uint8_t)(options[i].type);
         }
      }
      if (found_type == 0xff) {
         break;
      }

      byte_count = 0;
      if (found_type - last_type < 13) {
         ptr[0] = (found_type - last_type) << 4;
      } else {
         ptr[0] = 13 << 4;
         ptr[1] = found_type - last_type - 13;
         byte_count += 1;
      }

      if (options[found_index].length < 13) {
         ptr[0] |= options[found_index].length & 0x0f;
      } else {
         ptr[0] |= 13;
         ptr[byte_count+1] = options[found_index].length - 13;
         byte_count += 1;
      }

      ptr++; // normal delta and length field
      ptr += byte_count; // extended fields
      if (options[found_index].pValue != NULL) {
         memcpy(ptr, options[found_index].pValue, options[found_index].length);
      }
      ptr += options[found_index].length;

      options[found_index].inResponse = 1; // don't find this again
      last_type = found_type;
   }
   return E_SUCCESS;
}

// sets start_idx to the index of the first option of type key, returns the number of options of this type
uint8_t opencoap_getOption(coap_option_iht* options, coap_option_t key, uint8_t* start_idx) {
   uint8_t i, j;

   for (i=0; i<MAX_COAP_OPTIONS; i++) {
      if (options[i].type == key) {
         // skip until different type
         for (j=i; j<MAX_COAP_OPTIONS && options[j].type == key; j++) {}
         if (start_idx != NULL) {
            *start_idx = i;
         }
         return j-i;
      }
   }
   if (start_idx != NULL) {
      *start_idx = 0;
   }
   return 0;
}

// overwrites options from the beginning of the array, if the existing option
// was set in the request
// This function does *not* have to be called in the order of increasing option types
owerror_t opencoap_setOption(coap_option_iht* options, coap_option_t type, uint8_t length, uint8_t* value) {
   uint8_t i;
   // skip existing response options
   for (i=0; i<MAX_COAP_OPTIONS && options[i].inResponse; i++) {}
   if (i >= MAX_COAP_OPTIONS) {
      // error too many options
      return E_FAIL;
   }
   options[i].type = type;
   options[i].length = length;
   options[i].pValue = value;
   options[i].inResponse = 1;

   return E_SUCCESS;
}

// vim: ts=3 sw=3
