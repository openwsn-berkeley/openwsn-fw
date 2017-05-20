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

#define MAX_BLOCK_SIZE 64

//=========================== variables =======================================

opencoap_vars_t opencoap_vars;
opencoap_block_transfer_t opencoap_block_transfers[COAP_BLOCK_TRANSFERS];

//=========================== prototype =======================================

void opencoap_dropBlockTransfer(opencoap_block_transfer_t* bt_ptr);
opencoap_block_transfer_t* opencoap_lookupBlockTransfer(OpenQueueEntry_t* msg, coap_option_iht* coap_options, uint8_t p0_idx, uint8_t p1_idx);
opencoap_block_transfer_t* opencoap_createBlockTransfer(OpenQueueEntry_t* msg, coap_option_iht* coap_options, uint8_t p0_idx, uint8_t p1_idx);

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
   coap_option_t             last_option;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
   owerror_t                 outcome = E_SUCCESS;
   coap_type_t               response_type;
   // local variables passed to the handlers (with msg)
   coap_header_iht           coap_header;
   coap_option_iht           coap_options[MAX_COAP_OPTIONS];
   uint8_t                   b2_idx = MAX_COAP_OPTIONS; // index of Block2 option in coap_options
   uint8_t                   uripath0_idx = MAX_COAP_OPTIONS;
   uint8_t                   uripath1_idx = MAX_COAP_OPTIONS;
   uint8_t                   uripath2_idx = MAX_COAP_OPTIONS;
   opencoap_block_transfer_t* bt_ptr = NULL;
   bool                      response_too_big = FALSE;
   bool                      reused_packet = FALSE;
   uint32_t                  b2_blockNumber = 0;
   bool                      b2_moreBlocks;
   uint16_t                  b2_blockSize;
   uint8_t                   max_blockNumber;

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
   
   // initialize the coap_options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      coap_options[i].type = COAP_OPTION_NONE;
   }
   
   // fill in the coap_options
   last_option = COAP_OPTION_NONE;
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      
      // detect when done parsing options
      if (msg->payload[index]==COAP_PAYLOAD_MARKER) {
         // found the payload marker, done parsing options.
         index++; // skip marker and stop parsing options
         break;
      }
      if (msg->length<=index) {
         // end of message, no payload
         break;
      }
      
      // parse this option
      coap_options[i].type        = (coap_option_t)((uint8_t)last_option+(uint8_t)((msg->payload[index] & 0xf0) >> 4));
      last_option                 = coap_options[i].type;
      coap_options[i].length      = (msg->payload[index] & 0x0f);
      index++;
      coap_options[i].pValue      = &(msg->payload[index]);
      index                      += coap_options[i].length; //includes length as well

      switch(coap_options[i].type) {
         case COAP_OPTION_NUM_URIPATH:
            if (uripath0_idx == MAX_COAP_OPTIONS) {
               uripath0_idx = i;
            } else if (uripath1_idx == MAX_COAP_OPTIONS) {
               uripath1_idx = i;
            } else if (uripath2_idx == MAX_COAP_OPTIONS) {
               uripath2_idx = i;
            }
            break;
         case COAP_OPTION_NUM_BLOCK2:
            if (b2_idx == MAX_COAP_OPTIONS) {
               b2_idx = i;
            }
            break;
         default:
            break;
      }
   }
   
   // remove the CoAP header+options
   packetfunctions_tossHeader(msg,index);
   
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
      
      // iterate until matching resource found, or no match
      while (found==FALSE) {
         if (
               uripath0_idx != MAX_COAP_OPTIONS                 &&
               uripath1_idx != MAX_COAP_OPTIONS                 &&
               temp_desc->path0len>0                            &&
               temp_desc->path0val!=NULL                        &&
               temp_desc->path1len>0                            &&
               temp_desc->path1val!=NULL
            ) {
            // resource has a path of form path0/path1
               
            if (
                  coap_options[uripath0_idx].length==temp_desc->path0len                               &&
                  memcmp(coap_options[uripath0_idx].pValue,temp_desc->path0val,temp_desc->path0len)==0 &&
                  coap_options[uripath1_idx].length==temp_desc->path1len                               &&
                  memcmp(coap_options[uripath1_idx].pValue,temp_desc->path1val,temp_desc->path1len)==0
               ) {
               found = TRUE;
            };
         
         } else if (
               uripath0_idx != MAX_COAP_OPTIONS                 &&
               temp_desc->path0len>0                            &&
               temp_desc->path0val!=NULL
            ) {
            // resource has a path of form path0
               
            if (
                  coap_options[uripath0_idx].length==temp_desc->path0len                               &&
                  memcmp(coap_options[uripath0_idx].pValue,temp_desc->path0val,temp_desc->path0len)==0
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
               temp_desc->callbackRx(msg,&coap_header,&coap_options[0]);
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
      response_too_big = FALSE;
      b2_blockSize = MAX_BLOCK_SIZE; //default maximum

      // parse Block2 option
      if ( b2_idx != MAX_COAP_OPTIONS ) {
         if (coap_options[b2_idx].length == 0) {
            b2_blockNumber = 0;
         } else if (coap_options[b2_idx].length == 1) {
            b2_blockNumber = (uint8_t)(coap_options[b2_idx].pValue[0]);
         } else if (coap_options[b2_idx].length == 2) {
            b2_blockNumber = (uint8_t)(coap_options[b2_idx].pValue[0]) << 8;
            b2_blockNumber |= (uint8_t)(coap_options[b2_idx].pValue[1]);
         } else if (coap_options[b2_idx].length == 3) {
            b2_blockNumber = (uint8_t)(coap_options[b2_idx].pValue[0]) << 16;
            b2_blockNumber |= (uint8_t)(coap_options[b2_idx].pValue[1]) << 8;
            b2_blockNumber |= (uint8_t)(coap_options[b2_idx].pValue[2]);
         }

         b2_moreBlocks = (b2_blockNumber & 0x8) > 0;
         b2_blockSize = 1 << ((b2_blockNumber & 0x7) + 4); // 16 B -- 1 kB, 0b111 is reserved
         b2_blockSize = b2_blockSize>MAX_BLOCK_SIZE ? MAX_BLOCK_SIZE : b2_blockSize;
         b2_blockNumber = b2_blockNumber >> 4;
      }

      // lookup (client, URI) in block_transfer
      bt_ptr = opencoap_lookupBlockTransfer(msg, coap_options, uripath0_idx, uripath1_idx);

      // check if we should forget a previous block transfer
      if ( b2_idx == MAX_COAP_OPTIONS && bt_ptr != NULL ) {
         opencoap_dropBlockTransfer(bt_ptr);
         bt_ptr = NULL;
      }

      // we need to generate data if we can't load it from bt_ptr
      if ( bt_ptr == NULL ) {
         // create response
         outcome = temp_desc->callbackRx(msg,&coap_header,&coap_options[0]);

         response_too_big = msg->length > b2_blockSize; // this could also be up to roughly 71 Bytes
         reused_packet = (msg->payload >= &(msg->packet[0])) && (msg->payload - &(msg->packet[0]) < 127); //msg->payload points into msg->packet

         if ( response_too_big && !reused_packet) {
            // create block_transfer
            bt_ptr = opencoap_createBlockTransfer(msg, &(coap_options[0]), uripath0_idx, uripath1_idx);
            b2_blockNumber = 0;
            if (bt_ptr == NULL) {
               // TODO: print error
            }
         }
      } else {
         msg->payload = bt_ptr->data;
         msg->length = bt_ptr->dataLen;
         response_too_big = TRUE; // TRUE, because the complete payload is bigger than a block
         reused_packet = FALSE; // FALSE, because bt_ptr->data doesn't point to msg->packet
         coap_header.Code = COAP_CODE_RESP_CONTENT;
         outcome = E_SUCCESS;
      }

      // assert: msg->payload points to the complete data that we want to send

      // assert that we don't read past the generated data if the block number is too high
      max_blockNumber = msg->length / b2_blockSize;
      if ( b2_blockNumber > max_blockNumber ) {
         outcome = E_FAIL;
         // TODO: HTTP would respond with 416 Range Not Satisfiable
         coap_header.Code = COAP_CODE_RESP_BADREQ;
         i=0;
         b2_moreBlocks = FALSE;
      } else if ( b2_blockNumber == max_blockNumber ) {
         i = msg->length % b2_blockSize;
         b2_moreBlocks = FALSE;
      } else {
         i = b2_blockSize;
         b2_moreBlocks = TRUE;
      }
      // copy i bytes from msg->payload[blockNumber*blockSize] into msg->packet[127-i]
      msg->payload = msg->payload + b2_blockNumber*b2_blockSize;
      msg->length = i;
      // if msg->payload does not point into the second half of msg->packet, copy the data there
      if (msg->payload < &msg->packet[63] || msg->payload >= &msg->packet[127]) {
         // like memcpy but handles overlapping memory
         memmove(&msg->packet[127-msg->length], msg->payload, msg->length);
         msg->payload = &msg->packet[127-msg->length];
      }

      if (msg->length > 0) {
         packetfunctions_reserveHeaderSize(msg, 1);
         msg->payload[0] = COAP_PAYLOAD_MARKER;
      }

      if ( b2_idx != MAX_COAP_OPTIONS || response_too_big ) {
         // add block option to response
         b2_blockNumber = b2_blockNumber << 4;
         b2_blockNumber |= (b2_moreBlocks?1:0) << 3;
         for (i=4; i<12; i++) {
            if (b2_blockSize & (1<<i)) {
               b2_blockNumber |= i-4;
               break;
            }
         }
         if (b2_blockNumber == 0) {
            // empty option
         } else if (b2_blockNumber <= 0xff) {
            packetfunctions_reserveHeaderSize(msg, 1);
            msg->payload[0] = b2_blockNumber;
         } else if (b2_blockNumber <= 0xffff) {
            packetfunctions_reserveHeaderSize(msg, 2);
            msg->payload[0] = (b2_blockNumber & 0xff00) >> 8;
            msg->payload[1] = b2_blockNumber & 0xff;
         } else {
            packetfunctions_reserveHeaderSize(msg, 3);
            msg->payload[0] = (b2_blockNumber & 0xff0000) >> 16;
            msg->payload[1] = (b2_blockNumber & 0xff00) >> 8;
            msg->payload[2] = b2_blockNumber & 0xff;
         }
         packetfunctions_reserveHeaderSize(msg, 2);
         msg->payload[0] = (13<<4) | (b2_blockNumber == 0 ? 0 : (b2_blockNumber <= 0xff ? 1 : (b2_blockNumber <= 0xffff ? 2 : 3))); // option delta, option length
         msg->payload[1] = COAP_OPTION_NUM_BLOCK2 - 13;

      }
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
      if (
            coap_header.Code >= COAP_CODE_REQ_GET     &&
            coap_header.Code <= COAP_CODE_REQ_DELETE
         ) {
         // only set the response code if it hasn't been set
         coap_header.Code                 = COAP_CODE_RESP_METHODNOTALLOWED;
      }
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

void opencoap_dropBlockTransfer(opencoap_block_transfer_t* bt_ptr) {
   bt_ptr->uriPathLen = 0;
   bt_ptr->data = NULL;
   bt_ptr->dataLen = 0;
   bt_ptr->ongoing = FALSE;
}

opencoap_block_transfer_t* opencoap_lookupBlockTransfer(OpenQueueEntry_t* msg, coap_option_iht* opt, uint8_t p0_idx, uint8_t p1_idx) {
   uint8_t i;
   opencoap_block_transfer_t* bt_ptr = NULL;

   for (i=0; i<COAP_BLOCK_TRANSFERS; i++) {
      bt_ptr = &opencoap_block_transfers[i];
      if (
            bt_ptr->ongoing == TRUE                            &&
            packetfunctions_sameAddress(&(bt_ptr->clientAddr),
                                        &(msg->l3_sourceAdd))
         ) {
         // same address
         if (
               p0_idx != MAX_COAP_OPTIONS &&
               bt_ptr->uriPathLen == 1+opt[p0_idx].length &&
               memcmp(opt[p0_idx].pValue, &(bt_ptr->uriPath[1]), opt[p0_idx].length) == 0
            ) {
            // same uri (/foo)
            return &(opencoap_block_transfers[i]);
         } else if (
               p0_idx != MAX_COAP_OPTIONS &&
               p1_idx != MAX_COAP_OPTIONS &&
               bt_ptr->uriPathLen == 1+opt[p0_idx].length+1+opt[p1_idx].length &&
               memcmp(opt[p0_idx].pValue, &(bt_ptr->uriPath[1]), opt[p0_idx].length) == 0 &&
               memcmp(opt[p1_idx].pValue, &(bt_ptr->uriPath[1+opt[p0_idx].length+1]), opt[p0_idx].length) == 0
            )
            // same uri (/foo/bar)
            return &(opencoap_block_transfers[i]);
      } 
   }
   return NULL;
}

// create an entry in the block transfer array from an incoming msg
opencoap_block_transfer_t* opencoap_createBlockTransfer(OpenQueueEntry_t* msg, coap_option_iht* coap_options, uint8_t p0_idx, uint8_t p1_idx) {
   uint8_t i;
   uint8_t p0_len = 0;
   uint8_t p1_len = 0;
   opencoap_block_transfer_t* bt_ptr;

   for (i=0; i<COAP_BLOCK_TRANSFERS; i++) {
      bt_ptr = &(opencoap_block_transfers[i]);

      if (bt_ptr->ongoing) {
         continue;
      }
      // copy client address
      bt_ptr->clientAddr.type = msg->l3_sourceAdd.type;
      memcpy(&bt_ptr->clientAddr.addr_128b[0], &msg->l3_sourceAdd.addr_128b[0], LENGTH_ADDR128b);
      if (p0_idx != MAX_COAP_OPTIONS) {
         p0_len = coap_options[p0_idx].length;
         bt_ptr->uriPath[0] = '/';
         // copy first uri path segment
         if (1+p0_len >= 32) {
            return NULL;
         }
         memcpy(&(bt_ptr->uriPath[1]), coap_options[p0_idx].pValue, p0_len);
         if (p1_idx != MAX_COAP_OPTIONS) {
            p1_len = coap_options[p1_idx].length;
            bt_ptr->uriPath[1+p0_len] = '/';
            // copy second uri path segment if it exists
            if (1+p0_len+1+p1_len > 32) {
               return NULL;
            }
            memcpy(&(bt_ptr->uriPath[1+p0_len+1]), coap_options[p1_idx].pValue, p1_len);
         }
      }
      bt_ptr->uriPathLen = 1 + p0_len + (p1_len > 0 ? 1 : 0) + p1_len;

      bt_ptr->data = msg->payload;
      bt_ptr->dataLen = msg->length;
      bt_ptr->ongoing = TRUE;

      return bt_ptr;
   }
   return NULL;
}

// vim: ts=3 sw=3
