/**
\brief An example CoAP application.
*/

#include "opendefs.h"
#include "cjoin.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "scheduler.h"
//#include "ADC_Channel.h"
#include "idmanager.h"
#include "IEEE802154E.h"
#include "icmpv6rpl.h"

//=========================== defines =========================================

/// inter-packet period (in ms)
#define NUMBER_OF_EXCHANGES     6 

const uint8_t cjoin_path0[] = "j";

static const uint8_t ipAddr_jce[] = {0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};


//=========================== variables =======================================

cjoin_vars_t cjoin_vars;

//=========================== prototypes ======================================

owerror_t cjoin_receive(OpenQueueEntry_t* msg,
                    coap_header_iht*  coap_header,
                    coap_option_iht*  coap_options);
void    cjoin_timer_cb(opentimer_id_t id);
void    cjoin_task_cb(void);
void    cjoin_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);
owerror_t cjoin_sendPut(uint8_t payload); 
//=========================== public ==========================================

void cjoin_init() {
   
   // prepare the resource descriptor for the /j path
   cjoin_vars.desc.path0len             = sizeof(cjoin_path0)-1;
   cjoin_vars.desc.path0val             = (uint8_t*)(&cjoin_path0);
   cjoin_vars.desc.path1len             = 0;
   cjoin_vars.desc.path1val             = NULL;
   cjoin_vars.desc.componentID          = COMPONENT_CJOIN;
   cjoin_vars.desc.discoverable         = TRUE;
   cjoin_vars.desc.callbackRx           = &cjoin_receive;
   cjoin_vars.desc.callbackSendDone     = &cjoin_sendDone;
   cjoin_vars.joined = 0;
   
   
   opencoap_register(&cjoin_vars.desc);

   cjoin_schedule();
}

void cjoin_schedule() {
    uint16_t delay;
    
    if (cjoin_vars.joined == 0) {
        delay = openrandom_get16b();
        cjoin_vars.timerId    = opentimers_start((uint32_t) delay,        // random wait from 0 to 65535ms
                                                    TIMER_PERIODIC,TIME_MS,
                                                    cjoin_timer_cb);
   
        openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIO_OVERFLOWS,
                             (errorparameter_t)ieee154e_vars.state,
                            (errorparameter_t)delay);
    }
}

//=========================== private =========================================

owerror_t cjoin_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {


    if (msg->payload[0] == 0) {
        cjoin_vars.joined = 1;               // declare join is over
        opentimers_stop(cjoin_vars.timerId); // stop the timer
    }
    else {
        openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)msg->payload[0]);
        cjoin_sendPut(msg->payload[0]--);
    }

   return E_SUCCESS;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void cjoin_timer_cb(opentimer_id_t id){
   scheduler_push_task(cjoin_task_cb,TASKPRIO_COAP);
}

void cjoin_task_cb() {
    uint8_t temp;
  
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // don't run if no route to DAG root
   if (icmpv6rpl_getPreferredParentIndex(&temp) == FALSE) { 
        return;
   }

/*   if (icmpv6rpl_daoSent() == FALSE) {
        return;
   }
*/

   // don't run on dagroot
   if (idmanager_getIsDAGroot()) {
      opentimers_stop(cjoin_vars.timerId);
      return;
   }

   cjoin_sendPut(NUMBER_OF_EXCHANGES-1);
   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)ieee154e_vars.state,
                         (errorparameter_t)NUMBER_OF_EXCHANGES-1);

   return;
}

void cjoin_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

owerror_t cjoin_sendPut(uint8_t payload) {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
 
 // create a CoAP RD packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_CJOIN);
   if (pkt==NULL) {
      openserial_printError(
         COMPONENT_CJOIN,
         ERR_NO_FREE_PACKET_BUFFER,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
      openqueue_freePacketBuffer(pkt);
      return E_FAIL;
   }
   // take ownership over that packet
   pkt->creator                   = COMPONENT_CJOIN;
   pkt->owner                     = COMPONENT_CJOIN;
   // CoAP payload
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]             = payload;

   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0] = COAP_PAYLOAD_MARKER;
   
   // content-type option
   packetfunctions_reserveHeaderSize(pkt,2);
   pkt->payload[0]                = (COAP_OPTION_NUM_CONTENTFORMAT - COAP_OPTION_NUM_URIPATH) << 4
                                    | 1;
   pkt->payload[1]                = COAP_MEDTYPE_APPOCTETSTREAM;
   // location-path option
   packetfunctions_reserveHeaderSize(pkt,sizeof(cjoin_path0)-1);
   memcpy(&pkt->payload[0],cjoin_path0,sizeof(cjoin_path0)-1);
   packetfunctions_reserveHeaderSize(pkt,1);
   pkt->payload[0]                = ((COAP_OPTION_NUM_URIPATH) << 4) | (sizeof(cjoin_path0)-1);
   
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type    = ADDR_128B;
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],&ipAddr_jce,16);
   
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_NON,
      COAP_CODE_REQ_PUT,
      1,
      &cjoin_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
      return E_FAIL;
   }

  return E_SUCCESS;
}
