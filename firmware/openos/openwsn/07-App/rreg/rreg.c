#include "openwsn.h"
#include "rreg.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"

//=========================== variables =======================================

#define RREGPERIOD       300

typedef struct {
   uint16_t             delay;
   coap_resource_desc_t desc;
} rreg_vars_t;

rreg_vars_t rreg_vars;

const uint8_t rreg_path0[]    = "reg";
const uint8_t rreg_uriquery[] = "h=openwsn";

//=========================== prototypes ======================================

error_t rreg_receive(OpenQueueEntry_t* msg,
                     coap_header_iht*  coap_header,
                     coap_option_iht*  coap_options);
void    rreg_timer();
void    rreg_sendDone(OpenQueueEntry_t* msg,
                      error_t error);
uint8_t hexToAscii(uint8_t hex);

//=========================== public ==========================================

void rreg_init() {
   // prepare the resource descriptor for the /.well-known/core path
   rreg_vars.desc.path0len             = sizeof(rreg_path0)-1;
   rreg_vars.desc.path0val             = (uint8_t*)(&rreg_path0);
   rreg_vars.desc.path1len             = 0;
   rreg_vars.desc.path1val             = NULL;
   rreg_vars.desc.componentID          = COMPONENT_RREG;
   rreg_vars.desc.callbackRx           = &rreg_receive;
   rreg_vars.desc.callbackTimer        = rreg_timer;
   rreg_vars.desc.callbackSendDone     = &rreg_sendDone;
   
   rreg_vars.delay              = 0;
   
   opencoap_register(&rreg_vars.desc);
}

//=========================== private =========================================

error_t rreg_receive(OpenQueueEntry_t* msg,
                   coap_header_iht* coap_header,
                   coap_option_iht* coap_options) {
                      
   error_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      // set delay to RREGPERIOD to gets triggered next time rreg_timer is called
      rreg_vars.delay                  = RREGPERIOD;
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_VALID;
      
      outcome = E_SUCCESS;
   } else {
      outcome = E_FAIL;
   }
   
   return outcome;
}

void rreg_timer() {
   OpenQueueEntry_t* pkt;
   uint8_t           temp8b;
   
   rreg_vars.delay += 2;
   
   if (rreg_vars.delay>=RREGPERIOD) {
      // reset the timer
      rreg_vars.delay = 0;
      // create a CoAP RD packet
      pkt = openqueue_getFreePacketBuffer();
      if (pkt==NULL) {
         openserial_printError(COMPONENT_RREG,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         openqueue_freePacketBuffer(pkt);
         return;
      }
      // take ownership over that packet
      pkt->creator    = COMPONENT_RREG;
      pkt->owner      = COMPONENT_RREG;
      // CoAP payload
      opencoap_writeLinks(pkt);
      // URI-query
      packetfunctions_reserveHeaderSize(pkt,sizeof(rreg_uriquery)-1+2);
      memcpy(&pkt->payload[0],&rreg_uriquery,sizeof(rreg_uriquery)-1);
      temp8b = idmanager_getMyID(ADDR_16B)->addr_16b[1];
      pkt->payload[sizeof(rreg_uriquery)-1] = hexToAscii((temp8b>>4) & 0x0f);
      pkt->payload[sizeof(rreg_uriquery)-0] = hexToAscii((temp8b>>0) & 0x0f);
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0] = (COAP_OPTION_URIQUERY-COAP_OPTION_URIPATH) << 4 |
                        sizeof(rreg_uriquery)-1+2;
      // URI-path
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0] = 'r';
      pkt->payload[1] = 'd';
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0] = (COAP_OPTION_URIPATH-COAP_OPTION_CONTENTTYPE) << 4 |
                        2;
      // add content-type option
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
                                         1;
      pkt->payload[1]                  = COAP_MEDTYPE_APPLINKFORMAT;
      // metadata
      pkt->l4_destination_port         = WKP_UDP_COAP;
      pkt->l3_destinationORsource.type = ADDR_128B;
      /*
      // send RD registration to interop.ipso-alliance
      pkt->l3_destinationORsource.addr_128b[ 0] = 0x26;
      pkt->l3_destinationORsource.addr_128b[ 1] = 0x07;
      pkt->l3_destinationORsource.addr_128b[ 2] = 0xf7;
      pkt->l3_destinationORsource.addr_128b[ 3] = 0x40;
      pkt->l3_destinationORsource.addr_128b[ 4] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 5] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 6] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 7] = 0x3f;
      pkt->l3_destinationORsource.addr_128b[ 8] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 9] = 0x00;
      pkt->l3_destinationORsource.addr_128b[10] = 0x00;
      pkt->l3_destinationORsource.addr_128b[11] = 0x00;
      pkt->l3_destinationORsource.addr_128b[12] = 0x00;
      pkt->l3_destinationORsource.addr_128b[13] = 0x00;
      pkt->l3_destinationORsource.addr_128b[14] = 0x0e;
      pkt->l3_destinationORsource.addr_128b[15] = 0x29;
      */
      /*
      // send RD registration to backup server
      pkt->l3_destinationORsource.addr_128b[ 0] = 0x2a;
      pkt->l3_destinationORsource.addr_128b[ 1] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 2] = 0xdd;
      pkt->l3_destinationORsource.addr_128b[ 3] = 0x80;
      pkt->l3_destinationORsource.addr_128b[ 4] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 5] = 0x3c;
      pkt->l3_destinationORsource.addr_128b[ 6] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 7] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 8] = 0x00;
      pkt->l3_destinationORsource.addr_128b[ 9] = 0x00;
      pkt->l3_destinationORsource.addr_128b[10] = 0x00;
      pkt->l3_destinationORsource.addr_128b[11] = 0x00;
      pkt->l3_destinationORsource.addr_128b[12] = 0x00;
      pkt->l3_destinationORsource.addr_128b[13] = 0x00;
      pkt->l3_destinationORsource.addr_128b[14] = 0x0f;
      pkt->l3_destinationORsource.addr_128b[15] = 0x7b;
      */
      // send RD registration to local address (for debug)
      pkt->l3_destinationORsource.addr_128b[ 0] = 0x20;
      pkt->l3_destinationORsource.addr_128b[ 1] = 0x01;
      pkt->l3_destinationORsource.addr_128b[ 2] = 0x04;
      pkt->l3_destinationORsource.addr_128b[ 3] = 0x70;
      pkt->l3_destinationORsource.addr_128b[ 4] = 0x1f;
      pkt->l3_destinationORsource.addr_128b[ 5] = 0x05;
      pkt->l3_destinationORsource.addr_128b[ 6] = 0x19;
      pkt->l3_destinationORsource.addr_128b[ 7] = 0x37;
      pkt->l3_destinationORsource.addr_128b[ 8] = 0xa5;
      pkt->l3_destinationORsource.addr_128b[ 9] = 0xfe;
      pkt->l3_destinationORsource.addr_128b[10] = 0xc5;
      pkt->l3_destinationORsource.addr_128b[11] = 0x5f;
      pkt->l3_destinationORsource.addr_128b[12] = 0xf0;
      pkt->l3_destinationORsource.addr_128b[13] = 0x5e;
      pkt->l3_destinationORsource.addr_128b[14] = 0xe2;
      pkt->l3_destinationORsource.addr_128b[15] = 0x99;
      // send
      if (opencoap_send(pkt)==E_FAIL) {
         openqueue_freePacketBuffer(pkt);
      }
   }
   return;
}

void rreg_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}

inline uint8_t hexToAscii(uint8_t hex) {
   if (hex<=0 && hex>=9) {
      return '0'+(hex-0x00);
   } else {
      return 'A'+(hex-0x0a);
   }
}