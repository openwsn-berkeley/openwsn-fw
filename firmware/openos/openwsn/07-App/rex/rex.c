#include "openwsn.h"
#include "rex.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"

//=========================== defines =========================================

/// inter-packet period (in seconds)
#define REXPERIOD     1
#define PAYLOADLEN    62

const uint8_t rex_path0[] = "rex";

//=========================== variables =======================================

typedef struct {
   uint16_t             delay;
   coap_resource_desc_t desc;
} rex_vars_t;

rex_vars_t rex_vars;

//=========================== prototypes ======================================

error_t rex_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);
void    rex_timer();
void    rex_sendDone(OpenQueueEntry_t* msg,
                       error_t error);

//=========================== public ==========================================

void rex_init() {
   
   // prepare the resource descriptor for the /rex path
   rex_vars.desc.path0len             = sizeof(rex_path0)-1;
   rex_vars.desc.path0val             = (uint8_t*)(&rex_path0);
   rex_vars.desc.path1len             = 0;
   rex_vars.desc.path1val             = NULL;
   rex_vars.desc.componentID          = COMPONENT_REX;
   rex_vars.desc.callbackRx           = &rex_receive;
   rex_vars.desc.callbackTimer        = rex_timer;
   rex_vars.desc.callbackSendDone     = &rex_sendDone;
   
   rex_vars.delay                     = REXPERIOD;
   
   opencoap_register(&rex_vars.desc);
}

//=========================== private =========================================

error_t rex_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   return E_FAIL;
}

void rex_timer() {
   OpenQueueEntry_t* pkt;
   error_t           outcome;
   uint8_t           numOptions;
   uint8_t           i;
   
   rex_vars.delay += 2;
   
   if (rex_vars.delay>=REXPERIOD) {
      // create a CoAP RD packet
      pkt = openqueue_getFreePacketBuffer();
      if (pkt==NULL) {
         openserial_printError(COMPONENT_REX,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         openqueue_freePacketBuffer(pkt);
         return;
      }
      // take ownership over that packet
      pkt->creator    = COMPONENT_REX;
      pkt->owner      = COMPONENT_REX;
      // CoAP payload
      packetfunctions_reserveHeaderSize(pkt,PAYLOADLEN);
      for (i=0;i<PAYLOADLEN;i++) {
         pkt->payload[i] = i;
      }
      numOptions = 0;
      // location-path option
      packetfunctions_reserveHeaderSize(pkt,sizeof(rex_path0)-1);
      memcpy(&pkt->payload[0],&rex_path0,sizeof(rex_path0)-1);
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0]                  = (COAP_OPTION_LOCATIONPATH-COAP_OPTION_CONTENTTYPE) << 4 |
                                         sizeof(rex_path0)-1;
      numOptions++;
      // content-type option
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
                                         1;
      pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
      numOptions++;
      // metadata
      pkt->l4_destination_port         = WKP_UDP_COAP;
      pkt->l3_destinationORsource.type = ADDR_128B;
      memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_motesEecs,16);
      // send
      outcome = opencoap_send(pkt,
                              COAP_TYPE_NON,
                              COAP_CODE_REQ_PUT,
                              numOptions,
                              &rex_vars.desc);
      // avoid overflowing the queue if fails
      if (outcome==E_FAIL) {
         openqueue_freePacketBuffer(pkt);
      }
      // reset the timer
      rex_vars.delay = 0;
   }
   return;
}

void rex_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}