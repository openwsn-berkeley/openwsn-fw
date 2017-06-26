#include "opendefs.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opencoap.h"
#include "cborencoder.h"
#include "cryptoengine.h"
#include "sha.h"


//=========================== defines =========================================

#define EAAD_MAX_LEN            9 + OSCOAP_MAX_ID_LEN // assumes no Class I options
#define AAD_MAX_LEN            12 + EAAD_MAX_LEN

//=========================== variables =======================================

openoscoap_vars_t openoscoap_vars;

//=========================== prototype =======================================
owerror_t hkdf_derive_parameter(uint8_t* buffer,
        uint8_t* masterSecret, 
        uint8_t masterSecretLen, 
        uint8_t* masterSalt,
        uint8_t masterSaltLen,
        uint8_t* identifier,
        uint8_t identifierLen,
        uint8_t algorithm,
        oscoap_derivation_t type,
        uint8_t length
        );

bool is_request(uint8_t code);

uint8_t construct_aad(uint8_t* buffer, 
        uint8_t version, 
        uint8_t code,
        uint8_t* optionsSerialized,
        uint8_t optionsSerializedLen,
        uint8_t aeadAlgorithm,
        uint8_t* requestKid,
        uint8_t requestKidLen,
        uint8_t* requestSeq,
        uint8_t requestSeqLen
        );

void openoscoap_encode_compressed_COSE(OpenQueueEntry_t* msg, 
        uint8_t* requestSeq, uint8_t requestSeqLen, 
        uint8_t* requestKid, 
        uint8_t requestKidLen);

void xor_arrays(uint8_t* s1, uint8_t* s2, uint8_t* dst, uint8_t len);

void flip_first_bit(uint8_t* source, uint8_t* dst, uint8_t len);

//=========================== public ==========================================


/**
\brief Initialize OSCOAP security context.

This function will derive the parameters needed to initialize OSCOAP
security context.

\param[out] ctx OSCOAP security context structure.
\param[in] senderID Pointer to the Byte array containing Sender ID.
\param[in] senderIDLen Length of the Sender ID byte array in bytes.
\param[in] recipientID Pointer to the Byte array contaning Recipient ID.
\param[in] recipientIDLen Length of the Recipient ID byte array in bytes.
\param[in] Pointer to the byte array contaning the Master Secret.
\param[in] Length of the Master Secret byte array in bytes.
\param[in] Pointer to the byte array contaning Master Salt.
\param[in] Length of the Master Salt byte array in bytes.
*/

void openoscoap_init_security_context(oscoap_security_context_t *ctx, 
                                uint8_t* senderID, 
                                uint8_t senderIDLen,
                                uint8_t* recipientID,
                                uint8_t recipientIDLen,
                                uint8_t* masterSecret,
                                uint8_t masterSecretLen,
                                uint8_t* masterSalt,
                                uint8_t masterSaltLen) {
    
    if (senderIDLen > OSCOAP_MAX_ID_LEN || 
            recipientIDLen > OSCOAP_MAX_ID_LEN) {
        return;
    }

    // common context
    ctx->aeadAlgorithm = AES_CCM_16_64_128; 

    // sender context
    memcpy(ctx->senderID, senderID, senderIDLen);
    ctx->senderIDLen = senderIDLen;
    // invoke HKDF to get sender Key
    hkdf_derive_parameter(ctx->senderKey,
            masterSecret, 
            masterSecretLen,
            masterSalt, 
            masterSaltLen,
            senderID,
            senderIDLen,
            AES_CCM_16_64_128,
            OSCOAP_DERIVATION_TYPE_KEY,
            AES_CCM_16_64_128_KEY_LEN);
    // invoke HKDF to get sender IV
    hkdf_derive_parameter(ctx->senderIV,
            masterSecret, 
            masterSecretLen,
            masterSalt, 
            masterSaltLen,
            senderID,
            senderIDLen,
            AES_CCM_16_64_128,
            OSCOAP_DERIVATION_TYPE_IV,
            AES_CCM_16_64_128_IV_LEN);
   
    ctx->sequenceNumber = 0;

    // recipient context
    memcpy(ctx->recipientID, recipientID, recipientIDLen);
    ctx->recipientIDLen = recipientIDLen;
    // invoke HKDF to get recipient Key
    hkdf_derive_parameter(ctx->recipientKey,
            masterSecret, 
            masterSecretLen,
            masterSalt, 
            masterSaltLen,
            recipientID,
            recipientIDLen,
            AES_CCM_16_64_128,
            OSCOAP_DERIVATION_TYPE_KEY,
            AES_CCM_16_64_128_KEY_LEN);

    // invoke HKDF to get recipient IV
    hkdf_derive_parameter(ctx->recipientIV,
            masterSecret, 
            masterSecretLen,
            masterSalt, 
            masterSaltLen,
            recipientID,
            recipientIDLen,
            AES_CCM_16_64_128,
            OSCOAP_DERIVATION_TYPE_IV,
            AES_CCM_16_64_128_IV_LEN);
 
    ctx->window.bitArray = 0;
    ctx->window.base = 0;

}

owerror_t openoscoap_protect_message(
        oscoap_security_context_t *context, 
        uint8_t version, 
        uint8_t code,
        coap_option_iht* options,
        uint8_t optionsLen,
        OpenQueueEntry_t* msg,
        uint16_t sequenceNumber) {

    uint8_t* payload;
    uint8_t payloadLen;
    uint8_t aad[AAD_MAX_LEN];
    uint8_t aadLen;
    uint8_t nonce[AES_CCM_16_64_128_IV_LEN];
    uint8_t partialIV[AES_CCM_16_64_128_IV_LEN];
    uint8_t* requestSeq;
    uint8_t requestSeqLen;
    uint8_t *requestKid;
    uint8_t requestKidLen;
    owerror_t encStatus;
    coap_option_iht* objectSecurity;
    bool payloadPresent;

    // find object security option in the list of passed options
    objectSecurity = opencoap_find_object_security_option(options, optionsLen);
    if (objectSecurity == NULL) { // objectSecurity option should be set by the application
        return E_FAIL;
    }

    // convert sequence number to array and strip leading zeros
    memset(partialIV, 0x00, AES_CCM_16_64_128_IV_LEN);
    packetfunctions_htons(sequenceNumber, &partialIV[AES_CCM_16_64_128_IV_LEN - 2]);
    if (partialIV[AES_CCM_16_64_128_IV_LEN - 2] == 0x00) {
        requestSeq = &partialIV[AES_CCM_16_64_128_IV_LEN - 1];
        requestSeqLen = 1;
    }
    else {
        requestSeq = &partialIV[AES_CCM_16_64_128_IV_LEN - 2];
        requestSeqLen = 2;
    }

    requestKid = context->senderID;
    requestKidLen = context->senderIDLen;

    if (msg->length > 0 ) { // contains payload, add payload marker
        payloadPresent = TRUE;
        packetfunctions_reserveHeaderSize(msg,1);
        msg->payload[0] = COAP_PAYLOAD_MARKER;
    }
    else {
        payloadPresent = FALSE;
    }

    // encode the options to the openqueue payload buffer
    opencoap_options_encode(msg, 
            options, 
            optionsLen, 
            COAP_OPTION_CLASS_E);

    payload = &msg->payload[0];
    payloadLen = msg->length;
    // shift payload to leave space for authentication tag
    packetfunctions_reserveHeaderSize(msg, AES_CCM_16_64_128_TAG_LEN);
    memcpy(&msg->payload[0], payload, payloadLen);
    // update payload pointer but leave length intact
    payload = &msg->payload[0];

    aadLen = construct_aad(aad,
            version,
            code,
            NULL,
            0, // do not support Class I options at the moment
            AES_CCM_16_64_128,
            requestKid,
            requestKidLen,
            requestSeq,
            requestSeqLen);

    if (aadLen > AAD_MAX_LEN) {
        // corruption
        openserial_printError(
                COMPONENT_OPENOSCOAP,ERR_BUFFER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)0
        );
        return E_FAIL;
    }
    
    // construct nonce 
    if (is_request(code)) {
        xor_arrays(context->senderIV, partialIV, nonce, AES_CCM_16_64_128_IV_LEN); 
    }
    else {
        flip_first_bit(context->senderIV, nonce, AES_CCM_16_64_128_IV_LEN);
        xor_arrays(nonce, partialIV, nonce, AES_CCM_16_64_128_IV_LEN);
        // do not encode sequence number and ID in the response
        requestSeq = NULL;
        requestSeqLen = 0;
        requestKid = NULL;
        requestKidLen = 0;
    }

     encStatus = cryptoengine_aes_ccms_enc(aad,
                                        aadLen,
                                        payload,
                                        &payloadLen,
                                        nonce,
                                        2, // L=2 in 15.4 std
                                        context->senderKey,
                                        AES_CCM_16_64_128_TAG_LEN);

     if (encStatus != E_SUCCESS) {
        return E_FAIL;
     }

     // encode compressed COSE
     openoscoap_encode_compressed_COSE(msg, requestSeq, requestSeqLen, requestKid, requestKidLen);


    if (payloadPresent) {
        // set the object security option to 0 length
        // as the value will be carried in message payload
        objectSecurity->length = 0;
        objectSecurity->pValue = NULL;
    }
    else {
        objectSecurity->length = msg->length;
        // FIXME use the upper bytes in the msg->packet buffer
        memcpy(&msg->packet[0], &msg->payload[0], msg->length);
        objectSecurity->pValue = &msg->packet[0];
        packetfunctions_tossHeader(msg, msg->length); // reset packet to zero as objectSecurity option will cary payload
    }

    return E_SUCCESS;
}

uint16_t openoscoap_get_sequence_number(oscoap_security_context_t *context) {
    if (context->sequenceNumber == 0xffff) {
        openserial_printError(
                COMPONENT_OPENOSCOAP,ERR_SEQUENCE_NUMBER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)0
      );
    } else {
        context->sequenceNumber++;
    }
    return context->sequenceNumber;
}


//=========================== private =========================================

owerror_t hkdf_derive_parameter(uint8_t* buffer,
        uint8_t* masterSecret, 
        uint8_t masterSecretLen, 
        uint8_t* masterSalt,
        uint8_t masterSaltLen,
        uint8_t* identifier,
        uint8_t identifierLen,
        uint8_t algorithm,
        oscoap_derivation_t type,
        uint8_t length
        ){
   
    const uint8_t iv[] = "IV";
    const uint8_t key[] = "Key";
    uint8_t info[20];
    uint8_t infoLen;
    uint8_t *temp;
    uint8_t ret;

    temp = info;

    infoLen = 0;
    infoLen += cborencoder_put_array(&temp, 4);
    infoLen += cborencoder_put_bytes(&temp, identifierLen, identifier);
    infoLen += cborencoder_put_unsigned(&temp, algorithm);

    if (type == OSCOAP_DERIVATION_TYPE_KEY) {
        infoLen += cborencoder_put_text(&temp, (char *) key, sizeof(key)-1);
    } 
    else if (type == OSCOAP_DERIVATION_TYPE_IV) {
        infoLen += cborencoder_put_text(&temp, (char *) iv, sizeof(iv)-1);
    }
    else {
        return E_FAIL;
    }
    
    infoLen += cborencoder_put_unsigned(&temp, length);

    ret = hkdf(SHA256, masterSalt, masterSaltLen, masterSecret, masterSecretLen, info, infoLen, buffer, length);

    if (ret == shaSuccess) {
        return E_SUCCESS;
    }
    return E_FAIL;
}

bool is_request(uint8_t code) {
   if ( code == (uint8_t) COAP_CODE_REQ_GET     ||
        code == (uint8_t) COAP_CODE_REQ_POST    ||
        code == (uint8_t) COAP_CODE_REQ_PUT     ||
        code == (uint8_t) COAP_CODE_REQ_DELETE) {
        return TRUE;
   }
   else {
        return FALSE;
   }
}

uint8_t construct_aad(uint8_t* buffer, 
        uint8_t version, 
        uint8_t code,
        uint8_t* optionsSerialized,
        uint8_t optionsSerializedLen,
        uint8_t aeadAlgorithm,
        uint8_t* requestKid,
        uint8_t requestKidLen,
        uint8_t* requestSeq,
        uint8_t requestSeqLen
        ) {
    uint8_t* ptr;
    uint8_t externalAAD[EAAD_MAX_LEN];
    uint8_t externalAADLen;
    uint8_t ret;
    const uint8_t encrypt0[] = "Encrypt0";

    ret = 0;
    externalAADLen = 0;

    ptr = externalAAD;
    externalAADLen += cborencoder_put_array(&ptr, 6);
    externalAADLen += cborencoder_put_unsigned(&ptr, version);
    externalAADLen += cborencoder_put_unsigned(&ptr, code);
    externalAADLen += cborencoder_put_bytes(&ptr, optionsSerializedLen, optionsSerialized);
    externalAADLen += cborencoder_put_unsigned(&ptr, aeadAlgorithm);
    externalAADLen += cborencoder_put_bytes(&ptr, requestKidLen, requestKid);
    externalAADLen += cborencoder_put_bytes(&ptr, requestSeqLen, requestSeq);

    if (externalAADLen > EAAD_MAX_LEN) {
        // corruption
        openserial_printError(
                COMPONENT_OPENOSCOAP,ERR_BUFFER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)0
        );
        return 0;
    }

    ptr = buffer;
    ret += cborencoder_put_array(&ptr, 3); // COSE Encrypt0 structure with 3 elements
    // first element: "Encrypt0"
    ret += cborencoder_put_text(&ptr, (char *) encrypt0, sizeof(encrypt0) - 1); 
    // second element: empty byte string
    ret += cborencoder_put_bytes(&ptr, 0, NULL); 
    // third element: external AAD from OSCOAP
    ret += cborencoder_put_bytes(&ptr, externalAADLen, externalAAD);
    
    return ret;
}

void openoscoap_encode_compressed_COSE(OpenQueueEntry_t* msg, 
        uint8_t* partialIV, uint8_t partialIVLen, 
        uint8_t* kid, 
        uint8_t kidLen) {
    // ciphertext is already encoded and of length msg->length
    uint8_t kidFlag;

    if (kidLen != 0) {
        kidFlag = 1;
    }
    else {
        kidFlag = 0;
    }

    if (kidFlag) {
        packetfunctions_reserveHeaderSize(msg, kidLen + 1);
        msg->payload[0] = kidLen;
        memcpy(&msg->payload[1], kid, kidLen);
    }

    if (partialIVLen) {
        packetfunctions_reserveHeaderSize(msg, partialIVLen);
        memcpy(&msg->payload[0], partialIV, partialIVLen);
    }
    // flag byte
    packetfunctions_reserveHeaderSize(msg, 1);
    msg->payload[0] = ((kidFlag << 3) | partialIVLen);
}


void xor_arrays(uint8_t* s1, uint8_t* s2, uint8_t* dst, uint8_t len) {
    uint8_t i;
    for (i = 0; i < len; i++) {
        dst[i] = s1[i] ^ s2[i];
    }
}

void flip_first_bit(uint8_t* source, uint8_t* dst, uint8_t len){
    memcpy(dst, source, len);
    dst[0] = dst[0] ^ 0x80;
}
