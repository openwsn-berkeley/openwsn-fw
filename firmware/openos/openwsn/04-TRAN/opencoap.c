#include "openwsn.h"
#include "opencoap.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "openrandom.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "opentimers.h"
#include "scheduler.h"

//=========================== variables =======================================

opencoap_vars_t opencoap_vars;

//=========================== prototype =======================================

void icmpv6coap_timer_cb();

//=========================== public ==========================================

//===== from stack

void opencoap_init() {
   // initialize the resource linked list
   opencoap_vars.resources     = NULL;
   
   // initialize the messageID
   opencoap_vars.messageID     = openrandom_get16b();
   
   // start the timer
   if (idmanager_getIsDAGroot()==FALSE) {
      opencoap_vars.timerId = opentimers_start(1000,
                                               TIMER_PERIODIC,TIME_MS,
                                               icmpv6coap_timer_cb);
   }
}

void opencoap_receive(OpenQueueEntry_t* msg) {
   uint16_t                  temp_l4_destination_port;
   uint8_t                   i;
   uint8_t                   index;
   coap_option_t             last_option;
   coap_resource_desc_t*     temp_desc;
   bool                      found;
   owerror_t                   outcome;
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
   
   //poipoi xv. TKL tells us the length of the token. If we want to support tokens longer
   //than one token needs to be converted to an array and memcopy here for the length of TKL
   coap_header.token         = (msg->payload[index]);
   index+=coap_header.TKL;
   
   
   // reject unsupported header
   if (coap_header.Ver!=COAP_VERSION || coap_header.TKL>8) {
      /*openserial_printError(COMPONENT_OPENCOAP,ERR_WRONG_TRAN_PROTOCOL,
                            (errorparameter_t)0,
                            (errorparameter_t)coap_header.Ver);*/
      openqueue_freePacketBuffer(msg);
      return;
   }
   // initialize the coap_options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      coap_options[i].type = COAP_OPTION_NONE;
   }
   // fill in the coap_options
   last_option = COAP_OPTION_NONE;
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
     if (msg->payload[index]==0xFF){
       //found the payload spacer, options are already parsed.
       index++; //skip it and break.
       break;
     }
      coap_options[i].type        = (coap_option_t)((uint8_t)last_option+(uint8_t)((msg->payload[index] & 0xf0) >> 4));
      last_option                 = coap_options[i].type;
      coap_options[i].length      = (msg->payload[index] & 0x0f);
      index++;
      coap_options[i].pValue      = &(msg->payload[index]);
      index += coap_options[i].length; //includes length as well
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
                coap_options[0].type==COAP_OPTION_NUM_URIPATH &&
                coap_options[1].type==COAP_OPTION_NUM_URIPATH &&
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
                coap_options[0].type==COAP_OPTION_NUM_URIPATH &&
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
      
      // find the resource which matches    
      temp_desc = opencoap_vars.resources;
      while (found==FALSE) {
         if (coap_header.messageID==temp_desc->messageID) {
            found=TRUE;
            if (temp_desc->callbackRx!=NULL) {
               temp_desc->callbackRx(msg,&coap_header,&coap_options[0]);
            }
         }
         // iterate to next resource
         if (found==FALSE) {
            if (temp_desc->next!=NULL) {
               temp_desc = temp_desc->next;
            } else {
               break;
            }
         }
      };
      openqueue_freePacketBuffer(msg);
      // stop here: will will not respond to a response
      return;
   }
   
   //=== step 3. ask the resource to prepare response
   
   if (found==TRUE) {
      outcome = temp_desc->callbackRx(msg,&coap_header,&coap_options[0]);
   } else {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
      coap_header.Code                 = COAP_CODE_RESP_NOTFOUND;
   }
   
   if (outcome==E_FAIL) {
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      // set the CoAP header
      coap_header.TKL                  = 0;
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
   
   //set destination address as the current source.
   msg->l3_destinationAdd.type = ADDR_128B;
   memcpy(&msg->l3_destinationAdd.addr_128b[0],&msg->l3_sourceAdd.addr_128b[0],LENGTH_ADDR128b);
   
   
   // fill in CoAP header
   packetfunctions_reserveHeaderSize(msg,5);
   msg->payload[0]                  = (COAP_VERSION   << 6) |
                                      (COAP_TYPE_ACK  << 4) |
                                      (coap_header.TKL << 0);
   msg->payload[1]                  = coap_header.Code;
   msg->payload[2]                  = coap_header.messageID/256;
   msg->payload[3]                  = coap_header.messageID%256;
   msg->payload[4]                  = coap_header.token; //this will be a memcopy for TKL size
   
   if ((openudp_send(msg))==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

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
      if (temp_resource->componentID==msg->creator &&
          temp_resource->callbackSendDone!=NULL) {
         temp_resource->callbackSendDone(msg,error);
         return;
      }
      temp_resource = temp_resource->next;
   }
   
   /*openserial_printError(COMPONENT_OPENCOAP,ERR_UNEXPECTED_SENDDONE,
                         (errorparameter_t)0,
                         (errorparameter_t)0);*/
   openqueue_freePacketBuffer(msg);
}

void timers_coap_fired() {
   //do something here if necessary
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

/**
\brief Register a new CoAP resource.

Registration consists in adding a new resource at the end of the linked list
of resources.
*/
void opencoap_register(coap_resource_desc_t* desc) {
   coap_resource_desc_t* last_elem;
   
   // since this CoAP resource will be at the end of the list, its next element
   // should point to NULL, indicating the end of the linked list.
   desc->next = NULL;
   
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

owerror_t opencoap_send(OpenQueueEntry_t*     msg,
                      coap_type_t           type,
                      coap_code_t           code,
                      uint8_t               TKL,
                      coap_resource_desc_t* descSender) {
   // change the global messageID
   opencoap_vars.messageID          = openrandom_get16b();
   // take ownership
   msg->owner                       = COMPONENT_OPENCOAP;
   // metadata
   msg->l4_sourcePortORicmpv6Type   = WKP_UDP_COAP;
   // fill in CoAP header
   packetfunctions_reserveHeaderSize(msg,5);
   msg->payload[0]                  = (COAP_VERSION   << 6) |
                                      (type           << 4) |
                                      (TKL            << 0);
   msg->payload[1]                  = code;
   msg->payload[2]                  = (opencoap_vars.messageID>>8) & 0xff;
   msg->payload[3]                  = (opencoap_vars.messageID>>0) & 0xff;
   //poipoi xv token needs to be defined by the app or here
#define TOKEN 123
   msg->payload[4]                  = TOKEN; //this will be a memcopy for TKL size
  
   // record the messageID with this sender
   descSender->messageID            = opencoap_vars.messageID;
   descSender->token                = TOKEN;
   
   return openudp_send(msg);
}

//=========================== private =========================================

void icmpv6coap_timer_cb() {
   scheduler_push_task(timers_coap_fired,TASKPRIO_COAP);
}
