#include "openwsn.h"
#include "rtemp.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"

//=========================== variables =======================================

#define RTEMPPERIOD       10

typedef struct {
   uint16_t             delay;
   coap_resource_desc_t desc;
} rtemp_vars_t;

rtemp_vars_t rtemp_vars;

const uint8_t rtemp_path0[]    = "temp";

//=========================== prototypes ======================================

error_t rtemp_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options);
void    rtemp_timer();
void    rtemp_sendDone(OpenQueueEntry_t* msg,
                       error_t error);

//=========================== public ==========================================

void rtemp_init() {
   // prepare the resource descriptor for the /temp path
   rtemp_vars.desc.path0len             = sizeof(rtemp_path0)-1;
   rtemp_vars.desc.path0val             = (uint8_t*)(&rtemp_path0);
   rtemp_vars.desc.path1len             = 0;
   rtemp_vars.desc.path1val             = NULL;
   rtemp_vars.desc.componentID          = COMPONENT_RTEMP;
   rtemp_vars.desc.callbackRx           = &rtemp_receive;
   rtemp_vars.desc.callbackTimer        = rtemp_timer;
   rtemp_vars.desc.callbackSendDone     = &rtemp_sendDone;
   
   rtemp_vars.delay                     = 0;
   
   opencoap_register(&rtemp_vars.desc);
}

//=========================== private =========================================

error_t rtemp_receive(OpenQueueEntry_t* msg,
                   coap_header_iht* coap_header,
                   coap_option_iht* coap_options) {
                      
   error_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      // set delay to RTEMPPERIOD to gets triggered next time rtemp_timer is called
      rtemp_vars.delay                  = RTEMPPERIOD;
      
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

void rtemp_timer() {
   OpenQueueEntry_t* pkt;
   error_t           outcome;
   uint8_t           numOptions;
   
   rtemp_vars.delay += 2;
   
   if (rtemp_vars.delay>=RTEMPPERIOD) {
      // create a CoAP RD packet
      pkt = openqueue_getFreePacketBuffer();
      if (pkt==NULL) {
         openserial_printError(COMPONENT_RTEMP,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         openqueue_freePacketBuffer(pkt);
         return;
      }
      // take ownership over that packet
      pkt->creator    = COMPONENT_RTEMP;
      pkt->owner      = COMPONENT_RTEMP;
      // CoAP payload (2 bytes of temperature data)
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0] = 0x12;
      pkt->payload[1] = 0x34;
      numOptions = 0;
      // add content-type option
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
                                         1;
      pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
      numOptions++;
      // metadata
      pkt->l4_destination_port         = WKP_UDP_COAP;
      pkt->l3_destinationORsource.type = ADDR_128B;
      memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_local,16);
      // send
      outcome = opencoap_send(pkt,
                              COAP_TYPE_NON,
                              COAP_CODE_REQ_PUT,
                              numOptions);
      // avoid overflowing the queue if fails
      if (outcome==E_FAIL) {
         openqueue_freePacketBuffer(pkt);
      }
      // reset the timer
      rtemp_vars.delay = 0;
   }
   return;
}

void rtemp_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}

inline uint8_t hexToAscii(uint8_t hex) {
   if (hex<0x0a) {
      return '0'+(hex-0x00);
   } else {
      return 'A'+(hex-0x0a);
   }
}