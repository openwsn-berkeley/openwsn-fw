#include "openwsn.h"
#include "opencoap.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "opentimers.h"
#include "packetfunctions.h"
#include "idmanager.h"

//=========================== variables =======================================

// general variable to the CoAP core
typedef struct {
   coap_resource_desc_t* resources;
   bool                  busySending;
   int8_t                delayCounter;
   int16_t               messageID;
} opencoap_vars_t;

opencoap_vars_t opencoap_vars;

//=========================== prototype =======================================

//=========================== public ==========================================

//===== from stack

void opencoap_init() {
   // initialize the resource linked list
   opencoap_vars.resources     = NULL;
   
   // start the timer
   if (idmanager_getIsDAGroot()==FALSE) {
      opentimers_startPeriodic(TIMER_COAP,0xffff);// every 2 seconds
   }
}

void opencoap_receive(OpenQueueEntry_t* msg) {
   uint16_t                  temp_l4_destination_port;
   uint8_t                   i;
   uint8_t                   index;
   coap_option_t             last_option;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
   error_t                   outcome;
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
   coap_header.OC            = (msg->payload[index] & 0x0f);
   index++;
   coap_header.Code          = (coap_code_t)(msg->payload[index]);
   index++;
   coap_header.MessageId[0]  = msg->payload[index];
   index++;
   coap_header.MessageId[1]  = msg->payload[index];
   index++;
   // reject unsupported header
   if (
         coap_header.Ver!=COAP_VERSION ||
         coap_header.OC>MAX_COAP_OPTIONS
      ) {
      openserial_printError(COMPONENT_OPENCOAP,ERR_6LOWPAN_UNSUPPORTED,
                            (errorparameter_t)0,
                            (errorparameter_t)coap_header.Ver);
      openqueue_freePacketBuffer(msg);
      return;
   }
   // initialize the coap_options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      coap_options[i].type = COAP_OPTION_NONE;
   }
   // fill in the coap_options
   last_option = COAP_OPTION_NONE;
   for (i=0;i<coap_header.OC;i++) {
      coap_options[i].type        = (coap_option_t)((uint8_t)last_option+(uint8_t)((msg->payload[index] & 0xf0) >> 4));
      last_option                 = coap_options[i].type;
      coap_options[i].length      = (msg->payload[index] & 0x0f);
      index++;
      coap_options[i].pValue      = &(msg->payload[index]);
      index += coap_options[i].length;
   }
   // remove the CoAP header+options
   packetfunctions_tossHeader(msg,index);
   
   //=== step 2. find the resource to handle the packet
   
   // find the resource this applies to
   found = FALSE;
   if (coap_header.Code>=COAP_CODE_REQ_GET &&
       coap_header.Code<=COAP_CODE_REQ_DELETE) {
      // this is a request: target resource is indicated as COAP_OPTION_LOCATIONPATH option(s)
      
      // find the resource which matches    
      temp_desc = opencoap_vars.resources;
      while (found==FALSE) {
         if    (
                coap_options[0].type==COAP_OPTION_URIPATH &&
                coap_options[1].type==COAP_OPTION_URIPATH &&
                temp_desc->path0len>0                     &&
                temp_desc->path0val!=NULL                 &&
                temp_desc->path1len>0                     &&
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
                coap_options[0].type==COAP_OPTION_URIPATH &&
                temp_desc->path0len>0                     &&
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
         // iterate to next resource
         if (found==FALSE) {
            if (temp_desc->next!=NULL) {
               temp_desc = temp_desc->next;
            } else {
               break;
            }
         }
      };
   } else {
      // this is a response: target resource is indicated by message ID
      temp_desc = temp_desc;
   }
   
   //=== step 3. ask the resource to prepare response
   
   if (found==TRUE) {
      outcome = temp_desc->callbackRx(msg,&coap_header,&coap_options[0]);
   } else {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.OC                   = 0;
      coap_header.Code                 = COAP_CODE_RESP_NOTFOUND;
   }
   
   if (outcome==E_FAIL) {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.OC                   = 0;
      coap_header.Code                 = COAP_CODE_RESP_METHODNOTALLOWED;
   }
   
   //=== step 4. send that packet back
   
   // fill in packet metadata
   if (found==TRUE) {
      msg->creator                  = temp_desc->componentID;
   } else {
      msg->creator                  = COMPONENT_OPENCOAP;
   }
   msg->l4_protocol                 = IANA_UDP;
   temp_l4_destination_port         = msg->l4_destination_port;
   msg->l4_destination_port         = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type   = temp_l4_destination_port;
   
   // fill in CoAP header
   packetfunctions_reserveHeaderSize(msg,4);
   msg->payload[0]                  = (COAP_VERSION   << 6) |
                                      (COAP_TYPE_ACK  << 4) |
                                      (coap_header.OC << 0);
   msg->payload[1]                  = coap_header.Code;
   msg->payload[2]                  = coap_header.MessageId[0];
   msg->payload[3]                  = coap_header.MessageId[1];
   
   if ((openudp_send(msg))==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

void opencoap_sendDone(OpenQueueEntry_t* msg, error_t error) {
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
      if (temp_resource->componentID==msg->creator &&
          temp_resource->callbackSendDone!=NULL) {
         temp_resource->callbackSendDone(msg,error);
         return;
      }
      temp_resource = temp_resource->next;
   }
   
   openserial_printError(COMPONENT_OPENCOAP,ERR_UNEXPECTED_SENDDONE,
                         (errorparameter_t)0,
                         (errorparameter_t)0);
   openqueue_freePacketBuffer(msg);
}

void opentimers_coap_fired() {
   coap_resource_desc_t* temp_resource;
   
   temp_resource = opencoap_vars.resources;
   while (temp_resource!=NULL) {
      if (temp_resource->callbackTimer!=NULL) {
         temp_resource->callbackTimer();
      }
      temp_resource = temp_resource->next;
   }
}

//===== from CoAP resources

void opencoap_writeLinks(OpenQueueEntry_t* msg) {
   coap_resource_desc_t* temp_resource;
   
   temp_resource = opencoap_vars.resources;
   while (temp_resource!=NULL) {
      packetfunctions_reserveHeaderSize(msg,1);
      msg->payload[0] = '>';
      if (temp_resource->path1len>0) {
         packetfunctions_reserveHeaderSize(msg,temp_resource->path1len);
         memcpy(&msg->payload[0],temp_resource->path1val,temp_resource->path1len);
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0] = '/';
      }
      packetfunctions_reserveHeaderSize(msg,temp_resource->path0len);
      memcpy(msg->payload,temp_resource->path0val,temp_resource->path0len);
      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[1] = '/';
      msg->payload[0] = '<';
      if (temp_resource->next!=NULL) {
         packetfunctions_reserveHeaderSize(msg,1);
         msg->payload[0] = ',';
      }
      temp_resource = temp_resource->next;
   }
}

void opencoap_register(coap_resource_desc_t* desc) {
   coap_resource_desc_t* last_elem;
   
   // reset the messageIDused element
   desc->messageIDused = FALSE;
   
   if (opencoap_vars.resources==NULL) {
      opencoap_vars.resources = desc;
      return;
   }
   
   // add to the end of the resource linked list
   last_elem = opencoap_vars.resources;
   while (last_elem->next!=NULL) {
      last_elem = last_elem->next;
   }
   last_elem->next = desc;
}

error_t opencoap_send(OpenQueueEntry_t* msg) {
   // increase the global messageID
   opencoap_vars.messageID++;
   // take ownership
   msg->owner                       = COMPONENT_OPENCOAP;
   // metadata
   msg->l4_sourcePortORicmpv6Type   = WKP_UDP_COAP;
   // fill in CoAP header
   packetfunctions_reserveHeaderSize(msg,4);
   msg->payload[0]                  = (COAP_VERSION   << 6) |
                                      (COAP_TYPE_CON  << 4) |
                                      (3              << 0);
   msg->payload[1]                  = COAP_CODE_REQ_POST;
   msg->payload[2]                  = (opencoap_vars.messageID>>8) & 0xff;
   msg->payload[3]                  = (opencoap_vars.messageID>>0) & 0xff;
   
   return openudp_send(msg);
}

//=========================== private =========================================