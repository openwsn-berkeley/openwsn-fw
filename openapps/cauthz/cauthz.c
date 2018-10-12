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
const uint8_t cauthz_default_scope[] = "resource1";

//=========================== variables =======================================

cauthz_vars_t cauthz_vars;

//=========================== prototypes ======================================

owerror_t cauthz_receive(
        OpenQueueEntry_t* msg,
        coap_header_iht*  coap_header,
        coap_option_iht*  coap_incomingOptions,
        coap_option_iht*  coap_outgoingOptions,
        uint8_t*          coap_outgoingOptionsLen,
        bool              security
);
void cauthz_sendDone(
   OpenQueueEntry_t* msg,
   owerror_t error
);

owerror_t cauthz_cbor_decode_access_token(uint8_t* buf, uint8_t len, cauthz_oscore_cwt_t* token);
owerror_t cauthz_cbor_decode_cwt(uint8_t* ciphertext, uint8_t ciphertextLen, cauthz_oscore_cwt_t* token);

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

   memset(&cauthz_vars.accessToken, 0x00, sizeof(cauthz_oscore_cwt_t));

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
        uint8_t*          coap_outgoingOptionsLen,
        bool              security
) {
   owerror_t outcome;

   switch (coap_header->Code) {
      case COAP_CODE_REQ_POST:
            // fail on outcome returns METHODNOTALLOWED. we don't want that
            // rather, we want to handle errors explicitly in the app code
            // we return E_SUCCESS in all cases, sometimes returning errors as needed
            outcome                          = E_SUCCESS;

         // === decode and decrypt access token presented in the payload
         if (cauthz_cbor_decode_access_token(msg->payload, msg->length, &cauthz_vars.accessToken) == E_SUCCESS) {

             if (cauthz_vars.accessToken.path0len == 0 && cauthz_vars.accessToken.path1len == 0) {
                 // invoke opencoap to install the context for the implied scopee
                 opencoap_set_resource_security_context(
                         sizeof(cauthz_default_scope)-1,
                         (uint8_t*)(&cauthz_default_scope),
                         0,
                         NULL,
                         &cauthz_vars.accessToken.context
                         );

             } else {
                 // invoke opencoap to install the context for the explicitly signaled resource
                 opencoap_set_resource_security_context(
                         cauthz_vars.accessToken.path0len,
                         cauthz_vars.accessToken.path0val,
                         cauthz_vars.accessToken.path1len,
                         cauthz_vars.accessToken.path1val,
                         &cauthz_vars.accessToken.context
                         );
            }

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

owerror_t cauthz_cbor_decode_access_token(uint8_t *buf, uint8_t len, cauthz_oscore_cwt_t* token) {
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
        return cauthz_cbor_decode_cwt(ciphertext, ciphertextLen, token);
    }

    return E_FAIL;
}

owerror_t cauthz_cbor_decode_cwt(uint8_t* buf, uint8_t len, cauthz_oscore_cwt_t* token) {
    cbor_majortype_t major_type;
    uint8_t add_info;
    uint8_t i;
    uint8_t *tmp;
    uint8_t label;
    uint8_t num_elems;
    uint8_t master_secret[AES128_KEY_LENGTH];
    uint8_t master_secret_len;
    uint8_t master_salt[AES128_KEY_LENGTH];
    uint8_t master_salt_len;
    uint8_t client_id[OSCOAP_MAX_ID_LEN];
    uint8_t client_id_len;
    uint8_t server_id[OSCOAP_MAX_ID_LEN];
    uint8_t server_id_len;

    i = 0;
    label = 0;
    tmp = buf;
    master_secret_len = 0;
    master_salt_len = 0;
    client_id_len = 0;
    server_id_len = 0;

    // assert
    if (*tmp != 0xa1) {  // not a map with a single element (cnf)
        return E_FAIL;
    }

    tmp++;

    // expecting a uint
    tmp += cbor_load_uint(tmp, &label);

    if (label != ACE_PARAMETERS_LABELS_CNF) {
        return E_FAIL;
    }

    // assert
    if (*tmp != 0xa1) {  // not a map with a single element (COSE_Key)
        return E_FAIL;  // TODO extend to support scope parameter
    }

    tmp++;

    tmp += cbor_load_uint(tmp, &label);

    if (label != ACE_CWT_CNF_COSE_KEY) {
        return E_FAIL;
    }

    // now, we are actually at the useful part

    major_type = (cbor_majortype_t) *tmp >> 5;
    num_elems = *tmp & CBOR_ADDINFO_MASK;

    if (major_type != CBOR_MAJORTYPE_MAP && num_elems >= 23) {
        return E_FAIL;
    }

   tmp++;
   for (i = 0; i < num_elems; i++) {
        switch((cose_key_parameters_labels_t) *tmp) {
            case COSE_KEY_LABEL_KTY:
                tmp++;
                if (*tmp != COSE_KEY_VALUE_SYMMETRIC) {
                    // unsupported, fail immediately
                    return E_FAIL;
                }
                tmp++;
                break;
            case COSE_KEY_LABEL_K:  // OSCORE Master Secret
                tmp++;
                major_type = (cbor_majortype_t) *tmp >> 5;
                add_info = *tmp & CBOR_ADDINFO_MASK;
                if (major_type != CBOR_MAJORTYPE_BSTR && add_info > AES128_KEY_LENGTH) {
                    return E_FAIL;
                }
                tmp++;
                memcpy(master_secret, tmp, add_info);
                master_secret_len = add_info;
                tmp += add_info;
                break;
            case COSE_KEY_LABEL_SLT:  // OSCORE Master Salt
                tmp++;
                major_type = (cbor_majortype_t) *tmp >> 5;
                add_info = *tmp & CBOR_ADDINFO_MASK;
                if (major_type != CBOR_MAJORTYPE_BSTR && add_info > AES128_KEY_LENGTH) {
                    return E_FAIL;
                }
                tmp++;
                memcpy(master_secret, tmp, add_info);
                master_salt_len = add_info;
                tmp += add_info;
                break;
            case COSE_KEY_LABEL_CLIENT_ID:
                tmp++;
                major_type = (cbor_majortype_t) *tmp >> 5;
                add_info = *tmp & CBOR_ADDINFO_MASK;
                if (major_type != CBOR_MAJORTYPE_BSTR && add_info > OSCOAP_MAX_ID_LEN) {
                    return E_FAIL;
                }
                tmp++;
                memcpy(client_id, tmp, add_info);
                client_id_len = add_info;
                tmp += add_info;
                break;
            case COSE_KEY_LABEL_SERVER_ID:
                tmp++;
                major_type = (cbor_majortype_t) *tmp >> 5;
                add_info = *tmp & CBOR_ADDINFO_MASK;
                if (major_type != CBOR_MAJORTYPE_BSTR && add_info > OSCOAP_MAX_ID_LEN) {
                    return E_FAIL;
                }
                tmp++;
                memcpy(server_id, tmp, add_info);
                server_id_len = add_info;
                tmp += add_info;
                break;
            default:
                // reject any unsupported parameter
                return E_FAIL;
        }
    }

    openoscoap_init_security_context(&token->context,
            server_id,
            server_id_len,
            client_id,
            client_id_len,
            master_secret,
            master_secret_len,
            master_salt,
            master_salt_len);

    token->path0len = 0;
    token->path1len = 0;

    return E_SUCCESS;
}

