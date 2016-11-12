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
#define TIMEOUT                 60000

const uint8_t cjoin_path0[] = "j";

static const uint8_t ipAddr_jce[] = {0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

coap_resource_desc_t desc;
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
void cjoin_retransmission_cb(opentimer_id_t id);
void cjoin_retransmission_task_cb(void);
bool cjoin_getIsJoined(void);
void cjoin_setIsJoined(bool newValue);
//=========================== public ==========================================

void cjoin_init() {
   
   // prepare the resource descriptor for the /j path
   desc.path0len                        = sizeof(cjoin_path0)-1;
   desc.path0val                        = (uint8_t*)(&cjoin_path0);
   desc.path1len                        = 0;
   desc.path1val                        = NULL;
   desc.componentID                     = COMPONENT_CJOIN;
   desc.discoverable                    = TRUE;
   desc.callbackRx                      = &cjoin_receive;
   desc.callbackSendDone                = &cjoin_sendDone;
   cjoin_vars.lastPayload               = NUMBER_OF_EXCHANGES - 1;
   cjoin_vars.isJoined                  = FALSE;   

   memset(&cjoin_vars.joinAsn, 0x00, sizeof(asn_t));

   opencoap_register(&desc);

   cjoin_schedule();
}

void cjoin_schedule() {
    uint16_t delay;
    
    if (cjoin_getIsJoined() == FALSE) {
        delay = openrandom_get16b();
        cjoin_vars.startupTimerId    = opentimers_start((uint32_t) delay,        // random wait from 0 to 65535ms
                                                    TIMER_PERIODIC,TIME_MS,
                                                    cjoin_timer_cb);
    }
}

//=========================== private =========================================

owerror_t cjoin_receive(OpenQueueEntry_t* msg,
                      coap_header_iht* coap_header,
                      coap_option_iht* coap_options) {

    uint8_t asn[5];
    uint32_t asnCropped;


        opentimers_stop(cjoin_vars.retransmissionTimerId); // stop the timer

        openserial_printError(COMPONENT_IEEE802154E,ERR_WDRADIO_OVERFLOWS,
                             (errorparameter_t)0,
                             (errorparameter_t)msg->payload[0]);
 
        if (msg->payload[0] == 0) {
            cjoin_setIsJoined(TRUE);                  // declare join is over
            ieee154e_getAsn(asn);
            asnCropped = ((uint32_t) asn[3] << 24) | ((uint32_t) asn[2] << 16) | ((uint32_t) asn[1] << 8) | ((uint32_t) asn[0]);
            printf("JOIN ASN: %u\n", asnCropped);
        }
        else if (msg->payload[0] == cjoin_vars.lastPayload - 1) {
            cjoin_sendPut(msg->payload[0] - 1);
        }
        else {
            openserial_printError(COMPONENT_IEEE802154E,ERR_WDDATADURATION_OVERFLOWS,
                                     (errorparameter_t)0,
                                     (errorparameter_t)0);
        }
    
    return E_SUCCESS;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void cjoin_timer_cb(opentimer_id_t id){
   scheduler_push_task(cjoin_task_cb,TASKPRIO_COAP);
}

void cjoin_retransmission_cb(opentimer_id_t id) {

    scheduler_push_task(cjoin_retransmission_task_cb, TASKPRIO_COAP);
}

void cjoin_retransmission_task_cb() {
   cjoin_sendPut(cjoin_vars.lastPayload);
}

void cjoin_task_cb() {
    uint8_t temp;
  
   // don't run if not synch
   if (ieee154e_isSynch() == FALSE) return;

   // don't run if DAG root
   if (idmanager_getIsDAGroot() == TRUE) {
        return;
   }

   // don't run if no route to DAG root
   if (icmpv6rpl_getPreferredParentIndex(&temp) == FALSE) { 
        return;
   }

   if (icmpv6rpl_daoSent() == FALSE) {
        return;
   }
    opentimers_stop(cjoin_vars.startupTimerId);

    cjoin_sendPut(NUMBER_OF_EXCHANGES-1);

   return;
}

void cjoin_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

owerror_t cjoin_sendPut(uint8_t payload) {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;
 
   cjoin_vars.lastPayload = payload;

   openserial_printError(COMPONENT_IEEE802154E,ERR_MAXRXACKPREPARE_OVERFLOWS,
                         (errorparameter_t)0,
                         (errorparameter_t)payload);
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
      &desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
      return E_FAIL;
   }

   cjoin_vars.retransmissionTimerId    = opentimers_start((uint32_t) TIMEOUT,
                                                 TIMER_ONESHOT,TIME_MS,
                                                 cjoin_retransmission_cb);
  return E_SUCCESS;
}

bool cjoin_getIsJoined() {   
   bool res;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   res=cjoin_vars.isJoined;
   ENABLE_INTERRUPTS();

   return res;
}

void cjoin_setIsJoined(bool newValue) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   
   cjoin_vars.isJoined = newValue;

   ENABLE_INTERRUPTS();

   if (newValue == TRUE) {
        // log the info
        openserial_printInfo(COMPONENT_CJOIN, ERR_JOINED,
                             (errorparameter_t)0,
                             (errorparameter_t)0);
   }
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_joined() {
   asn_t output;
   output.byte4         =  cjoin_vars.joinAsn.byte4;
   output.bytes2and3    =  cjoin_vars.joinAsn.bytes2and3;
   output.bytes0and1    =  cjoin_vars.joinAsn.bytes0and1;
   openserial_printStatus(STATUS_JOINED,(uint8_t*)&output,sizeof(output));
   return TRUE;
}
