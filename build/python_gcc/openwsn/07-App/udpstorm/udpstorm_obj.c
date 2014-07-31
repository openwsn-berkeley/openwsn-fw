/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:16.262085.
*/
#include "openwsn_obj.h"
#include "udpstorm_obj.h"
#include "opencoap_obj.h"
#include "opentimers_obj.h"
#include "openqueue_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "openrandom_obj.h"
#include "scheduler_obj.h"
//#include "ADC_Channel.h"
#include "IEEE802154E_obj.h"
#include "idmanager_obj.h"

//=========================== defines =========================================

const uint8_t udpstorm_path0[]    = "storm";
const uint8_t udpstorm_payload[]  = "OpenWSN";
static const uint8_t dst_addr[]   = {
   0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
}; 

//=========================== variables =======================================

// declaration of global variable _udpstorm_vars_ removed during objectification.

//=========================== prototypes ======================================

owerror_t udpstorm_receive(OpenMote* self, 
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);
void udpstorm_timer_cb(OpenMote* self);
void udpstorm_task_cb(OpenMote* self);
void udpstorm_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void udpstorm_init(OpenMote* self) {
   
   // register to OpenCoAP module
   (self->udpstorm_vars).desc.path0len              = sizeof(udpstorm_path0)-1;
   (self->udpstorm_vars).desc.path0val              = (uint8_t*)(&udpstorm_path0);
   (self->udpstorm_vars).desc.path1len              = 0;
   (self->udpstorm_vars).desc.path1val              = NULL;
   (self->udpstorm_vars).desc.componentID           = COMPONENT_UDPSTORM;
   (self->udpstorm_vars).desc.callbackRx            = &udpstorm_receive;
   (self->udpstorm_vars).desc.callbackSendDone      = &udpstorm_sendDone;
 opencoap_register(self, &(self->udpstorm_vars).desc);
   
   /*
   (self->udpstorm_vars).period           = 0;
   
   (self->udpstorm_vars).timerId                    = opentimers_start(self, 
      (self->udpstorm_vars).period,
      TIMER_PERIODIC,TIME_MS,
      udpstorm_timer_cb
   );
   
 opentimers_stop(self, (self->udpstorm_vars).timerId);
   */
}

//=========================== private =========================================

owerror_t udpstorm_receive(OpenMote* self, 
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
 packetfunctions_reserveHeaderSize(self, msg, 3);
         msg->payload[0]          = COAP_PAYLOAD_MARKER;
         
         // return as big endian
         msg->payload[1]          = (uint8_t)((self->udpstorm_vars).period >> 8);
         msg->payload[2]          = (uint8_t)((self->udpstorm_vars).period & 0xff);
         
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
         (self->udpstorm_vars).period     = 0;
         (self->udpstorm_vars).period    |= (msg->payload[0] << 8);
         (self->udpstorm_vars).period    |= msg->payload[1];
         
         /*
         // stop and start again only if period > 0
 opentimers_stop(self, (self->udpstorm_vars).timerId);
         
         if((self->udpstorm_vars).period > 0) {
 opentimers_setPeriod(self, (self->udpstorm_vars).timerId,TIME_MS,(self->udpstorm_vars).period);
 opentimers_restart(self, (self->udpstorm_vars).timerId);
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
void udpstorm_timer_cb(OpenMote* self){
 scheduler_push_task(self, udpstorm_task_cb,TASKPRIO_COAP);
}

void udpstorm_task_cb(OpenMote* self) {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              numOptions;
   
   // don't run if not synch
   if ( ieee154e_isSynch(self) == FALSE) return;
   
   // don't run on dagroot
   if ( idmanager_getIsDAGroot(self)) {
 opentimers_stop(self, (self->udpstorm_vars).timerId);
      return;
   }
   
   if((self->udpstorm_vars).period == 0) {
      // stop the periodic timer
 opentimers_stop(self, (self->udpstorm_vars).timerId);
      return;
   }
   
   // if you get here, send a packet
   
   // get a packet
   pkt = openqueue_getFreePacketBuffer(self, COMPONENT_UDPSTORM);
   if (pkt==NULL) {
 openserial_printError(self, COMPONENT_UDPSTORM,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
 openqueue_freePacketBuffer(self, pkt);
      return;
   }
   
   // take ownership over that packet
   pkt->creator    = COMPONENT_UDPSTORM;
   pkt->owner      = COMPONENT_UDPSTORM;
   
   // add payload
 packetfunctions_reserveHeaderSize(self, pkt,sizeof(udpstorm_payload)-1);
   memcpy(&pkt->payload[0],udpstorm_payload,sizeof(udpstorm_payload)-1);
   
   numOptions = 0;
   // location-path option
 packetfunctions_reserveHeaderSize(self, pkt,sizeof(udpstorm_path0)-1);
   memcpy(&pkt->payload[0],&udpstorm_path0,sizeof(udpstorm_path0)-1);
 packetfunctions_reserveHeaderSize(self, pkt,1);
   pkt->payload[0] = (COAP_OPTION_NUM_URIPATH) << 4 | (sizeof(udpstorm_path0)-1);
   numOptions++;
   
   // content-type option
 packetfunctions_reserveHeaderSize(self, pkt,2);
   pkt->payload[0] = COAP_OPTION_NUM_CONTENTFORMAT << 4 | 1;
   pkt->payload[1] = COAP_MEDTYPE_APPOCTETSTREAM;
   numOptions++;
   
   // metadata
   pkt->l4_destination_port = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&dst_addr,16);
   
   // send
   outcome = opencoap_send(self, 
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      numOptions,
      &(self->udpstorm_vars).desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
 openqueue_freePacketBuffer(self, pkt);
   }
}

void udpstorm_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openqueue_freePacketBuffer(self, msg);
}
