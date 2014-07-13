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
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t udpstorm_path0[]    = "storm";
const uint8_t udpstorm_payload[]  = "OpenWSN";
static const uint8_t dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
}; 

//=========================== variables =======================================

udpstorm_vars_t udpstorm_vars;

//=========================== prototypes ======================================

owerror_t udpstorm_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void udpstorm_timer_cb(void);
void udpstorm_task_cb(void);
void udpstorm_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void udpstorm_init(void) {
   
   // register to OpenCoAP module
   udpstorm_vars.desc.path0len              = sizeof(udpstorm_path0)-1;
   udpstorm_vars.desc.path0val              = (uint8_t*)(&udpstorm_path0);
   udpstorm_vars.desc.path1len              = 0;
   udpstorm_vars.desc.path1val              = NULL;
   udpstorm_vars.desc.componentID           = COMPONENT_UDPSTORM;
   udpstorm_vars.desc.callbackRx            = &udpstorm_receive;
   udpstorm_vars.desc.callbackSendDone      = &udpstorm_sendDone;
   opencoap_register(&udpstorm_vars.desc);
   
   /*
   udpstorm_vars.period           = 0;
   
   udpstorm_vars.timerId                    = opentimers_start(
      udpstorm_vars.period,
      TIMER_PERIODIC,TIME_MS,
      udpstorm_timer_cb
   );
   
   opentimers_stop(udpstorm_vars.timerId);
   */
}

//=========================== private =========================================

owerror_t udpstorm_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   owerror_t outcome;
   
   switch (coap_header->Code) {
      
      case COAP_CODE_REQ_GET:
         
         // reset packet payload
         msg->payload             = &(msg->packet[127]);
         msg->length              = 0;
         
         // add CoAP payload
         packetfunctions_reserveHeaderSize(msg, 3);
         msg->payload[0]          = COAP_PAYLOAD_MARKER;
         
         // return as big endian
         msg->payload[1]          = (uint8_t)(udpstorm_vars.period >> 8);
         msg->payload[2]          = (uint8_t)(udpstorm_vars.period & 0xff);
         
         // set the CoAP header
         coap_header->Code        = COAP_CODE_RESP_CONTENT;
         
         outcome                  = E_SUCCESS;
         break;
      
      case COAP_CODE_REQ_PUT:
         
         if (msg->length!=2) {
            outcome               = E_FAIL;
            coap_header->Code     = COAP_CODE_RESP_BADREQ;
         }
         
         // read the new period
         udpstorm_vars.period     = 0;
         udpstorm_vars.period    |= (msg->payload[0] << 8);
         udpstorm_vars.period    |= msg->payload[1];
         
         /*
         // stop and start again only if period > 0
         opentimers_stop(udpstorm_vars.timerId);
         
         if(udpstorm_vars.period > 0) {
            opentimers_setPeriod(udpstorm_vars.timerId,TIME_MS,udpstorm_vars.period);
            opentimers_restart(udpstorm_vars.timerId);
         }
         */
         
         // reset packet payload
         msg->payload             = &(msg->packet[127]);
         msg->length              = 0;
         
         // set the CoAP header
         coap_header->Code        = COAP_CODE_RESP_CHANGED;
         
         outcome                  = E_SUCCESS;
         break;
      
      default:
         outcome = E_FAIL;
         break;
   }
   
   return outcome;
}

/**
\note timer fired, but we don't want to execute task in ISR mode instead, push
   task to scheduler with CoAP priority, and let scheduler take care of it.
*/
void udpstorm_timer_cb(){
   scheduler_push_task(udpstorm_task_cb,TASKPRIO_COAP);
}

void udpstorm_task_cb() {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              numOptions;
   
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(udpstorm_vars.timerId);
      return;
   }
   
   if(udpstorm_vars.period == 0) {
      // stop the periodic timer
      opentimers_stop(udpstorm_vars.timerId);
      return;
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
   packetfunctions_reserveHeaderSize(pkt,sizeof(udpstorm_payload)-1);
   memcpy(&pkt->payload[0],udpstorm_payload,sizeof(udpstorm_payload)-1);
   
   numOptions = 0;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(udpstorm_path0)-1);
   memcpy(&pkt->payload[0],&udpstorm_path0,sizeof(udpstorm_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = (COAP_OPTION_NUM_URIPATH) << 4 | (sizeof(udpstorm_path0)-1);
   numOptions++;
   
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0] = COAP_OPTION_NUM_CONTENTFORMAT << 4 | 1;
   pkt->payload[1] = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   
   // metadata
   pkt->l4_destination_port = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&dst_addr,16);
   
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      numOptions,
      &udpstorm_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void udpstorm_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
