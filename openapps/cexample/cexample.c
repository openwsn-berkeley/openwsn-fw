/**
\brief An example CoAP application.
*/

#include "opendefs.h"
#include "cexample.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "idmanager.h"
#include "IEEE802154E.h"

//=========================== defines =========================================

/// inter-packet period (in ms)
#define CEXAMPLEPERIOD  10000
#define PAYLOADLEN      40

const uint8_t cexample_path0[] = "ex";

//=========================== variables =======================================

cexample_vars_t cexample_vars;

//=========================== prototypes ======================================

owerror_t cexample_receive( OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen);

void    cexample_timer_cb(opentimers_id_t id);
void    cexample_task_cb(void);
void    cexample_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);

//=========================== public ==========================================

void cexample_init() {
   
    // prepare the resource descriptor for the /ex path
    cexample_vars.desc.path0len             = sizeof(cexample_path0)-1;
    cexample_vars.desc.path0val             = (uint8_t*)(&cexample_path0);
    cexample_vars.desc.path1len             = 0;
    cexample_vars.desc.path1val             = NULL;
    cexample_vars.desc.componentID          = COMPONENT_CEXAMPLE;
    cexample_vars.desc.securityContext      = NULL;
    cexample_vars.desc.discoverable         = TRUE;
    cexample_vars.desc.callbackRx           = &cexample_receive;
    cexample_vars.desc.callbackSendDone     = &cexample_sendDone;
    
    
    opencoap_register(&cexample_vars.desc);
    cexample_vars.timerId    = opentimers_create();
    opentimers_scheduleIn(
        cexample_vars.timerId, 
        CEXAMPLEPERIOD, 
        TIME_MS, 
        TIMER_PERIODIC,
        cexample_timer_cb
    );
}

//=========================== private =========================================

owerror_t cexample_receive( OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen) {

   return E_FAIL;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void cexample_timer_cb(opentimers_id_t id){
   scheduler_push_task(cexample_task_cb,TASKPRIO_COAP);
}

void cexample_task_cb() {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
   uint8_t              i;
   coap_option_iht      options[2];
   
   uint16_t             x_int       = 0;
   uint16_t             sum         = 0;
   uint16_t             avg         = 0;
   uint8_t              N_avg       = 10;
   uint8_t              medtype;
   
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;
   
   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_destroy(cexample_vars.timerId);
      return;
   }
   
   for (i = 0; i < N_avg; i++) {
      sum += x_int;
   }
   avg = sum/N_avg;
   
   // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CEXAMPLE);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_CEXAMPLE,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return;
   }
   // take ownership over that packet
   pkt->creator                   = COMPONENT_CEXAMPLE;
   pkt->owner                     = COMPONENT_CEXAMPLE;
   // CoAP payload
   packetfunctions_reserveHeaderSize(pkt,PAYLOADLEN);
   for (i=0;i<PAYLOADLEN;i++) {
      pkt->payload[i]             = i;
   }
   avg = openrandom_get16b();
   pkt->payload[0]                = (avg>>8)&0xff;
   pkt->payload[1]                = (avg>>0)&0xff;

   // set location-path option
   options[0].type = COAP_OPTION_NUM_URIPATH;
   options[0].length = sizeof(cexample_path0) - 1;
   options[0].pValue = (uint8_t *) cexample_path0;

   // set content-type option
   medtype = COAP_MEDTYPE_APPOCTETSTREAM;
   options[1].type = COAP_OPTION_NUM_CONTENTFORMAT;
   options[1].length = 1;
   options[1].pValue = &medtype;
   
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type    = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_motesEecs,16);
   
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      1, // token len
      options,
      2, // options len
      &cexample_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
   
   return;
}

void cexample_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    openqueue_freePacketBuffer(msg);
}



