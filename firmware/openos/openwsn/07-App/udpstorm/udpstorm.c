#include "openwsn.h"
#include "udpstorm.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
#include "ADC_Channel.h"

#include "IEEE802154E.h"
//=========================== defines =========================================

/// inter-packet period (in ms)
#define UDPSTORMPERIOD            1000
#define SAMPLE                    300

const uint8_t udpstorm_path0[] =  "ust";
//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t  timerId;
   uint32_t  sequence;
   uint32_t  rate;
} udpstorm_vars_t;

udpstorm_vars_t udpstorm_vars;

//=========================== prototypes ======================================

error_t udpstorm_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);
void    udpstorm_timer_cb();
void    udpstorm_task_cb();
void    udpstorm_sendDone(OpenQueueEntry_t* msg,
                       error_t error);

//=========================== public ==========================================

void udpstorm_init() {
   // prepare the resource descriptor for the /ust path
   udpstorm_vars.desc.path0len             = sizeof(udpstorm_path0)-1;
   udpstorm_vars.desc.path0val             = (uint8_t*)(&udpstorm_path0);
   udpstorm_vars.desc.path1len             = 0;
   udpstorm_vars.desc.path1val             = NULL;
   udpstorm_vars.desc.componentID          = 0xcc; //just a number
   udpstorm_vars.desc.callbackRx           = &udpstorm_receive;
   udpstorm_vars.desc.callbackSendDone     = &udpstorm_sendDone;
   
   opencoap_register(&udpstorm_vars.desc);
   udpstorm_vars.timerId     = opentimers_start(UDPSTORMPERIOD,
                                                TIMER_PERIODIC,TIME_MS,
                                                udpstorm_timer_cb);
   udpstorm_vars.sequence    = 0;
}

//=========================== private =========================================

error_t udpstorm_receive(OpenQueueEntry_t* msg,
                         coap_header_iht* coap_header,
                         coap_option_iht* coap_options) {
   return E_FAIL;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with CoAP priority, and let scheduler take care of it
void udpstorm_timer_cb(){
   scheduler_push_task(udpstorm_task_cb,TASKPRIO_COAP);
}

void udpstorm_task_cb() {
   OpenQueueEntry_t* pkt;
   error_t           outcome;
   uint8_t           numOptions;
   demo_t            demoPayload;
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(0xcc);
   if (pkt==NULL) {
      openserial_printError(0xcc,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = 0xcc; //just a number randomly picked for this temp app
   pkt->owner      = 0xcc;
   // CoAP payload
   packetfunctions_reserveHeaderSize(pkt,sizeof(demo_t));
   construct_demo(&demoPayload);
   memcpy((pkt->payload),&demoPayload,sizeof(demo_t));
   
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(udpstorm_path0)-1);
   memcpy(&pkt->payload[0],&udpstorm_path0,sizeof(udpstorm_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                  = (COAP_OPTION_LOCATIONPATH-COAP_OPTION_CONTENTTYPE) << 4 |
      sizeof(udpstorm_path0)-1;
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
   memcpy(&pkt->l3_destinationORsource.addr_128b[0],&ipAddr_local,16);
   
   // send
   if(udpstorm_vars.sequence<SAMPLE)
   {
      udpstorm_vars.sequence++;
      outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_PUT,
                           numOptions,
                           &udpstorm_vars.desc);
   }
   else{
      opentimers_stop(udpstorm_vars.timerId);
      openqueue_freePacketBuffer(pkt);
   }
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void udpstorm_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}

void construct_demo(demo_t* data) {
        data->seq[0] = (uint8_t)((udpstorm_vars.sequence & 0xff000000)>>24);
        data->seq[1] = (uint8_t)((udpstorm_vars.sequence & 0x00ff0000)>>16);
        data->seq[2] = (uint8_t)((udpstorm_vars.sequence & 0x0000ff00)>>8);
        data->seq[3] = (uint8_t)(udpstorm_vars.sequence & 0x00000000ff);
}