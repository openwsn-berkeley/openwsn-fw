#include "openwsn.h"
#include "rt.h"
#include  "opentimers.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "sensitive_accel_temperature.h"


//=========================== defines =========================================

/// inter-packet period (in mseconds)
#define RTPERIOD     20000

const uint8_t rt_path0[] = "t";

//=========================== variables =======================================

typedef struct {
   opentimer_id_t  timerId;
   coap_resource_desc_t desc;
} rt_vars_t;

rt_vars_t rt_vars;

//=========================== prototypes ======================================

owerror_t rt_receive(OpenQueueEntry_t* msg,
                      coap_header_iht*  coap_header,
                      coap_option_iht*  coap_options);
void    rt_timer();
void    rt_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void rt_init() {
   // startup the sensor
   sensitive_accel_temperature_init();
   
   // prepare the resource descriptor for the /temp path
   rt_vars.desc.path0len             = sizeof(rt_path0)-1;
   rt_vars.desc.path0val             = (uint8_t*)(&rt_path0);
   rt_vars.desc.path1len             = 0;
   rt_vars.desc.path1val             = NULL;
   rt_vars.desc.componentID          = COMPONENT_RT;
   rt_vars.desc.callbackRx           = &rt_receive;
   rt_vars.desc.callbackSendDone     = &rt_sendDone;
   

   rt_vars.timerId    = opentimers_start(openrandom_get16b()%RTPERIOD,
                                          TIMER_PERIODIC,TIME_MS,
                                          rt_timer);
   opencoap_register(&rt_vars.desc);
}

//=========================== private =========================================

owerror_t rt_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {
   owerror_t outcome;
   uint8_t rawdata[SENSITIVE_ACCEL_TEMPERATURE_DATALEN];
   
   if (coap_header->Code==COAP_CODE_REQ_POST) {
      // start/stop the data generation to data server
      
      if (msg->payload[0]=='1') {
         opentimers_restart(rt_vars.timerId);
      } else {
         opentimers_stop(rt_vars.timerId);
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
      
      // CoAP payload (2 bytes of temperature data)
      packetfunctions_reserveHeaderSize(msg,3);
      sensitive_accel_temperature_get_measurement(&rawdata[0]);
      msg->payload[0] = COAP_PAYLOAD_MARKER;
      msg->payload[1] = rawdata[8];
      msg->payload[2] = rawdata[9];
         
      // set the CoAP header
       coap_header->Code                = COAP_CODE_RESP_CONTENT;
      
      outcome                          = E_SUCCESS;
   
   } else {
      // return an error message
      outcome = E_FAIL;
   }
   
   return outcome;
}

void rt_timer() {
   OpenQueueEntry_t* pkt;
   owerror_t           outcome;
   uint8_t           numOptions;
   uint8_t           rawdata[SENSITIVE_ACCEL_TEMPERATURE_DATALEN];
   
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_RT);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_RT,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_RT;
   pkt->owner      = COMPONENT_RT;
   // CoAP payload (2 bytes of temperature data)
   packetfunctions_reserveHeaderSize(pkt,2);
   sensitive_accel_temperature_get_measurement(&rawdata[0]);
   pkt->payload[0] = rawdata[8];
   pkt->payload[1] = rawdata[9];
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(rt_path0)-1);
   memcpy(&pkt->payload[0],&rt_path0,sizeof(rt_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
      sizeof(rt_path0)-1;
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
                           &rt_vars.desc);
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void rt_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}