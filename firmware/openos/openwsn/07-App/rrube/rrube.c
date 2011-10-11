#include "openwsn.h"
#include "rrube.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"
#include "board.h"
#include "heli.h"

//=========================== defines =========================================

typedef enum {
   RRUBE_ST_IDLE,
   RRUBE_ST_POSTRXD,
   RRUBE_ST_PUTRXD,
   RRUBE_ST_WAITACK,
} rrube_state_t;

//=========================== variables =======================================

typedef struct {
   rrube_state_t        rrube_state;
   coap_resource_desc_t desc;
   open_addr_t          nextHop;
} rrube_vars_t;

rrube_vars_t rrube_vars;

const uint8_t rrube_path0[]    = "g";

//=========================== prototypes ======================================

error_t rrube_receive(OpenQueueEntry_t* msg,
                     coap_header_iht*  coap_header,
                     coap_option_iht*  coap_options);
void    rrube_timer();
void    rrube_sendDone(OpenQueueEntry_t* msg,
                      error_t error);
uint8_t hexToAscii(uint8_t hex);

//=========================== public ==========================================

void rrube_init() {
   heli_init();
   
   rrube_vars.nextHop.type              = ADDR_128B;
   rrube_vars.rrube_state               = RRUBE_ST_IDLE;
   
   // prepare the resource descriptor for the /.well-known/core path
   rrube_vars.desc.path0len             = sizeof(rrube_path0)-1;
   rrube_vars.desc.path0val             = (uint8_t*)(&rrube_path0);
   rrube_vars.desc.path1len             = 0;
   rrube_vars.desc.path1val             = NULL;
   rrube_vars.desc.componentID          = COMPONENT_RRUBE;
   rrube_vars.desc.callbackRx           = &rrube_receive;
   rrube_vars.desc.callbackTimer        = &rrube_timer;
   rrube_vars.desc.callbackSendDone     = &rrube_sendDone;
   
   opencoap_register(&rrube_vars.desc);
}

//=========================== private =========================================

error_t rrube_receive(OpenQueueEntry_t* msg,
                   coap_header_iht* coap_header,
                   coap_option_iht* coap_options) {
                      
   error_t outcome;
   
   if (rrube_vars.rrube_state==RRUBE_ST_IDLE &&
       coap_header->Code==COAP_CODE_REQ_POST) {
      // record the next hop's address
      memcpy(&rrube_vars.nextHop.addr_128b[0],&msg->payload[0],16);
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_VALID;
      
      // advance state machine
      rrube_vars.rrube_state           = RRUBE_ST_POSTRXD;
      
      outcome = E_SUCCESS;
   } else if (rrube_vars.rrube_state==RRUBE_ST_IDLE &&
      coap_header->Code==COAP_CODE_REQ_PUT) {
      heli_on();     
   } else {
      outcome = E_FAIL;
   }
   
   return outcome;
}

void rrube_timer() {
   OpenQueueEntry_t* pkt;
   uint8_t           numOptions;
   error_t           outcome;
   
   heli_off();
   
   if (rrube_vars.rrube_state == RRUBE_ST_POSTRXD) {
      // I received a POST from the server, I need to send data to the next hop
      
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
      pkt->creator    = COMPONENT_RRUBE;
      pkt->owner      = COMPONENT_RRUBE;
      numOptions      = 0;
      // URI-path
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0] = 'g';
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0] = (COAP_OPTION_URIPATH) << 4 |
                        2;
      numOptions++;
      // metadata
      pkt->l4_destination_port         = WKP_UDP_COAP;
      pkt->l3_destinationORsource.type = ADDR_128B;
      memcpy(&pkt->l3_destinationORsource,&rrube_vars.nextHop,sizeof(open_addr_t));
      // send
      outcome = opencoap_send(pkt,
                              COAP_TYPE_NON,
                              COAP_CODE_REQ_PUT,
                              numOptions,
                              &rrube_vars.desc);
      // avoid overflowing the queue if fails
      if (outcome==E_FAIL) {
         openqueue_freePacketBuffer(pkt);
      }
      // advance state machine
      rrube_vars.rrube_state           = RRUBE_ST_IDLE;
   }
   return;
}

void rrube_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}