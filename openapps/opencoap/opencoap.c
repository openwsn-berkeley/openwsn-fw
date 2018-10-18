#include "opendefs.h"
#include "opencoap.h"
#include "openoscoap.h"
#include "openqueue.h"
#include "openserial.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "opentimers.h"
#include "scheduler.h"
#include "cryptoengine.h"
#include "icmpv6rpl.h"

//=========================== defines =========================================

//=========================== variables =======================================

opencoap_vars_t opencoap_vars;

//=========================== prototype =======================================
void opencoap_header_encode(OpenQueueEntry_t *msg,
        uint8_t version,
        coap_type_t type,
        uint8_t TKL,
        coap_code_t code,
        uint16_t messageID,
        uint8_t *token);

void opencoap_handle_proxy_scheme(OpenQueueEntry_t *msg,
        coap_header_iht* header,
        coap_option_iht* incomingOptions,
        uint8_t incomingOptionsLen);

void opencoap_handle_stateless_proxy(OpenQueueEntry_t *msg,
        coap_header_iht* header,
        coap_option_iht* incomingOptions,
        uint8_t incomingOptionsLen);

void opencoap_add_stateless_proxy_option(coap_option_iht* option,
        uint8_t* address,
        uint8_t addressLen,
        uint16_t portNumber);

void opencoap_forward_message(OpenQueueEntry_t *msg,
        coap_header_iht* header,
        coap_option_iht* outgoinOptions,
        uint8_t outgoingOptionsLen,
        open_addr_t* destIP,
        uint16_t destPortNumber);

//=========================== public ==========================================

//===== from stack

/**
\brief Initialize this module.
*/
void opencoap_init(void) {
   uint16_t rand;
   uint8_t pos;

   pos = 0;

   // initialize the resource linked list
   opencoap_vars.resources     = NULL;

   // initialize the messageID
   opencoap_vars.messageID     = openrandom_get16b();

   // stateless proxy vars

   //generate a key at random
   while (pos<16) {
       rand = openrandom_get16b();
       memcpy(&opencoap_vars.statelessProxy.key[pos],&rand,2);
       pos+=2;
   }
   // init sequence number to zero
   opencoap_vars.statelessProxy.sequenceNumber = 0;

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
   uint8_t                   index;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
   owerror_t                 outcome = 0;
   coap_type_t               response_type;
   // local variables passed to the handlers (with msg)
   coap_header_iht           coap_header;
   coap_option_iht           coap_incomingOptions[MAX_COAP_OPTIONS];
   coap_option_iht           coap_outgoingOptions[MAX_COAP_OPTIONS];
   uint8_t                   coap_incomingOptionsLen;
   uint8_t                   coap_outgoingOptionsLen;
   uint8_t                   option_count;
   uint8_t                   option_index;
   owerror_t                 decStatus;
   coap_option_iht*          objectSecurity;
   coap_option_iht*          proxyScheme;
   coap_option_iht*          statelessProxy;
   uint16_t                  rcvdSequenceNumber;
   uint8_t*                  rcvdKid;
   uint8_t                   rcvdKidLen;
   oscoap_security_context_t* blindContext;
   coap_code_t               securityReturnCode;
   coap_option_class_t       class;

   // init options len
   coap_incomingOptionsLen = MAX_COAP_OPTIONS;
   coap_outgoingOptionsLen = 0;
   class = COAP_OPTION_CLASS_ALL;

   // init returnCode
   securityReturnCode = COAP_CODE_EMPTY;

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

   // remove the CoAP header
   packetfunctions_tossHeader(msg,index);

   // parse options and toss header
   index = opencoap_options_parse(&msg->payload[0], msg->length, coap_incomingOptions, &coap_incomingOptionsLen);

   // toss options
   packetfunctions_tossHeader(msg,index);

   // process handled options
   //== Stateless Proxy option
   option_count = opencoap_find_option(coap_incomingOptions, coap_incomingOptionsLen, COAP_OPTION_NUM_STATELESSPROXY, &option_index);
   if(option_count >= 1) {
     statelessProxy = &coap_incomingOptions[option_index];
   } else {
     statelessProxy = NULL;
   }
   if (statelessProxy) {
       opencoap_handle_stateless_proxy(msg, &coap_header, coap_incomingOptions, coap_incomingOptionsLen);
       openqueue_freePacketBuffer(msg);
       return;
   }

   //== Proxy Scheme option
   option_count = opencoap_find_option(coap_incomingOptions, coap_incomingOptionsLen, COAP_OPTION_NUM_PROXYSCHEME, &option_index);
   if(option_count >= 1) {
     proxyScheme = &coap_incomingOptions[option_index];
   } else {
     proxyScheme = NULL;
   }
   if (proxyScheme) {
        opencoap_handle_proxy_scheme(msg, &coap_header, coap_incomingOptions, coap_incomingOptionsLen);
        openqueue_freePacketBuffer(msg);
        return;
   }


   //== Object Security Option
   option_count = opencoap_find_option(coap_incomingOptions, coap_incomingOptionsLen, COAP_OPTION_NUM_OBJECTSECURITY, &option_index);
   if(option_count >= 1) {
     objectSecurity = &coap_incomingOptions[option_index];
   } else {
     objectSecurity = NULL;
   }
   if (objectSecurity) {
       if ((objectSecurity->length == 0 && msg->length == 0) ||
               (objectSecurity->length != 0 && msg->length != 0)) {
            // malformated object security message
            return;
       }

       if (objectSecurity->length == 0) {
           index = openoscoap_parse_compressed_COSE(&msg->payload[0],
                   msg->length,
                   &rcvdSequenceNumber,
                   &rcvdKid,
                   &rcvdKidLen);
           if (index == 0) {
               return;
           }
           packetfunctions_tossHeader(msg, index);
       }
       else {
           index = openoscoap_parse_compressed_COSE(objectSecurity->pValue,
                   objectSecurity->length,
                   &rcvdSequenceNumber,
                   &rcvdKid,
                   &rcvdKidLen);

           if (index == 0) {
               return;
           }
           objectSecurity->length -= index;
           objectSecurity->pValue += index;
       }
   }

   //=== step 2. find the resource to handle the packet

   // find the resource this applies to
   found = FALSE;

   if (
         coap_header.Code>=COAP_CODE_REQ_GET &&
         coap_header.Code<=COAP_CODE_REQ_DELETE
      ) {
      // this is a request: target resource is indicated as COAP_OPTION_LOCATIONPATH option(s)

      // first, we need to decrypt the request and to do so find the right security context
      if (objectSecurity) {
            temp_desc = opencoap_vars.resources;
            blindContext = NULL;
            // loop through all resources and compare recipient context
            do {
                if (temp_desc->securityContext != NULL &&
                    temp_desc->securityContext->recipientIDLen == rcvdKidLen &&
                    memcmp(rcvdKid, temp_desc->securityContext->recipientID, rcvdKidLen) == 0) {

                    blindContext = temp_desc->securityContext;
                    break;
                }
                temp_desc = temp_desc->next;
            }
            while (temp_desc->next!=NULL);

            if (blindContext) {
                    coap_incomingOptionsLen = MAX_COAP_OPTIONS;
                    decStatus = openoscoap_unprotect_message(blindContext,
                            coap_header.Ver,
                            coap_header.Code,
                            coap_incomingOptions,
                            &coap_incomingOptionsLen,
                            msg,
                            rcvdSequenceNumber
                    );

                    if (decStatus != E_SUCCESS) {
                        securityReturnCode = COAP_CODE_RESP_BADREQ;
                    }

            }
            else {
                securityReturnCode = COAP_CODE_RESP_UNAUTHORIZED;
            }
      }


      // find the resource which matches

      // start with the first resource in the linked list
      temp_desc = opencoap_vars.resources;

      // iterate until matching resource found, or no match
      while (found==FALSE && securityReturnCode==COAP_CODE_EMPTY) {

        option_count = opencoap_find_option(coap_incomingOptions, coap_incomingOptionsLen, COAP_OPTION_NUM_URIPATH, &option_index);
        if (
             option_count == 2                                       &&
             temp_desc->path0len>0                                   &&
             temp_desc->path0val!=NULL                               &&
             temp_desc->path1len>0                                   &&
             temp_desc->path1val!=NULL
          ) {
          // resource has a path of form path0/path1

          if (
                coap_incomingOptions[option_index].length==temp_desc->path0len                                  &&
                memcmp(coap_incomingOptions[option_index].pValue,temp_desc->path0val,temp_desc->path0len)==0    &&
                coap_incomingOptions[option_index+1].length==temp_desc->path1len                                &&
                memcmp(coap_incomingOptions[option_index+1].pValue,temp_desc->path1val,temp_desc->path1len)==0
             ) {
             if (temp_desc->securityContext != NULL &&
                 blindContext != temp_desc->securityContext) {
                 securityReturnCode = COAP_CODE_RESP_UNAUTHORIZED;
             }
              found = TRUE;
          };

        } else if (
             option_count == 1                                &&
             temp_desc->path0len>0                            &&
             temp_desc->path0val!=NULL
          ) {
          // resource has a path of form path0

          if (
                coap_incomingOptions[option_index].length==temp_desc->path0len                               &&
                memcmp(coap_incomingOptions[option_index].pValue,temp_desc->path0val,temp_desc->path0len)==0
             ) {
             if (temp_desc->securityContext != NULL &&
                 blindContext != temp_desc->securityContext) {
                 securityReturnCode = COAP_CODE_RESP_UNAUTHORIZED;
             }
             found = TRUE;
          };
        } else {
          // option_count == 0  ||
          // option_count >= 2
          // resource has not a valid path or path is too long
          found = FALSE;
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

            if (coap_header.T==COAP_TYPE_ACK        ||
                    coap_header.T==COAP_TYPE_RES    ||
                    coap_header.TKL == 0) {
                if (coap_header.messageID==temp_desc->last_request.messageID) {
                    found=TRUE;
                }
            } else {
                found=TRUE;
            }

            // resource found
            // verify if it needs to be decrypted
            // errors are passed to the application without decryption as they do not contain
            // object security option
            if (found==TRUE && temp_desc->callbackRx!=NULL) {
                if (temp_desc->securityContext != NULL && coap_header.Code < COAP_CODE_RESP_BADREQ) {
                    coap_incomingOptionsLen = MAX_COAP_OPTIONS;
                    decStatus = openoscoap_unprotect_message(temp_desc->securityContext,
                            coap_header.Ver,
                            coap_header.Code,
                            coap_incomingOptions,
                            &coap_incomingOptionsLen,
                            msg,
                            temp_desc->last_request.oscoapSeqNum
                    );

                    if (decStatus != E_SUCCESS) {
                        return;
                    }
                }
               temp_desc->callbackRx(msg,&coap_header,&coap_incomingOptions[0], NULL, NULL);
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

   if (found==TRUE && securityReturnCode==COAP_CODE_EMPTY) {

      // call the resource's callback
      outcome = temp_desc->callbackRx(msg,&coap_header,&coap_incomingOptions[0], coap_outgoingOptions, &coap_outgoingOptionsLen);
      if (outcome == E_FAIL) {
            securityReturnCode = COAP_CODE_RESP_METHODNOTALLOWED;
      }
    if (temp_desc->securityContext != NULL) {
        coap_outgoingOptions[coap_outgoingOptionsLen++].type = COAP_OPTION_NUM_OBJECTSECURITY;
        if (coap_outgoingOptionsLen > MAX_COAP_OPTIONS) {
            securityReturnCode = COAP_CODE_RESP_SERVERERROR; // no space for object security option
        }
        // protect the message in the openqueue buffer
        openoscoap_protect_message(
                  temp_desc->securityContext,
                  COAP_VERSION,
                  coap_header.Code,
                  coap_outgoingOptions,
                  coap_outgoingOptionsLen,
                  msg,
                  rcvdSequenceNumber);
        class = COAP_OPTION_CLASS_U;
    } else {
        class = COAP_OPTION_CLASS_ALL;
    }

   } else {
      // reset packet payload (DO NOT DELETE, we will reuse same buffer for response)
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      if (securityReturnCode) {
         coap_header.Code              = securityReturnCode;
      }
      else {
         coap_header.Code              = COAP_CODE_RESP_NOTFOUND;
      }
   }

   if (outcome==E_FAIL) {
      // reset packet payload (DO NOT DELETE, we will reuse same buffer for response)
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      coap_header.Code                 = securityReturnCode;
   }

   if (coap_header.T == COAP_TYPE_CON) {
       response_type = COAP_TYPE_ACK;
   } else {
       response_type = COAP_TYPE_NON;
   }

   //=== step 4. add the payload marker and encode options

   if (msg->length > 0 ) { // contains payload, add payload marker
      packetfunctions_reserveHeaderSize(msg,1);
      msg->payload[0] = COAP_PAYLOAD_MARKER;
   }

   // once header is reserved, encode the options to the openqueue payload buffer
   opencoap_options_encode(msg, coap_outgoingOptions, coap_outgoingOptionsLen, class);

   //=== step 5. send that packet back

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
   opencoap_header_encode(msg,
           COAP_VERSION,
           response_type,
           coap_header.TKL,
           coap_header.Code,
           coap_header.messageID,
           &coap_header.token[0]);

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
\param[in] options An array of sorted CoAP options.
\param[in] optionsLen The length of the options array.
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
      coap_option_iht*       options,
      uint8_t                optionsLen,
      coap_resource_desc_t*  descSender
   ) {
   uint16_t token;
   uint8_t tokenPos=0;
   coap_header_iht* request;
   owerror_t ret;
   coap_option_class_t class;

   class = COAP_OPTION_CLASS_ALL;

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

   if (descSender->securityContext != NULL) { // security activated for the resource
      // get new sequence number and save it
      request->oscoapSeqNum = openoscoap_get_sequence_number(descSender->securityContext);
      // protect the message in the openqueue buffer
      ret = openoscoap_protect_message(
              descSender->securityContext,
              COAP_VERSION,
              code,
              options,
              optionsLen,
              msg,
              request->oscoapSeqNum);

      if (ret != E_SUCCESS) {
         return E_FAIL;
      }
      class = COAP_OPTION_CLASS_U;
   }

    // add payload marker
    if (msg->length) {
        packetfunctions_reserveHeaderSize(msg, 1);
        msg->payload[0] = COAP_PAYLOAD_MARKER;
    }

   // once header is reserved, encode the options to the openqueue payload buffer
   opencoap_options_encode(msg,
           options,
           optionsLen,
           class);

   // pre-pend CoAP header (version,type,TKL,code,messageID,Token)
   opencoap_header_encode(msg, COAP_VERSION, type, request->TKL, code, request->messageID, request->token);

   return openudp_send(msg);
}

/**
\brief Lookup the OSCOAP class for a given option.

This function is called to resolve the OSCOAP class of the passed option.
CLASS_E options get encrypted, CLASS_I options are integrity protected,
and CLASS_U options are unprotected by OSCOAP, if security is activated.

\param[in] type The CoAP option type that needs to be resolved.
*/
coap_option_class_t opencoap_get_option_class(coap_option_t type) {
    switch(type) {
        // class E options
        case COAP_OPTION_NUM_IFMATCH:
        case COAP_OPTION_NUM_ETAG:
        case COAP_OPTION_NUM_IFNONEMATCH:
        case COAP_OPTION_NUM_LOCATIONPATH:
        case COAP_OPTION_NUM_URIPATH:
        case COAP_OPTION_NUM_CONTENTFORMAT:
        case COAP_OPTION_NUM_MAXAGE:
        case COAP_OPTION_NUM_URIQUERY:
        case COAP_OPTION_NUM_ACCEPT:
        case COAP_OPTION_NUM_LOCATIONQUERY:
            return COAP_OPTION_CLASS_E;
        // class I options none supported

        //class U options
        case COAP_OPTION_NUM_URIHOST:
        case COAP_OPTION_NUM_URIPORT:
        case COAP_OPTION_NUM_PROXYURI:
        case COAP_OPTION_NUM_PROXYSCHEME:
        case COAP_OPTION_NUM_OBJECTSECURITY:
            return COAP_OPTION_CLASS_U;
        default:
            return COAP_OPTION_CLASS_U;
    }
}

owerror_t opencoap_options_encode(
        OpenQueueEntry_t*       msg,
        coap_option_iht*        options,
        uint8_t                 optionsLen,
        coap_option_class_t     class
        ) {

    uint8_t i;
    uint8_t ii;
    uint32_t delta;
    uint8_t optionDelta;
    uint8_t optionDeltaExt[2];
    uint8_t optionDeltaExtLen;
    uint8_t optionLength;
    uint8_t optionLengthExt[2];
    uint8_t optionLengthExtLen;
    coap_option_t previousOptionNum;

    // encode options in reversed order
    if (options != NULL && optionsLen != 0) {
        for (i = optionsLen ; i-- > 0 ; ) {
            // skip option if inappropriate class
            if (class != opencoap_get_option_class(options[i].type) &&
                class != COAP_OPTION_CLASS_ALL) {
                continue;
            }

            // loop to find the previous option to which delta should be calculated
            previousOptionNum = COAP_OPTION_NONE;
            for (ii = i ; ii-- > 0 ; ) {
                if (class != opencoap_get_option_class(options[ii].type) &&
                    class != COAP_OPTION_CLASS_ALL) {
                    continue;
                }
                else {
                    previousOptionNum = options[ii].type;
                    break;
                }
            }

            if (previousOptionNum > options[i].type) {
                return E_FAIL; // we require the options to be sorted
            }
            delta = options[i].type - previousOptionNum;

            if (delta <= 12) {
                optionDelta = (uint8_t) delta;
                optionDeltaExtLen = 0;
            }
            else if (delta <= 0xff + 13) {
                optionDelta = 13;
                optionDeltaExt[0] = (uint8_t) delta - 13;
                optionDeltaExtLen = 1;
            }
            else if (delta <= 0xffff + 269) {
                optionDelta = 14;
                packetfunctions_htons((uint16_t) delta - 269, optionDeltaExt);
                optionDeltaExtLen = 2;
            }
            else {
                return E_FAIL;
            }

            if (options[i].length <= 12) {
                optionLength = options[i].length;
                optionLengthExtLen = 0;
            }
            else {
                // we do not support fragmentation so option length cannot be larger
                // than 0xff. therefore, we default to the case where optionLength = 13.
                // see RFC7252 Section 3.1 for more details.
                optionLength = 13;
                optionLengthExt[0] = options[i].length - 13;
                optionLengthExtLen = 1;
            }

            // write to packet in reversed order
            packetfunctions_reserveHeaderSize(msg, options[i].length);
            memcpy(&msg->payload[0], options[i].pValue, options[i].length);

            packetfunctions_reserveHeaderSize(msg, optionLengthExtLen);
            memcpy(&msg->payload[0], optionLengthExt, optionLengthExtLen);

            packetfunctions_reserveHeaderSize(msg, optionDeltaExtLen);
            memcpy(&msg->payload[0], optionDeltaExt, optionDeltaExtLen);

            packetfunctions_reserveHeaderSize(msg, 1);
            msg->payload[0] = (optionDelta << 4) | optionLength;
        }
    }
    return E_SUCCESS;
}



uint8_t opencoap_find_option(coap_option_iht* array, uint8_t arrayLen, coap_option_t option, uint8_t* startIndex) {
  uint8_t i;
  uint8_t j;
  bool found;

  //init local variables
  j = 0;
  found = FALSE;

  for (i=0; i< arrayLen; i++){
    if (array[i].type == option) {
      // validate if startIndex is already set
      if (found == FALSE) {
        if (startIndex != NULL) {
           *startIndex = i;
        }
        found = TRUE;
      }
      //increment option counter
      j++;
    }
  }

  // option not found
  if (found == FALSE) {
    if (startIndex != NULL) {
       *startIndex = 0;
    }
  }

  return j;

}

//=========================== private =========================================

uint8_t opencoap_options_parse(
        uint8_t*                buffer,
        uint8_t                 bufferLen,
        coap_option_iht*        options,
        uint8_t*                optionsLen
        ) {

    uint8_t index;
    uint8_t i;
    coap_option_t lastOption;
    coap_option_t optionDelta;
    uint8_t optionLength;
    uint8_t numOptions;

    index = 0;
    numOptions = 0;

    // initialize the coap_incomingOptions
    for (i=0;i<*optionsLen;i++) {
        options[i].type = COAP_OPTION_NONE;
        options[i].length = 0;
        options[i].pValue = NULL;
    }

    lastOption = COAP_OPTION_NONE;
    for (i = 0; i < *optionsLen; i++) {

        // detect when done parsing options
        if (buffer[index]==COAP_PAYLOAD_MARKER) {
            // found the payload marker, done parsing options.
            index++; // skip marker and stop parsing options
            break;
        }
        if (bufferLen<=index) {
             // end of message, no payload
            break;
        }

        optionDelta = ((buffer[index] & 0xf0) >> 4);
        optionLength = (buffer[index] & 0x0f);

        index++;

        if (optionDelta <= 12) {
        }
        else if (optionDelta == 13) {
            optionDelta = buffer[index] + 13;
            index++;
        }
        else if (optionDelta == 14) {
            optionDelta = (coap_option_t) (packetfunctions_ntohs(&buffer[index]) + 269);
            index += 2;
        }
        else {
            break;
        }

        if (optionLength <= 12) {

        }
        else if (optionLength == 13) {
            optionLength = buffer[index] + 13;
            index++;
        }
        else {
            // case 14 not supported
            break;
        }

        if (bufferLen <= index) {
            break;
        }

        // create new option
        options[i].type = lastOption + optionDelta;
        options[i].length = optionLength;
        if (optionLength) {
            options[i].pValue = &(buffer[index]);
        }
        index += optionLength;
        lastOption = options[i].type;
        numOptions++;
    }
    *optionsLen = numOptions;
    return index;
}

void opencoap_handle_proxy_scheme(OpenQueueEntry_t *msg,
        coap_header_iht* header,
        coap_option_iht* incomingOptions,
        uint8_t incomingOptionsLen) {

    uint8_t i;
    coap_option_iht outgoingOptions[MAX_COAP_OPTIONS];
    uint8_t outgoingOptionsLen;
    uint8_t option_count;
    uint8_t option_index;
    coap_option_iht *uriHost;
    coap_option_iht *proxyScheme;
    const uint8_t proxySchemeCoap[] = "coap";
    const uint8_t uriHost6tisch[] = "6tisch.arpa";
    open_addr_t JRCaddress;

    // verify that Proxy Scheme is set to coap
    option_count = opencoap_find_option(incomingOptions, incomingOptionsLen, COAP_OPTION_NUM_PROXYSCHEME, &option_index);
    if(option_count >= 1) {
      proxyScheme = &incomingOptions[option_index];
    } else {
      proxyScheme = NULL;
    }
    if (memcmp(proxySchemeCoap, proxyScheme->pValue, sizeof(proxySchemeCoap)-1) != 0) {
        return;
    }

    // verify that UriHost is set to "6tisch.arpa"
    option_count = opencoap_find_option(incomingOptions, incomingOptionsLen, COAP_OPTION_NUM_URIHOST, &option_index);
    if(option_count >= 1) {
      uriHost = &incomingOptions[option_index];
    } else {
      uriHost = NULL;
    }
    if (uriHost) {
        if (memcmp(uriHost6tisch, uriHost->pValue, sizeof(uriHost6tisch)-1) != 0) {
            return;
        }
    }
    else {
        return;
    }

    outgoingOptionsLen = 0;

    // process options
    for (i = 0; i < incomingOptionsLen; i++) {
        if (incomingOptions[i].type == COAP_OPTION_NUM_PROXYSCHEME ||
                incomingOptions[i].type == COAP_OPTION_NUM_URIHOST) {
            continue;
        }
        outgoingOptions[outgoingOptionsLen].type = incomingOptions[i].type;
        outgoingOptions[outgoingOptionsLen].length = incomingOptions[i].length;
        outgoingOptions[outgoingOptionsLen].pValue = incomingOptions[i].pValue;
        outgoingOptionsLen++;
    }

    opencoap_add_stateless_proxy_option(&outgoingOptions[outgoingOptionsLen++],
        &msg->l3_sourceAdd.addr_128b[8],
        8,
        msg->l4_sourcePortORicmpv6Type);

    // the JRC is co-located with DAG root, get the address from RPL module
    JRCaddress.type = ADDR_128B;
    if (icmpv6rpl_getRPLDODAGid(JRCaddress.addr_128b) == E_SUCCESS) {
        opencoap_forward_message(msg, header, outgoingOptions, outgoingOptionsLen, &JRCaddress, WKP_UDP_COAP);
    }
    return;
}

void opencoap_handle_stateless_proxy(OpenQueueEntry_t *msg,
        coap_header_iht* header,
        coap_option_iht* incomingOptions,
        uint8_t incomingOptionsLen) {
    uint16_t portNumber;
    coap_option_iht* statelessProxy;
    uint8_t i;
    coap_option_iht outgoingOptions[MAX_COAP_OPTIONS];
    uint8_t outgoingOptionsLen;
    uint8_t                   option_count;
    uint8_t                   option_index;
    open_addr_t eui64;
    open_addr_t destIP;
    open_addr_t link_local_prefix;

    option_count = opencoap_find_option(incomingOptions, incomingOptionsLen, COAP_OPTION_NUM_STATELESSPROXY, &option_index);
    if(option_count >= 1)
    {
      statelessProxy = &incomingOptions[option_index];
    }
    else
    {
      statelessProxy = NULL;
    }
    if (statelessProxy == NULL) {
        return;
    }
    // parse the value of Stateless Proxy
    if (statelessProxy->length < 8) {
        return;
    }
    eui64.type = ADDR_64B;
    memcpy(eui64.addr_64b, statelessProxy->pValue, 8);

    // use link-local prefix to forward the response
    memset(&link_local_prefix, 0x00, sizeof(open_addr_t));
    link_local_prefix.type = ADDR_PREFIX;
    link_local_prefix.prefix[0] = 0xfe;
    link_local_prefix.prefix[1] = 0x80;

    packetfunctions_mac64bToIp128b(&link_local_prefix, &eui64, &destIP);

    if (statelessProxy->length == 10) {
        portNumber = packetfunctions_ntohs(&statelessProxy->pValue[8]);
    }
    else if (statelessProxy->length == 8) {
        portNumber = WKP_UDP_COAP;
    }
    else {
        // unsupported
        return;
    }

    outgoingOptionsLen = 0;

    // process options
    for (i = 0; i < incomingOptionsLen; i++) {
        if (incomingOptions[i].type == COAP_OPTION_NUM_STATELESSPROXY) {
            continue;
        }
        outgoingOptions[outgoingOptionsLen].type = incomingOptions[i].type;
        outgoingOptions[outgoingOptionsLen].length = incomingOptions[i].length;
        outgoingOptions[outgoingOptionsLen].pValue = incomingOptions[i].pValue;
        outgoingOptionsLen++;
    }

    opencoap_forward_message(msg, header, outgoingOptions, outgoingOptionsLen, &destIP, portNumber);
}

void opencoap_header_encode(OpenQueueEntry_t *msg,
        uint8_t version,
        coap_type_t type,
        uint8_t TKL,
        coap_code_t code,
        uint16_t messageID,
        uint8_t *token) {
   // pre-pend CoAP header (version,type,TKL,code,messageID,Token)
   packetfunctions_reserveHeaderSize(msg,4+TKL);
   msg->payload[0]                  = (version        << 6) |
                                      (type           << 4) |
                                      (TKL            << 0);
   msg->payload[1]                  = code;
   msg->payload[2]                  = (messageID>>8) & 0xff;
   msg->payload[3]                  = (messageID>>0) & 0xff;

   memcpy(&msg->payload[4],token,TKL);
}

void opencoap_add_stateless_proxy_option(coap_option_iht* option,
        uint8_t* address,
        uint8_t addressLen,
        uint16_t portNumber) {
    uint8_t len;

    // FIXME due to the lack of space in the 802.15.4 frame
    // we do not encrypt and authenticate the Stateless-Proxy state

    len = 0;

    // next bytes are address
    memcpy(&opencoap_vars.statelessProxy.buffer[len], address, addressLen);
    len += addressLen;

    if (portNumber != WKP_UDP_COAP) {
        packetfunctions_htons(portNumber, &opencoap_vars.statelessProxy.buffer[len]);
        len += 2;
    }

    option->type = COAP_OPTION_NUM_STATELESSPROXY;
    option->length = len;
    option->pValue = opencoap_vars.statelessProxy.buffer;
}

void opencoap_forward_message(OpenQueueEntry_t *msg,
        coap_header_iht* header,
        coap_option_iht* outgoingOptions,
        uint8_t outgoingOptionsLen,
        open_addr_t* destIP,
        uint16_t destPortNumber) {

    OpenQueueEntry_t* outgoingPacket;

    outgoingPacket = openqueue_getFreePacketBuffer(COMPONENT_OPENCOAP);
    if (outgoingPacket==NULL) {
        openserial_printError(
                COMPONENT_OPENCOAP,
                ERR_NO_FREE_PACKET_BUFFER,
                (errorparameter_t)0,
                (errorparameter_t)0
        );
      openqueue_freePacketBuffer(outgoingPacket);
      return;
    }

    // take ownership over that packet and set destination IP and port
    outgoingPacket->creator                   = COMPONENT_OPENCOAP;
    outgoingPacket->owner                     = COMPONENT_OPENCOAP;
    outgoingPacket->l4_destination_port       = destPortNumber;
    outgoingPacket->l3_destinationAdd.type    = ADDR_128B;
    memcpy(outgoingPacket->l3_destinationAdd.addr_128b,destIP->addr_128b,16);

    // fill in source port number
    outgoingPacket->l4_sourcePortORicmpv6Type = WKP_UDP_COAP;

    // fill payload
    if (msg->length) {
        packetfunctions_reserveHeaderSize(outgoingPacket,msg->length);
        memcpy(outgoingPacket->payload, msg->payload, msg->length);
        packetfunctions_reserveHeaderSize(outgoingPacket, 1);
        outgoingPacket->payload[0] = COAP_PAYLOAD_MARKER;
    }

    // encode options
    opencoap_options_encode(outgoingPacket, outgoingOptions, outgoingOptionsLen, COAP_OPTION_CLASS_ALL);

    // encode CoAP header
    opencoap_header_encode(outgoingPacket,
            header->Ver,
            header->T,
            header->TKL,
            header->Code,
            header->messageID,
            header->token);

    if ((openudp_send(outgoingPacket))==E_FAIL) {
      openqueue_freePacketBuffer(outgoingPacket);
    }
}
