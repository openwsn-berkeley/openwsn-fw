/**
\brief A CoAP resource accepting ACE access tokens.
*/

#include "opendefs.h"
#include "cauthz.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "openrandom.h"
#include "board.h"
#include "idmanager.h"
#include "eui64.h"
#include "cbor.h"
#include "cryptoengine.h"
#include "openoscoap.h"

//=========================== defines =========================================

const uint8_t cauthz_path0[] = "authz-info";

//=========================== variables =======================================

cauthz_vars_t cauthz_vars;

//=========================== prototypes ======================================

owerror_t cauthz_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
);
void cauthz_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);

owerror_t cauthz_cbor_decode_access_token(uint8_t *buf, uint8_t len, cauthz_access_token_t* token);

//=========================== public ==========================================

/**
\brief Initialize this module.
*/
void cauthz_init(void) {
    uint8_t myId[8];
    uint8_t *joinKey;

   // do not run if DAGroot
   if(idmanager_getIsDAGroot()==TRUE) return;

   // prepare the resource descriptor for the /i path
   cauthz_vars.desc.path0len             = sizeof(cauthz_path0)-1;
   cauthz_vars.desc.path0val             = (uint8_t*)(&cauthz_path0);
   cauthz_vars.desc.path1len             = 0;
   cauthz_vars.desc.path1val             = NULL;
   cauthz_vars.desc.componentID          = COMPONENT_CAUTHZ;
   cauthz_vars.desc.securityContext      = NULL;
   cauthz_vars.desc.discoverable         = TRUE;
   cauthz_vars.desc.callbackRx           = &cauthz_receive;
   cauthz_vars.desc.callbackSendDone     = &cauthz_sendDone;

   memset(cauthz_vars.accessToken, 0x00, sizeof(cauthz_access_token_t));

   eui64_get(myId);
   idmanager_getJoinKey(&joinKey);

   // derive appKey
   openoscoap_hkdf_derive_parameter(cauthz_vars.appKey,
           joinKey,
           16,
           NULL,
           0,
           myId,
           8,
           AES_CCM_16_64_128,
           OSCOAP_DERIVATION_TYPE_ACE,
           AES_CCM_16_64_128_KEY_LEN);

   // register with the CoAP module
   opencoap_register(&cauthz_vars.desc);
}

//=========================== private =========================================

/**
\brief Called when a CoAP message is received for this resource.

\param[in] msg          The received message. CoAP header and options already
   parsed.
\param[in] coap_header  The CoAP header contained in the message.
\param[in] coap_options The CoAP options contained in the message.

\return Whether the response is prepared successfully.
*/
owerror_t cauthz_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen
) {
   owerror_t outcome;

   switch (coap_header->Code) {
      case COAP_CODE_REQ_POST:
            // fail on outcome returns METHODNOTALLOWED. we don't want that
            // rather, we want to handle errors explicitly in the app code
            // we return E_SUCCESS in all cases, sometimes returning errors as needed
            outcome                          = E_SUCCESS;

         // === decode and decrypt access token presented in the payload
         if (cauthz_cbor_decode_access_token(msg->payload, msg->length, &cauthz_vars.accessToken[0]) == E_SUCCESS) {

             // invoke opencoap to install the context for a given resource
             opencoap_set_resource_security_context(cauthz_vars.accessToken[0].path0len,
                     cauthz_vars.accessToken[0].path0val,
                     cauthz_vars.accessToken[0].path1len,
                     cauthz_vars.accessToken[0].path1val,
                     &cauthz_vars.accessToken[0].context);

            //=== reset packet payload (we will reuse this packetBuffer)
            msg->payload                     = &(msg->packet[127]);
            msg->length                      = 0;

            //=== prepare  CoAP response

            // set the CoAP header
            coap_header->Code                = COAP_CODE_RESP_CREATED;
         } else {
            //=== reset packet payload (we will reuse this packetBuffer)
            msg->payload                     = &(msg->packet[127]);
            msg->length                      = 0;

            // set the CoAP header
            coap_header->Code                = COAP_CODE_RESP_BADREQ;
         }
         break;
      default:
         // return an error message
         outcome = E_FAIL;
   }

   return outcome;
}

/**
\brief The stack indicates that the packet was sent.

\param[in] msg The CoAP message just sent.
\param[in] error The outcome of sending it.
*/
void cauthz_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openqueue_freePacketBuffer(msg);
}

owerror_t cauthz_cbor_decode_access_token(uint8_t *buf, uint8_t len, cauthz_access_token_t* token) {
    owerror_t decStatus;
    cbor_majortype_t major_type;
    uint8_t add_info;
    uint8_t i;
    uint8_t *tmp;
    uint8_t ciphertextLen;
    uint8_t *ciphertext;
    uint8_t nonce[13];
    // CBOR serialization of ["Encrypt0", h'', h'']
    const uint8_t aad[] = {0x83, 0x68, 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x30, 0x40, 0x40};

    i = 0;

    memset(nonce, 0x00, 13);

    // expecting COSE_Encrypt0 array
    // [ protected, unprotected, ciphertext ]

    tmp = buf;
    major_type = (cbor_majortype_t) *buf >> 5;
    add_info = *buf & CBOR_ADDINFO_MASK;

    // assert
    if (major_type != CBOR_MAJORTYPE_ARRAY) {  // not COSE_Encrypt0
        return E_FAIL;
    }

    // sanity check
    if (add_info != 3) {   // not COSE_Encrypt0
        return E_FAIL;
    }

    tmp++;

    // Step 1. Decode protected bucket, expecting an empty string
    if (*tmp != 0x40) { // protected bucket is not empty
        return E_FAIL;
    }

    // Step 2. Moving on to the unprotected bucket
    tmp++;

    if (*tmp != 0xa1) {
        return E_FAIL; // expecting a map with a single element: PIV
    }

    // expected label is PIV
    tmp++;
    if (*tmp != COSE_COMMON_HEADER_PARAMETERS_PIV) {
        return E_FAIL;
    }

    // expected value of PIV is an uint8_t
    tmp++;
    if (cbor_load_uint(tmp, &nonce[12]) != 1) {
        return E_FAIL;
    }

    tmp++;

    // Step 3. Ciphertext
    major_type = (cbor_majortype_t) *tmp >> 5;
    add_info = *tmp & CBOR_ADDINFO_MASK;

    if (major_type != CBOR_MAJORTYPE_BSTR) {
        return E_FAIL;
    }

    if (add_info < AES_CCM_16_64_128_TAG_LEN) {
        return E_FAIL;
    }

    if (add_info <= 23) {
        ciphertextLen = add_info;
    } else if (add_info == 24) {
        tmp++;
        ciphertextLen = *tmp;
    } else {
        return E_FAIL;
    }
    ciphertext = ++tmp;
    tmp += ciphertextLen;

    if (tmp - buf != len) {
        return E_FAIL;
    }

    decStatus = cryptoengine_aes_ccms_dec( (uint8_t *) aad,
                                    sizeof(aad),
                                    ciphertext,
                                    &ciphertextLen,
                                    nonce,
                                    2,
                                    cauthz_vars.appKey,
                                    AES_CCM_16_64_128_TAG_LEN);

    if (decStatus == E_SUCCESS) {
        return E_SUCCESS;
    }

    return E_FAIL;
}

