#include "openwsn.h"
#include "rreg.h"
#include "opentimers.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"
#include "board.h"
#include "scheduler.h"

//=========================== variables =======================================

#define RREGPERIOD       30000

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t  timerId;
} rreg_vars_t;

rreg_vars_t rreg_vars;

const uint8_t rreg_path0[]    = "r";

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
   rreg_vars.desc.callbackSendDone     = &rreg_sendDone;
   

   
   opencoap_register(&rreg_vars.desc);
   // register to the RD server every 30s
   rreg_vars.timerId    = opentimers_start(RREGPERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          rreg_timer);
}

//=========================== private =========================================

error_t rreg_receive(OpenQueueEntry_t* msg,
                   coap_header_iht* coap_header,
                   coap_option_iht* coap_options) {
                      
   error_t outcome;
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      // request to register received
      
      // triggered: schedule task to execute timer function next
      scheduler_push_task(rreg_timer,TASKPRIO_COAP);
      //call timer here, but reset timer after
      
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // set the CoAP header
      coap_header->OC                  = 0;
      coap_header->Code                = COAP_CODE_RESP_VALID;
      
      outcome = E_SUCCESS;
   } else if (coap_header->T==COAP_TYPE_ACK) {
      // it worked!
   } else {
      outcome = E_FAIL;
   }
   
   return outcome;
}

void rreg_timer() {
   OpenQueueEntry_t* pkt;
   uint8_t           temp8b;
   error_t           outcome;
   uint8_t           numOptions;
   

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
   numOptions = 0;
   // URI-query
   packetfunctions_reserveHeaderSize(pkt,sizeof(rreg_uriquery)-1+2);
   memcpy(&pkt->payload[0],&rreg_uriquery,sizeof(rreg_uriquery)-1);
   temp8b = idmanager_getMyID(ADDR_16B)->addr_16b[1];
   pkt->payload[sizeof(rreg_uriquery)-1] = hexToAscii((temp8b>>4) & 0x0f);
   pkt->payload[sizeof(rreg_uriquery)-0] = hexToAscii((temp8b>>0) & 0x0f);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_URIQUERY-COAP_OPTION_URIPATH) << 4 |
      sizeof(rreg_uriquery)-1+2;
   numOptions++;
   // URI-path
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0] = 'r';
   pkt->payload[1] = 'd';
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_URIPATH-COAP_OPTION_CONTENTTYPE) << 4 |
      2;
   numOptions++;
   // add content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                  = COAP_OPTION_CONTENTTYPE << 4 |
      1;
   pkt->payload[1]                  = COAP_MEDTYPE_APPLINKFORMAT;
   numOptions++;
   // metadata
   pkt->l4_destination_port         = WKP_UDP_COAP;
   pkt->l3_destinationORsource.type = ADDR_128B;
   memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_ipsoRD,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_CON,
                           COAP_CODE_REQ_POST,
                           numOptions,
                           &rreg_vars.desc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void rreg_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}

inline uint8_t hexToAscii(uint8_t hex) {
   if (hex<0x0a) {
      return '0'+(hex-0x00);
   } else {
      return 'A'+(hex-0x0a);
   }
}