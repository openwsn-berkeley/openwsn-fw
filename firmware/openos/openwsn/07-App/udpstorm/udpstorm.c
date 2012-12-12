#include "openwsn.h"
#include "udpstorm.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "IEEE802154E.h"

//=========================== defines =========================================

/// inter-packet period (in ms)
#define UDPSTORMPERIOD            1000
#define NUMPACKETS                 300

const uint8_t udpstorm_path0[] =  "strm";

PRAGMA(pack(1));
typedef struct {
   uint16_t             seqNum;
} udpstorm_payload_t;
PRAGMA(pack());

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t       timerId;
   uint16_t             seqNum;
} udpstorm_vars_t;

udpstorm_vars_t udpstorm_vars;

//=========================== prototypes ======================================

error_t udpstorm_receive(OpenQueueEntry_t* msg,
                         coap_header_iht*  coap_header,
                         coap_option_iht*  coap_options);
void    udpstorm_timer_cb();
void    udpstorm_task_cb();
void    udpstorm_sendDone(OpenQueueEntry_t* msg,
                          error_t           error);

//=========================== public ==========================================

void udpstorm_init() {
   // prepare the resource descriptor for the path
   udpstorm_vars.desc.path0len             = sizeof(udpstorm_path0)-1;
   udpstorm_vars.desc.path0val             = (uint8_t*)(&udpstorm_path0);
   udpstorm_vars.desc.path1len             = 0;
   udpstorm_vars.desc.path1val             = NULL;
   udpstorm_vars.desc.componentID          = COMPONENT_UDPSTORM;
   udpstorm_vars.desc.callbackRx           = &udpstorm_receive;
   udpstorm_vars.desc.callbackSendDone     = &udpstorm_sendDone;
   
   opencoap_register(&udpstorm_vars.desc);
   udpstorm_vars.timerId     = opentimers_start(UDPSTORMPERIOD,
                                                TIMER_PERIODIC,TIME_MS,
                                                udpstorm_timer_cb);
   udpstorm_vars.seqNum      = 0;
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
   
   if(udpstorm_vars.seqNum>=NUMPACKETS) {
      // we've sent enough packets
      
      // stop the periodic timer
      opentimers_stop(udpstorm_vars.timerId);
      
      // reset the sequence number
      udpstorm_vars.seqNum = 0;
   }
   
   // if you get here, send a packet
   
   // get a packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_UDPSTORM);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_UDPSTORM,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator    = COMPONENT_UDPSTORM;
   pkt->owner      = COMPONENT_UDPSTORM;
   
   // add payload
   packetfunctions_reserveHeaderSize(pkt,sizeof(udpstorm_payload_t));
   ((udpstorm_payload_t*)(pkt->payload))->seqNum = udpstorm_vars.seqNum;
   
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(udpstorm_path0)-1);
   memcpy(&pkt->payload[0],&udpstorm_path0,sizeof(udpstorm_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                = (COAP_OPTION_LOCATIONPATH-COAP_OPTION_CONTENTTYPE) << 4 |
                                     sizeof(udpstorm_path0)-1;
   numOptions++;
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                = COAP_OPTION_CONTENTTYPE << 4 |
      1;
   pkt->payload[1]                = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_local,16);
   
   // send
   outcome = opencoap_send(pkt,
                           COAP_TYPE_NON,
                           COAP_CODE_REQ_PUT,
                           numOptions,
                           &udpstorm_vars.desc);
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   // increment counter
   udpstorm_vars.seqNum++;
}

void udpstorm_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openqueue_freePacketBuffer(msg);
}