#include "openwsn.h"
#include "opencoap.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

const uint8_t coapwellknown_req_option0[]  = ".well-known";
const uint8_t coapwellknown_req_option1[]  = "core";

const uint8_t coapwellknown_resp_payload[] = "</led>";

//=========================== public ==========================================

void opencoap_init() {
}

void opencoap_receive(OpenQueueEntry_t* msg) {
   coap_header_iht  coap_header;
   coap_option_iht  options[MAX_COAP_OPTIONS];
   uint16_t         temp_l4_destination_port;
   uint8_t          i;
   uint8_t          index;
   
   index = 0;
   // initialize the options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      options[i].type = COAP_OPTION_NONE;
   }
   // take ownership over the packet
   msg->owner   = COMPONENT_OPENCOAP;
   // parse the CoAP header and remove from packet
   coap_header.Ver          = (msg->payload[index] & 0xc0) >> 6;
   coap_header.T            = (msg->payload[index] & 0x30) >> 4;
   coap_header.OC           = (msg->payload[index] & 0x0f);
   index++;
   coap_header.Code         = msg->payload[index];
   index++;
   coap_header.MessageId[0] = msg->payload[index];
   index++;
   coap_header.MessageId[1] = msg->payload[index];
   index++;
   // filter unsupported
   if (
         coap_header.Ver!=1 ||
         coap_header.OC>MAX_COAP_OPTIONS
      ) {
      openserial_printError(COMPONENT_OPENCOAP,ERR_6LOWPAN_UNSUPPORTED,
                            (errorparameter_t)0,
                            (errorparameter_t)coap_header.Ver);
      openqueue_freePacketBuffer(msg);
      return;
   }
   // fill in the options
   for (i=0;i<MAX_COAP_OPTIONS;i++) {
      options[i].type   = (msg->payload[index] & 0xf0) >> 4;
      options[i].length = (msg->payload[index] & 0x0f);
      index++;
      options[i].pValue = &(msg->payload[index]);
      index += options[i].length;
   }
   // is this ia well-known request?
   if (coap_header.OC==2 &&
       memcmp(options[0].pValue,&coapwellknown_req_option0,sizeof(coapwellknown_req_option0)-1)==0 &&
       memcmp(options[1].pValue,&coapwellknown_req_option1,sizeof(coapwellknown_req_option1)-1)==0) {
      //reply with the same OpenQueueEntry_t
          
      // metadata
      msg->creator                     = COMPONENT_OPENCOAP;
      msg->l4_protocol                 = IANA_UDP;
      temp_l4_destination_port         = msg->l4_destination_port;
      msg->l4_destination_port         = msg->l4_sourcePortORicmpv6Type;
      msg->l4_sourcePortORicmpv6Type   = temp_l4_destination_port;
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // payload
      packetfunctions_reserveHeaderSize(msg,sizeof(coapwellknown_resp_payload)-1);
      memcpy(msg->payload,coapwellknown_resp_payload,sizeof(coapwellknown_resp_payload)-1);
      
      // content-type option
      packetfunctions_reserveHeaderSize(msg,2);
      msg->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
                                         1;
      msg->payload[1]                  = COAP_MEDTYPE_APPLINKFORMAT;
      
      // header
      packetfunctions_reserveHeaderSize(msg,4);
      msg->payload[0]                  = (1 << 6)             |
                                         (COAP_TYPE_ACK << 4) |
                                         1;
      msg->payload[1]                  = COAP_CODE_RESP_CONTENT;
      msg->payload[2]                  = coap_header.MessageId[0];
      msg->payload[3]                  = coap_header.MessageId[1];
      
      if ((openudp_send(msg))==E_FAIL) {
         openqueue_freePacketBuffer(msg);
      }
   } else {
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