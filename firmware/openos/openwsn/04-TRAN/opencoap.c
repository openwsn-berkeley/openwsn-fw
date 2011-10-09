#include "openwsn.h"
#include "opencoap.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

// general to the CoAP core
coap_resource_desc_t* resources;

// specific the /.well-known/core path handler

const uint8_t rwellknown_path0[]        = ".well-known";
const uint8_t rwellknown_path1[]        = "core";
const uint8_t rwellknown_resp_payload[] = "</poipoi>";

coap_resource_desc_t rwellknown_desc;

//=========================== prototype =======================================

void rwellknown_receive();

//=========================== public ==========================================

void opencoap_init() {
   // initialize the resource linked list
   resources                 = NULL;
   
   // prepare the resource descriptor for the /.well-known/core path
   rwellknown_desc.path0len   = sizeof(rwellknown_path0)-1;
   rwellknown_desc.path0val   = (uint8_t*)(&rwellknown_path0);
   rwellknown_desc.path1len   = sizeof(rwellknown_path1)-1;
   rwellknown_desc.path1val   = (uint8_t*)(&rwellknown_path1);
   rwellknown_desc.callbackRx = &rwellknown_receive;
   
   opencoap_register(&rwellknown_desc);
}

void opencoap_register(coap_resource_desc_t* desc) {
   coap_resource_desc_t* last_elem;
   
   // reset the messageIDused element
   desc->messageIDused = FALSE;
   
   if (resources==NULL) {
      resources = desc;
   }
   
   // add to the end of the resource linked list
   last_elem = resources;
   while (last_elem!=NULL) {
      last_elem = last_elem->next;
   }
   last_elem = desc;
}

void opencoap_receive(OpenQueueEntry_t* msg) {
   uint16_t                  temp_l4_destination_port;
   uint8_t                   i;
   uint8_t                   index;
   coap_option_t             last_option;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
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
   
   //=== step 2. find the resource to handle the packet
   
   // find the resource this applies to
   found = FALSE;
   if (coap_header.Code>=COAP_CODE_REQ_GET &&
       coap_header.Code<=COAP_CODE_REQ_DELETE) {
      // this is a request: target resource is indicated as COAP_OPTION_LOCATIONPATH option(s)
      
      // find the resource which matches    
      temp_desc = resources;
      while (found==FALSE && temp_desc!=NULL) {
         if (
            coap_options[0].length==temp_desc->path0len                               &&
            memcmp(coap_options[0].pValue,temp_desc->path0val,temp_desc->path0len)==0 &&
            coap_options[1].length==temp_desc->path1len                               &&
            memcmp(coap_options[1].pValue,temp_desc->path1val,temp_desc->path1len)==0
            ) {
            found = TRUE;
         };
         temp_desc = temp_desc->next;
      };
   } else {
      // this is a response: target resource is indicated by message ID
   }
   
   //=== step 3. ask the resource to prepare response
   
   if (found==TRUE) {
      rwellknown_desc.callbackRx(msg,&coap_header,&coap_options[0]);
   } else {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.OC                   = 0;
      coap_header.Code                 = COAP_CODE_RESP_NOTFOUND;
   }
   
   //=== step 4. send that packet back
   
   // fill in packet metadata
   msg->creator                     = COMPONENT_OPENCOAP;
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
   msg->owner = COMPONENT_OPENCOAP;
   if (msg->creator!=COMPONENT_OPENCOAP) {
      openserial_printError(COMPONENT_OPENCOAP,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

//=========================== private =========================================

void rwellknown_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header, coap_option_iht* coap_options) {
   if (coap_header->Code==COAP_CODE_REQ_GET) {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // add CoAP payload
      packetfunctions_reserveHeaderSize(msg,sizeof(rwellknown_resp_payload)-1);
      memcpy(msg->payload,rwellknown_resp_payload,sizeof(rwellknown_resp_payload)-1);
         
      // add return option
      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
                                         1;
      msg->payload[1]                  = COAP_MEDTYPE_APPLINKFORMAT;
      
      // set the CoAP header
      coap_header->OC                  = 1;
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
   }
}