#include "openwsn.h"
#include "rxl1.h"
#include "opentimers.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "sensitive_accel_temperature.h"


//=========================== defines =========================================

/// inter-packet period (in mseconds)
#define RXL1PERIOD     6000

const uint8_t rxl1_path0[] = "x";

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t  timerId;
} rxl1_vars_t;

rxl1_vars_t rxl1_vars;

//=========================== prototypes ======================================

owerror_t rxl1_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options);
void    rxl1_timer();
void    rxl1_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void rxl1_init() {
   // startup the sensor
   sensitive_accel_temperature_init();
   
   // prepare the resource descriptor for the /temp path
   rxl1_vars.desc.path0len             = sizeof(rxl1_path0)-1;
   rxl1_vars.desc.path0val             = (uint8_t*)(&rxl1_path0);
   rxl1_vars.desc.path1len             = 0;
   rxl1_vars.desc.path1val             = NULL;
   rxl1_vars.desc.componentID          = COMPONENT_RXL1;
   rxl1_vars.desc.callbackRx           = &rxl1_receive;
   rxl1_vars.desc.callbackSendDone     = &rxl1_sendDone;
   
   //we start a timer, but just to get a timer ID, we stop it immediately
   rxl1_vars.timerId    = opentimers_start(openrandom_get16b()%RXL1PERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          rxl1_timer);
   opentimers_stop(rxl1_vars.timerId);
   
   opencoap_register(&rxl1_vars.desc);
}

//=========================== private =========================================

owerror_t rxl1_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   owerror_t outcome;
   uint8_t rawdata[SENSITIVE_ACCEL_TEMPERATURE_DATALEN];
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      // start/stop the data generation to data server
      
      if (msg->payload[0]=='1') {
         //restart timer
         opentimers_restart(rxl1_vars.timerId);
         
      } else {
         //stop timer
         opentimers_stop(rxl1_vars.timerId);   
      }
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // CoAP header
      coap_header->Code                = COAP_CODE_RESP_VALID;
      
      outcome = E_SUCCESS;
   
   } else if (coap_header->Code==COAP_CODE_REQ_GET) {
      // return current sensor value
      
      // reset packet payload
      msg->payload                     = &(msg->packet[127]);
      msg->length                      = 0;
      
      // CoAP payload (8 bytes of XL data)
      packetfunctions_reserveHeaderSize(msg,9);
      sensitive_accel_temperature_get_measurement(&rawdata[0]);
      msg->payload[0] = COAP_PAYLOAD_MARKER;
      memcpy(&msg->payload[1],&rawdata[8],8);
         
      // set the CoAP header
      coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   
   } else {
      // return an error message
      outcome = E_FAIL;
   }
   
   return outcome;
}

void rxl1_timer() {
   OpenQueueEntry_t* pkt;
   owerror_t           outcome;
   uint8_t           numOptions;
   uint8_t           rawdata[SENSITIVE_ACCEL_TEMPERATURE_DATALEN];
   
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_RXL1);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_RXL1,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_RXL1;
   pkt->owner      = COMPONENT_RXL1;
   // CoAP payload (2 bytes of temperature data)
   packetfunctions_reserveHeaderSize(pkt,2);
   sensitive_accel_temperature_get_measurement(&rawdata[0]);
   pkt->payload[0] = rawdata[8];
   pkt->payload[1] = rawdata[9];
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(rxl1_path0)-1);
   memcpy(&pkt->payload[0],&rxl1_path0,sizeof(rxl1_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
      sizeof(rxl1_path0)-1;
   numOptions++;
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
      1;
   pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   // metadata
   pkt->l4_destination_port         = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motesEecs,16);
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_PUT,
                           numOptions,
                           &rxl1_vars.desc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   
   return;
}

void rxl1_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
