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
#define ASN_LENGTH              5

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
owerror_t cjoin_sendJoinRequest(void);
void cjoin_retransmission_cb(opentimer_id_t id);
void cjoin_retransmission_task_cb(void);
bool cjoin_getIsJoined(void);
void cjoin_setIsJoined(bool newValue);

owerror_t cjoin_parse_join_response(join_response_t *, uint8_t *, uint8_t);
owerror_t cjoin_parse_keyset(COSE_keyset_t *, uint8_t *, uint8_t *);
owerror_t cjoin_parse_short_address(short_address_t *, uint8_t *, uint8_t *);
owerror_t cjoin_parse_key(COSE_symmetric_key_t *, uint8_t *, uint8_t *);

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
    uint8_t i;
    join_response_t join_response;
    owerror_t ret;

    opentimers_stop(cjoin_vars.retransmissionTimerId); // stop the timer

    if (coap_header->Code != COAP_CODE_RESP_CONTENT) {
        return E_FAIL;        
    }

    // loop through the options and look for content format
    i = 0;
    while(coap_options[i].type != COAP_OPTION_NONE) {
        if (coap_options[i].type == COAP_OPTION_NUM_CONTENTFORMAT && 
                        *(coap_options[i].pValue) == COAP_MEDTYPE_APPCBOR) {
            ret = cjoin_parse_join_response(&join_response, msg->payload, msg->length);
            if (ret == E_FAIL) { return E_FAIL; }

             // set the internal keys as per the parsed values
            cjoin_setIsJoined(TRUE); // declare join is over
            break;
        }
        i++; 
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
    cjoin_sendJoinRequest();
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

    cjoin_sendJoinRequest();

   return;
}

void cjoin_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

owerror_t cjoin_sendJoinRequest(void) {
   OpenQueueEntry_t*    pkt;
   owerror_t            outcome;

   // immediately arm the retransmission timer
   cjoin_vars.retransmissionTimerId    = opentimers_start((uint32_t) TIMEOUT,
                                                 TIMER_ONESHOT,TIME_MS,
                                                 cjoin_retransmission_cb);

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
      COAP_TYPE_CON,
      COAP_CODE_REQ_GET,
      1,
      &desc
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
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   if (cjoin_vars.isJoined == newValue) {
        ENABLE_INTERRUPTS();
        return;
   }
   
   cjoin_vars.isJoined = newValue;

   // Update Join ASN value
   if (idmanager_getIsDAGroot() == FALSE) {
        ieee154e_getAsn(array);
        cjoin_vars.joinAsn.bytes0and1           = ((uint16_t) array[1] << 8) | ((uint16_t) array[0]);
        cjoin_vars.joinAsn.bytes2and3           = ((uint16_t) array[3] << 8) | ((uint16_t) array[2]);
        cjoin_vars.joinAsn.byte4                = array[4]; 
    } else {
        // Dag root resets the ASN value to zero
        memset(&cjoin_vars.joinAsn, 0x00, sizeof(asn_t));
    }
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

/**
\brief Parse the received join response.

This function expects the join response structure from minimal-security-02 draft.

\param[out] response The join_response_t structure containing parsed info.
\param[in] buf The received join response.
\param[in] len Length of the payload.
*/
owerror_t cjoin_parse_join_response(join_response_t *response, uint8_t *buf, uint8_t len) {

    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t ret;
    uint8_t *tmp;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    if (additional_info > 2 || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;

    if (cjoin_parse_keyset(&(response->keyset), tmp, &ret) == E_FAIL) {
        return E_FAIL;
    }

    tmp += ret;
    
    if (additional_info == 2) { // short address present
        if (cjoin_parse_short_address(&(response->short_address), tmp, &ret) == E_FAIL) {
            return E_FAIL;
        }
        tmp += ret;
    }

    if ( (uint8_t)(tmp - buf) != len) { // final check that everything has been parsed 
        memset(response, 0x00, sizeof(join_response_t)); // invalidate join response
        return E_FAIL;
    }

    return E_SUCCESS;
}

/**
\brief Parse the received COSE_Keyset.

The function expects COSE_Keyset with symmetric keys as per minimal-security-02 draft
and parses it into COSE_symmetric_key_t structure.

\param[out] keyset The COSE_keyset_t structure containing parsed keys.
\param[in] buf Input buffer.
\param[out] len Processed length.
*/
owerror_t cjoin_parse_keyset(COSE_keyset_t *keyset, uint8_t *buf, uint8_t* len) {

    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t i;
    uint8_t ret;
    uint8_t *tmp;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    if (additional_info > CJOIN_MAX_NUM_KEYS || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;

    for(i = 0; i < additional_info; i++) {
        // parse symmetric key map
        if (cjoin_parse_key(&keyset->key[i], tmp, &ret) == E_FAIL) {
            return E_FAIL;
        }
        tmp += ret;
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

/**
\brief Parse a COSE symmetric key.

The function expects COSE symmetric key as per minimal-security-02 draft
and parses it into COSE_symmetric_key_t structure.

\param[out] key The COSE_symmetric_ket_t structure containing parsed key.
\param[in] buf Input buffer.
\param[out] len Processed length.
*/
owerror_t cjoin_parse_key(COSE_symmetric_key_t *key, uint8_t* buf, uint8_t* len) {

    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t i;
    uint8_t *tmp;
    uint8_t l;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_MAP) {
        return E_FAIL;
    }

    if (additional_info > COSE_SYMKEY_MAXNUMPAIRS || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;

    for (i = 0; i < additional_info; i++) {
        switch((cose_key_label_t) *tmp) {
            case COSE_KEY_LABEL_KTY:
                tmp++;
                key->kty = (cose_key_value_t) *tmp;
                tmp++;
                break;
            case COSE_KEY_LABEL_ALG: // step by key alg
                tmp++;
                tmp++;
                break;
            case COSE_KEY_LABEL_KID:
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                key->kid = tmp;
                key->kid_len = l;
                tmp += l;
                break;
            case COSE_KEY_LABEL_K:
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                key->k = tmp;
                key->k_len = l;
                tmp += l;
                break;
            case COSE_KEY_LABEL_BASEIV:   // step by base iv
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                tmp += l;
                break;
            case COSE_KEY_LABEL_KEYOPS: // step by key ops
                tmp++;
                l = *tmp & CBOR_ADDINFO_MASK;
                tmp++;
                tmp += l;
                break;
            default:
                return E_FAIL;
        }
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

/**
\brief Parse the received short address.

The function expects short_address as per minimal-security-02 draft
and parses it into short_address_t structure.

\param[out] address The short_address_t structure containing parsed short_address and lease time.
\param[in] buf Input buffer.
\param[out] len Processed length.
*/
owerror_t cjoin_parse_short_address(short_address_t *short_address, uint8_t *buf, uint8_t* len) {
    
    cbor_majortype_t major_type;
    uint8_t additional_info;
    uint8_t *tmp;
    uint8_t l;

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    additional_info = *buf & CBOR_ADDINFO_MASK;
        
    if (major_type != CBOR_MAJORTYPE_ARRAY) {
        return E_FAIL;
    }

    if (additional_info > 2 || additional_info == 0) {
        return E_FAIL;  // unsupported join response structure
    }

    tmp++;
    l = *tmp & CBOR_ADDINFO_MASK;
    
    if (l != CJOIN_SHORT_ADDRESS_LENGTH) {
        return E_FAIL;
    }

    tmp++;
    short_address->address = tmp;

    tmp += l;

    if (additional_info == 2) { // lease time present
        l = *tmp & CBOR_ADDINFO_MASK;
        if (l != ASN_LENGTH) { // 5 byte ASN expected
            return E_FAIL;
        }
        tmp++;

        (short_address->lease_asn).bytes0and1           = ((uint16_t) tmp[1] << 8) | ((uint16_t) tmp[0]);
        (short_address->lease_asn).bytes2and3           = ((uint16_t) tmp[3] << 8) | ((uint16_t) tmp[2]);
        (short_address->lease_asn).byte4                = tmp[4]; 

        tmp += ASN_LENGTH;
    }

    *len = (uint8_t) (tmp - buf);
    return E_SUCCESS;
}

