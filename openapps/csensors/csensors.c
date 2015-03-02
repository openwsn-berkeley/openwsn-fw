/**
\brief A CoAP resource which allows an application to GET/SET the state of
   sensors.
*/

#include "opendefs.h"
#include "csensors.h"
#include "opencoap.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "idmanager.h"
#include "opensensors.h"
#include "opentimers.h"
#include "scheduler.h"
#include "openserial.h"
#include "IEEE802154E.h"
#include "openrandom.h"

//=========================== defines =========================================

/// task list used for matching callbacks from scheduler to the related timers
#define CSENSORSTASKLIST 40

const uint8_t csensors_path0[]       = "s";

//=========================== variables =======================================

csensors_vars_t               csensors_vars;
uint8_t                       cb_vars[CSENSORSTASKLIST];
uint8_t                       cb_put = 0;
uint8_t                       cb_get = 0;

//=========================== prototypes ======================================

void csensors_register(
   csensors_resource_t* csensors_resource,
   opensensors_resource_desc_t* opensensors_resource_desc
);

owerror_t csensors_receive(
   OpenQueueEntry_t* msg,
   coap_header_iht*  coap_header,
   coap_option_iht*  coap_options
);

void csensors_timer_cb(opentimer_id_t id);

void csensors_task_cb(void);

void csensors_setPeriod(
   uint32_t period,
   uint8_t id
);

void csensors_fillpayload(
   OpenQueueEntry_t* msg,
   uint8_t           id
);

void csensors_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);

//=========================== public ==========================================

void csensors_init() {
   uint8_t i;
   opensensors_vars_t* opensensors;

   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return;

   memset(&csensors_vars,0,sizeof(csensors_vars_t));

   csensors_vars.desc.path0len         = sizeof(csensors_path0)-1;
   csensors_vars.desc.path0val         = (uint8_t*)(&csensors_path0);
   csensors_vars.desc.path1len         = 0;
   csensors_vars.desc.path1val         = NULL;
   csensors_vars.desc.componentID      = COMPONENT_CSENSORS;
   csensors_vars.desc.discoverable     = TRUE;
   csensors_vars.desc.callbackRx       = &csensors_receive;
   csensors_vars.desc.callbackSendDone = &csensors_sendDone;
   // register with the CoAP module
   opencoap_register(&csensors_vars.desc);

   opensensors = opensensors_read();
   csensors_vars.numSensors = opensensors->numSensors;

   for(i=0;i<csensors_vars.numSensors;i++) {
      csensors_vars.csensors_resource[i].timerId = MAX_NUM_TIMERS;
      csensors_vars.csensors_resource[i].period  = 0;
      csensors_register(&csensors_vars.csensors_resource[i],&opensensors->opensensors_resource[i]);
   }
}

//=========================== private =========================================

void csensors_register(
      csensors_resource_t* csensors_resource,
      opensensors_resource_desc_t* opensensors_resource_desc
   ) {
   csensors_resource->desc.path0len         = sizeof(csensors_path0)-1;
   csensors_resource->desc.path0val         = (uint8_t*)(&csensors_path0);
   csensors_resource->desc.path1len         = opensensors_resource_desc->path1len;
   csensors_resource->desc.path1val         = opensensors_resource_desc->path1val;
   csensors_resource->desc.componentID      = COMPONENT_CSENSORS;
   csensors_resource->desc.discoverable     = TRUE;
   csensors_resource->desc.callbackRx       = &csensors_receive;
   csensors_resource->desc.callbackSendDone = &csensors_sendDone;

   // register with the CoAP module
   opencoap_register(&csensors_resource->desc);
}

/**
\brief Called when a CoAP message is received for this resource.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t csensors_receive(
      OpenQueueEntry_t* msg,
      coap_header_iht*  coap_header,
      coap_option_iht*  coap_options
   ) {
   owerror_t outcome;
   uint8_t   id,i;
   uint32_t  period;


   switch (coap_header->Code) {
      case COAP_CODE_REQ_GET:
         // reset packet payload
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;

         if (coap_options[1].type != COAP_OPTION_NUM_URIPATH) {

            // have CoAP module write links to csensors resources
            opencoap_writeLinks(msg,COMPONENT_CSENSORS);

            packetfunctions_reserveHeaderSize(msg,1);
            msg->payload[0]     = COAP_PAYLOAD_MARKER;

            // add return option
            packetfunctions_reserveHeaderSize(msg,2);
            msg->payload[0]     = COAP_OPTION_NUM_CONTENTFORMAT << 4 | 1;
            msg->payload[1]     = COAP_MEDTYPE_APPLINKFORMAT;

         } else {

            for(id=0;id<csensors_vars.numSensors;id++) {
               if (
                  memcmp(
                     coap_options[1].pValue,
                     csensors_vars.csensors_resource[id].desc.path1val,
                     csensors_vars.csensors_resource[id].desc.path1len
                  )==0
                  ) {
                  break;
               }
            }
            csensors_fillpayload(msg,id);
            packetfunctions_reserveHeaderSize(msg,2);
            msg->payload[0] = (COAP_OPTION_NUM_CONTENTFORMAT << 4) | 1;
            msg->payload[1] = COAP_MEDTYPE_APPOCTETSTREAM;
         }
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CONTENT;

         outcome                          = E_SUCCESS;

         break;

      case COAP_CODE_REQ_PUT:

         period = 0;

         for(i=0;i<msg->length;i++) {
            if ((uint8_t)msg->payload[i]>=(uint8_t)'0' && (uint8_t)msg->payload[i]<=(uint8_t)'0'+9) {
               period = period*10 + (uint8_t)msg->payload[i] - (uint8_t)'0';
            } else {
               period = 0;
               break;
            }
         }

         if (coap_options[1].type == COAP_OPTION_NUM_URIPATH) {

            for(id=0;id<csensors_vars.numSensors;id++) {
               if (
                  memcmp(
                     coap_options[1].pValue,
                     csensors_vars.csensors_resource[id].desc.path1val,
                     csensors_vars.csensors_resource[id].desc.path1len
                  )==0
                  ) {
                  break;
               }
            }

            csensors_setPeriod(period,id);

         } else {

            for(id=0;id<csensors_vars.numSensors;id++) {
               csensors_setPeriod(period,id);
            }
         }

         // reset packet payload
         msg->payload                     = &(msg->packet[127]);
         msg->length                      = 0;
         // set the CoAP header
         coap_header->Code                = COAP_CODE_RESP_CHANGED;

         outcome                          = E_SUCCESS;
         break;

      default:
         outcome                          = E_FAIL;
         break;
   }

   return outcome;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void csensors_timer_cb(opentimer_id_t id){
   uint8_t i;

   for(i=0;i<csensors_vars.numSensors;i++) {
      if (csensors_vars.csensors_resource[i].timerId == id) {
         cb_vars[cb_put] = i;
         cb_put = (cb_put+1)%CSENSORSTASKLIST;
         opentimers_setPeriod(
            csensors_vars.csensors_resource[i].timerId,
            TIME_MS,
            csensors_vars.csensors_resource[i].period
         );
         break;
      }
   }
   scheduler_push_task(csensors_task_cb,TASKPRIO_COAP);
}

void csensors_task_cb() {
   OpenQueueEntry_t*          pkt;
   owerror_t                  outcome;
   uint8_t                    id = cb_vars[cb_get];

   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(csensors_vars.csensors_resource[id].timerId);
      return;
   }

   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CSENSORS);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_CSENSORS,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return;
   }

   // take ownership over that packet
   pkt->creator                   = COMPONENT_CSENSORS;
   pkt->owner                     = COMPONENT_CSENSORS;

   // CoAP payload
   csensors_fillpayload(pkt,id);

   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                = (COAP_OPTION_NUM_CONTENTFORMAT - COAP_OPTION_NUM_URIPATH) << 4
                                       | 1;
   pkt->payload[1]                = COAP_MEDTYPE_APPOCTETSTREAM;

   // location-path1 option
   packetfunctions_reserveHeaderSize(pkt,1+csensors_vars.csensors_resource[id].desc.path1len);
   memcpy(&pkt->payload[1],csensors_vars.csensors_resource[id].desc.path1val,csensors_vars.csensors_resource[id].desc.path1len);
   pkt->payload[0]                =  sizeof(csensors_path0)-1;
   // location-path0 option
   packetfunctions_reserveHeaderSize(pkt,sizeof(csensors_path0));
   memcpy(&pkt->payload[1],csensors_path0,sizeof(csensors_path0)-1);
   pkt->payload[0]                = ((COAP_OPTION_NUM_URIPATH) << 4) | (sizeof(csensors_path0)-1);

   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type    = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_ringmaster,16);

   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      2,
      &csensors_vars.csensors_resource[id].desc
   );

   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   } else {
      cb_get = (cb_get+1)%CSENSORSTASKLIST;
   }

   return;
}

/**
\brief Called when receiving a CoAP PUT.

\param[in] period       The period used for reporting data.
\param[in] id           Resource id in sensors array.

*/
void csensors_setPeriod(uint32_t period,
      uint8_t id) {

   uint32_t old_period = csensors_vars.csensors_resource[id].period;
   float denominator = 65535;

   if (period>0) {
      csensors_vars.csensors_resource[id].period = period;
      if (csensors_vars.csensors_resource[id].timerId != MAX_NUM_TIMERS) {
         opentimers_setPeriod(
            csensors_vars.csensors_resource[id].timerId,
            TIME_MS,
            (uint32_t)(period*(openrandom_get16b()/denominator)));
         if (old_period==0) {
            opentimers_restart(csensors_vars.csensors_resource[id].timerId);
         }
      } else {
         csensors_vars.csensors_resource[id].timerId = opentimers_start(
            (uint32_t)(period*(openrandom_get16b()/denominator)),
            TIMER_PERIODIC,TIME_MS,
            csensors_timer_cb);
      }
   } else {
      if ((csensors_vars.csensors_resource[id].timerId != MAX_NUM_TIMERS) && (old_period != 0)) {
         csensors_vars.csensors_resource[id].period = period;
         opentimers_stop(csensors_vars.csensors_resource[id].timerId);
      }
   }
}

/**
\brief Called when a CoAP message has to fill the payload with csensors data.

\param[in] msg          The message to be filled.
\param[in] id           Resource id in sensors array.

*/
void csensors_fillpayload(OpenQueueEntry_t* msg,
      uint8_t id) {

   uint16_t              value;
   opensensors_vars_t*   opensensors = opensensors_read();

   value=opensensors->opensensors_resource[id].callbackRead();

   packetfunctions_reserveHeaderSize(msg,3);

   // add CoAP payload
   msg->payload[0]                  = COAP_PAYLOAD_MARKER;

   // add value
   msg->payload[1]                  = (value>>8) & 0x00ff;
   msg->payload[2]                  = value & 0x00ff;


}


/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void csensors_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}
