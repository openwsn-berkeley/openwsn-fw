/**
\brief CoAP application implementing Simple Join Protocol from minimal-security-02 draft.
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
#include "idmanager.h"
#include "IEEE802154E.h"
#include "IEEE802154_security.h"
#include "cbor.h"
#include "eui64.h"
#include "neighbors.h"

//=========================== defines =========================================

/// inter-packet period (in ms)
#define TIMEOUT                 60000

const uint8_t cjoin_path0[] = "j";

static const uint8_t masterSecret[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, \
                                     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const uint8_t jrcHostName[] = "6tisch.arpa";

static const uint8_t proxyScheme[] = "coap";

//=========================== variables =======================================

cjoin_vars_t cjoin_vars;

//=========================== prototypes ======================================
void        cjoin_init_security_context();

owerror_t   cjoin_receive(OpenQueueEntry_t* msg,
                          coap_header_iht*  coap_header,
                          coap_option_iht*  coap_incomingOptions,
                          coap_option_iht*  coap_outgoingOptions,
                          uint8_t*          coap_outgoingOptionsLen);
void        cjoin_timer_cb(opentimers_id_t id);
void        cjoin_task_cb(void);
void        cjoin_sendDone(OpenQueueEntry_t* msg,
                       owerror_t error);
owerror_t   cjoin_sendJoinRequest(open_addr_t *joinProxy);
void        cjoin_retransmission_cb(opentimers_id_t id);
void        cjoin_retransmission_task_cb(void);
bool        cjoin_getIsJoined(void);
void        cjoin_setIsJoined(bool newValue);
//=========================== public ==========================================

void cjoin_init() {
   // prepare the resource descriptor for the /j path
   cjoin_vars.desc.path0len                        = sizeof(cjoin_path0)-1;
   cjoin_vars.desc.path0val                        = (uint8_t*)(&cjoin_path0);
   cjoin_vars.desc.path1len                        = 0;
   cjoin_vars.desc.path1val                        = NULL;
   cjoin_vars.desc.componentID                     = COMPONENT_CJOIN;
   cjoin_vars.desc.securityContext                 = &cjoin_vars.context;
   cjoin_vars.desc.discoverable                    = TRUE;
   cjoin_vars.desc.callbackRx                      = &cjoin_receive;
   cjoin_vars.desc.callbackSendDone                = &cjoin_sendDone;
  
   cjoin_vars.isJoined                             = FALSE;   

   opencoap_register(&cjoin_vars.desc);

   cjoin_vars.timerId = opentimers_create();

   idmanager_setJoinKey((uint8_t *) masterSecret);

   cjoin_schedule();
}

void cjoin_init_security_context() {
   uint8_t senderID[9];     // needs to hold EUI-64 + 1 byte
   uint8_t recipientID[9];  // needs to hold EUI-64 + 1 byte
   uint8_t* joinKey;
   eui64_get(senderID);
   senderID[8] = 0x00;      // construct sender ID according to the minimal-security-03 draft
   eui64_get(recipientID);
   recipientID[8] = 0x01; // construct recipient ID according to the minimal-security-03 draft

   idmanager_getJoinKey(&joinKey);
   
   openoscoap_init_security_context(&cjoin_vars.context, 
                                senderID, 
                                sizeof(senderID),
                                recipientID,
                                sizeof(recipientID),
                                joinKey,
                                16,
                                NULL,
                                0);
}

void cjoin_schedule() {
    uint16_t delay;
    
    if (cjoin_getIsJoined() == FALSE) {
        delay = openrandom_get16b();
        
        opentimers_scheduleIn(cjoin_vars.timerId,
                (uint32_t) delay, // random wait from 0 to 65535ms
                TIME_MS, 
                TIMER_PERIODIC,
                cjoin_timer_cb
        );
    }

}

//=========================== private =========================================
owerror_t cjoin_receive(OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen) {

    join_response_t join_response;
    owerror_t ret;

    opentimers_cancel(cjoin_vars.timerId); // cancel the retransmission timer

    if (coap_header->Code != COAP_CODE_RESP_CONTENT) {
        return E_FAIL;        
    }

    ret = cbor_parse_join_response(&join_response, msg->payload, msg->length);
    if (ret == E_FAIL) { return E_FAIL; }

    // set the L2 keys as per the parsed value
    IEEE802154_security_setBeaconKey(join_response.keyset.key[0].kid[0], join_response.keyset.key[0].k);
    IEEE802154_security_setDataKey(join_response.keyset.key[0].kid[0], join_response.keyset.key[0].k);

    cjoin_setIsJoined(TRUE); // declare join is over

    return E_SUCCESS;
}

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void cjoin_timer_cb(opentimers_id_t id){
   scheduler_push_task(cjoin_task_cb,TASKPRIO_COAP);
}

void cjoin_retransmission_cb(opentimers_id_t id) {
    scheduler_push_task(cjoin_retransmission_task_cb, TASKPRIO_COAP);
}

void cjoin_retransmission_task_cb() {
    open_addr_t* joinProxy;

    joinProxy = neighbors_getJoinProxy();
    if(joinProxy == NULL) {
      openserial_printError(
         COMPONENT_CJOIN,
         ERR_ABORT_JOIN_PROCESS,
         (errorparameter_t)0,
         (errorparameter_t)0
      );
        return;
    }
 
    cjoin_sendJoinRequest(joinProxy);
}

void cjoin_task_cb() {
    open_addr_t *joinProxy;
  
    // don't run if not synch
    if (ieee154e_isSynch() == FALSE) return;

    // don't run if DAG root
    if (idmanager_getIsDAGroot() == TRUE) {
        opentimers_destroy(cjoin_vars.timerId);
        return;
    }

    joinProxy = neighbors_getJoinProxy();
    if(joinProxy == NULL) {
        return;
    }
    
    // cancel the startup timer but do not destroy it as we reuse it for retransmissions
    opentimers_cancel(cjoin_vars.timerId);

    // init the security context only here in order to use the latest joinKey
    // that may be set over the serial
    cjoin_init_security_context();

    cjoin_sendJoinRequest(joinProxy);

    return;
}

void cjoin_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

owerror_t cjoin_sendJoinRequest(open_addr_t* joinProxy) {
   OpenQueueEntry_t*    pkt;
   open_addr_t*         prefix;
   owerror_t            outcome;
   coap_option_iht      options[5];

    // immediately arm the retransmission timer
    opentimers_scheduleIn(cjoin_vars.timerId,
            (uint32_t) TIMEOUT,
            TIME_MS, 
            TIMER_ONESHOT,
            cjoin_retransmission_cb
    );

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

   // uri-host set to 6tisch.arpa
   options[0].type = COAP_OPTION_NUM_URIHOST;
   options[0].length = sizeof(jrcHostName)-1;
   options[0].pValue = (uint8_t *)jrcHostName;
   
   // location-path option
   options[1].type = COAP_OPTION_NUM_URIPATH;
   options[1].length = sizeof(cjoin_path0)-1;
   options[1].pValue = (uint8_t *)cjoin_path0;

   // object security option
   // length and value are set by the CoAP library
   options[2].type = COAP_OPTION_NUM_OBJECTSECURITY;

   // ProxyScheme set to "coap"
   options[3].type = COAP_OPTION_NUM_PROXYSCHEME;
   options[3].length = sizeof(proxyScheme)-1;
   options[3].pValue = (uint8_t *)proxyScheme;
 
   // metadata
   pkt->l4_destination_port       = WKP_UDP_COAP;
   pkt->l3_destinationAdd.type    = ADDR_128B;
   prefix = idmanager_getMyID(ADDR_PREFIX); // at this point, this is link-local prefix
   memcpy(&pkt->l3_destinationAdd.addr_128b[0],prefix->prefix,8);
   memcpy(&pkt->l3_destinationAdd.addr_128b[8],joinProxy->addr_64b,8); // set host to eui-64 of the join proxy
 
   // send
   outcome = opencoap_send(
      pkt,
      COAP_TYPE_CON,
      COAP_CODE_REQ_GET,
      0, // token len
      options,
      4, // options len
      &cjoin_vars.desc
   );
   
   // avoid overflowing the queue if fails
   if (outcome==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
      return E_FAIL;
   }

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
    uint8_t array[5];
    asn_t joinAsn;

    if (cjoin_vars.isJoined == newValue) {
        return;
    }

    cjoin_vars.isJoined = newValue;

    // Update Join ASN value
    ieee154e_getAsn(array);
    joinAsn.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
    joinAsn.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
    joinAsn.byte4                = array[4];

    idmanager_setJoinAsn(&joinAsn);

    if (newValue == TRUE) {
        // log the info
        openserial_printInfo(COMPONENT_CJOIN, ERR_JOINED,
                             (errorparameter_t)0,
                             (errorparameter_t)0);
    }
}

