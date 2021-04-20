#include "opendefs.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "oscore.h"
#include "cborencoder.h"
#include "ccms.h"
#include "sha.h"


//=========================== defines =========================================

#define EAAD_MAX_LEN            9 + OSCOAP_MAX_ID_LEN // assumes no Class I options
#define AAD_MAX_LEN            12 + EAAD_MAX_LEN
#define INFO_MAX_LEN           2 * OSCOAP_MAX_ID_LEN + 2 + 1 + 4 + 1 + 3 

//=========================== variables =======================================

//=========================== prototype =======================================
owerror_t hkdf_derive_parameter(uint8_t *buffer,
                                uint8_t *masterSecret,
                                uint8_t masterSecretLen,
                                uint8_t *masterSalt,
                                uint8_t masterSaltLen,
                                uint8_t *identifier,
                                uint8_t identifierLen,
				uint8_t *idContext,
				uint8_t idContextLen,
                                uint8_t algorithm,
                                oscore_derivation_t type,
                                uint8_t length
);

bool is_request(uint8_t code);

uint8_t oscore_construct_aad(uint8_t *buffer,
                             uint8_t version,
                             uint8_t aeadAlgorithm,
                             uint8_t *requestKid,
                             uint8_t requestKidLen,
                             uint8_t *requestSeq,
                             uint8_t requestSeqLen,
                             uint8_t *optionsSerialized,
                             uint8_t optionsSerializedLen);

void oscore_construct_nonce(uint8_t *buffer,
		            uint8_t *partialIV,
			    uint8_t partialIVLen,
			    uint8_t *idPiv,
			    uint8_t idPivLen,
			    uint8_t *commonIV,
			    uint8_t commonIVLen);

uint8_t oscore_encode_compressed_COSE(uint8_t *buf,
		                   uint8_t bufMaxLen,
                                   uint8_t *requestSeq,
				   uint8_t requestSeqLen,
				   uint8_t *kidContext,
				   uint8_t kidContextLen,
                                   uint8_t *requestKid,
                                   uint8_t requestKidLen);

void xor_arrays(uint8_t *s1, uint8_t *s2, uint8_t *dst, uint8_t len);

void flip_first_bit(uint8_t *source, uint8_t *dst, uint8_t len);

bool replay_window_check(oscore_security_context_t *context, uint16_t sequenceNumber);

void replay_window_update(oscore_security_context_t *context, uint16_t sequenceNumber);

uint8_t oscore_convert_sequence_number(uint16_t sequenceNumber, uint8_t **buffer);
//=========================== public ==========================================


void oscore_init_security_context(oscore_security_context_t *ctx,
                                  uint8_t *senderID,
                                  uint8_t senderIDLen,
                                  uint8_t *recipientID,
                                  uint8_t recipientIDLen,
				  uint8_t *idContext,
				  uint8_t idContextLen,
                                  uint8_t *masterSecret,
                                  uint8_t masterSecretLen,
                                  uint8_t *masterSalt,
                                  uint8_t masterSaltLen) {

    if (senderIDLen > OSCOAP_MAX_ID_LEN || recipientIDLen > OSCOAP_MAX_ID_LEN) {
        return;
    }

    // common context
    ctx->aeadAlgorithm = AES_CCM_16_64_128;

    memcpy(ctx->idContext, idContext, idContextLen);
    ctx->idContextLen = idContextLen;

    // invoke HKDF to get common IV
    hkdf_derive_parameter(ctx->commonIV,
                          masterSecret,
                          masterSecretLen,
                          masterSalt,
                          masterSaltLen,
                          NULL,
                          0,
			  idContext,
			  idContextLen,
                          AES_CCM_16_64_128,
                          OSCOAP_DERIVATION_TYPE_IV,
                          AES_CCM_16_64_128_IV_LEN);


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
			  idContext,
			  idContextLen,
                          AES_CCM_16_64_128,
                          OSCOAP_DERIVATION_TYPE_KEY,
                          AES_CCM_16_64_128_KEY_LEN);
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
			  idContext,
			  idContextLen,
                          AES_CCM_16_64_128,
                          OSCOAP_DERIVATION_TYPE_KEY,
                          AES_CCM_16_64_128_KEY_LEN);

    ctx->window.bitArray = 0x01; // LSB set
    ctx->window.rightEdge = 0;

}

owerror_t oscore_protect_message(
        oscore_security_context_t *context,
        uint8_t version,
        coap_code_t *code,
        coap_option_iht *incomingOptions,
        uint8_t incomingOptionsLen,
        OpenQueueEntry_t *msg,
        uint16_t sequenceNumber) {

    uint8_t *payload;
    uint8_t payloadLen;
    uint8_t aad[AAD_MAX_LEN];
    uint8_t aadLen;
    uint8_t nonce[AES_CCM_16_64_128_IV_LEN];
    uint8_t partialIV[AES_CCM_16_64_128_IV_LEN];
    uint8_t *requestSeq;
    uint8_t requestSeqLen;
    uint8_t *requestKid;
    uint8_t requestKidLen;
    uint8_t *idContext;
    uint8_t idContextLen;
    owerror_t encStatus;
    coap_option_iht *objectSecurity;
    uint8_t option_count;
    uint8_t option_index;

    // find object security option in the list of passed options
    option_count = coap_find_option(incomingOptions, incomingOptionsLen, COAP_OPTION_NUM_OSCORE,
                                    &option_index);
    if (option_count >= 1) {
        objectSecurity = &incomingOptions[option_index];
    } else {
        objectSecurity = NULL;
    }
    if (objectSecurity == NULL) { // objectSecurity option should be set by the application
        return E_FAIL;
    }

    // convert sequence number to array and strip leading zeros
    memset(partialIV, 0x00, AES_CCM_16_64_128_IV_LEN);
    requestSeq = &partialIV[AES_CCM_16_64_128_IV_LEN - 2];
    requestSeqLen = oscore_convert_sequence_number(sequenceNumber, &requestSeq);

    if (is_request(*code)) {
        requestKid = context->senderID;
        requestKidLen = context->senderIDLen;
    } else {
        requestKid = context->recipientID;
        requestKidLen = context->recipientIDLen;
    }

    if (msg->length > 0) { // contains payload, add payload marker
        if (packetfunctions_reserveHeader(&msg, 1) == E_FAIL){
            return E_FAIL;
        }
        msg->payload[0] = COAP_PAYLOAD_MARKER;
    }

    // encode the options to the openqueue payload buffer
    coap_options_encode(msg, incomingOptions, incomingOptionsLen, COAP_OPTION_CLASS_E);

    // encode CoAP code
    packetfunctions_reserveHeader(&msg, 1);
    msg->payload[0] = *code;

    payload = &msg->payload[0];
    payloadLen = msg->length;
    // shift payload to leave space for authentication tag
    if (packetfunctions_reserveHeader(&msg, AES_CCM_16_64_128_TAG_LEN) == E_FAIL){
        return E_FAIL;
    }
    memcpy(&msg->payload[0], payload, payloadLen);
    // update payload pointer but leave length intact
    payload = &msg->payload[0];

    aadLen = oscore_construct_aad(aad,
                                  version,
                                  AES_CCM_16_64_128,
                                  requestKid,
                                  requestKidLen,
                                  requestSeq,
                                  requestSeqLen,
				  NULL,
                                  0); // do not support Class I options at the moment

    if (aadLen > AAD_MAX_LEN) {
        // corruption
        LOG_ERROR(COMPONENT_OSCORE, ERR_BUFFER_OVERFLOW, (errorparameter_t) 0, (errorparameter_t) 1);
        return E_FAIL;
    }



    if (is_request(*code)) {
	// return "encrypted" code
	*code = COAP_CODE_REQ_POST;
	idContext = context->idContext;
	idContextLen = context->idContextLen;
    } else {
	*code = COAP_CODE_RESP_CHANGED;
        // do not encode sequence number and ID in the response
        requestSeq = NULL;
        requestSeqLen = 0;
        requestKid = NULL;
        requestKidLen = 0;
	idContext = NULL;
	idContextLen = 0;
    }

    // construct nonce
    oscore_construct_nonce(nonce,
		           requestSeq,
			   requestSeqLen,
			   requestKid,
			   requestKidLen,
			   context->commonIV,
			   AES_CCM_16_64_128_IV_LEN);

    encStatus = aes128_ccms_enc(aad,
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
    objectSecurity->length = oscore_encode_compressed_COSE(objectSecurity->pValue,
		    objectSecurity->length,
		    requestSeq,
		    requestSeqLen,
		    idContext,
		    idContextLen,
		    requestKid,
		    requestKidLen);

    return E_SUCCESS;
}

owerror_t oscore_unprotect_message(
        oscore_security_context_t *context,
        uint8_t version,
        coap_code_t *code,
        coap_option_iht *incomingOptions,
        uint8_t *incomingOptionsLen,
        OpenQueueEntry_t *msg,
        uint16_t sequenceNumber) {

    uint8_t nonce[AES_CCM_16_64_128_IV_LEN];
    uint8_t partialIV[AES_CCM_16_64_128_IV_LEN];
    uint8_t *requestKid;
    uint8_t requestKidLen;
    uint8_t *requestSeq;
    uint8_t requestSeqLen;
    uint8_t aad[AAD_MAX_LEN];
    uint8_t aadLen;
    coap_option_iht *objectSecurity;
    owerror_t decStatus;
    uint8_t *ciphertext;
    uint8_t ciphertextLen;
    uint8_t index;
    uint8_t option_count;
    uint8_t option_index;

    // find object security option in the list of passed options
    option_count = coap_find_option(incomingOptions, *incomingOptionsLen, COAP_OPTION_NUM_OSCORE,
                                    &option_index);
    if (option_count >= 1) {
        objectSecurity = &incomingOptions[option_index];
    } else {
        objectSecurity = NULL;
    }
    if (objectSecurity == NULL) { // return FAIL if object security option is not present
        return E_FAIL;
    }

    ciphertext = &msg->payload[0];
    ciphertextLen = msg->length;

    if (is_request(*code)) {
        if (replay_window_check(context, sequenceNumber) == FALSE) {
            LOG_ERROR(COMPONENT_OSCORE, ERR_REPLAY_FAILED, (errorparameter_t) 0, (errorparameter_t) 0);
            return E_FAIL;
        }
        requestKid = context->recipientID;
        requestKidLen = context->recipientIDLen;
    } else {
        requestKid = context->senderID;
        requestKidLen = context->senderIDLen;
    }

    // convert sequence number to array and strip leading zeros
    memset(partialIV, 0x00, AES_CCM_16_64_128_IV_LEN);
    requestSeq = &partialIV[AES_CCM_16_64_128_IV_LEN - 2];
    requestSeqLen = oscore_convert_sequence_number(sequenceNumber, &requestSeq);

    aadLen = oscore_construct_aad(aad,
                                  version,
                                  AES_CCM_16_64_128,
                                  requestKid,
                                  requestKidLen,
                                  requestSeq,
                                  requestSeqLen,
				  NULL,
                                  0); // do not support Class I options at the moment

    if (aadLen > AAD_MAX_LEN) {
        // corruption
        LOG_ERROR(COMPONENT_OSCORE, ERR_BUFFER_OVERFLOW, (errorparameter_t) 0, (errorparameter_t) 0);
        return E_FAIL;
    }

    oscore_construct_nonce(nonce,
		           requestSeq,
			   requestSeqLen,
			   requestKid,
			   requestKidLen,
			   context->commonIV,
			   AES_CCM_16_64_128_IV_LEN);

    decStatus = aes128_ccms_dec(aad,
                                aadLen,
                                ciphertext,
                                &ciphertextLen,
                                nonce,
                                2,
                                context->recipientKey,
                                AES_CCM_16_64_128_TAG_LEN);

    if (decStatus != E_SUCCESS) {
        LOG_ERROR(COMPONENT_OSCORE, ERR_DECRYPTION_FAILED, (errorparameter_t) 0, (errorparameter_t) 0);
        return E_FAIL;
    }

    if (is_request(*code)) {
        replay_window_update(context, sequenceNumber);
    }

    packetfunctions_tossFooter(&msg, AES_CCM_16_64_128_TAG_LEN);

    // parse code from plaintext
    *code = msg->payload[0];
    packetfunctions_tossHeader(&msg, 1);
    // parse inner coap options
    index = coap_options_parse(&msg->payload[0], msg->length, incomingOptions, incomingOptionsLen);
    packetfunctions_tossHeader(&msg, index);

    return E_SUCCESS;
}

uint16_t oscore_get_sequence_number(oscore_security_context_t *context) {
    if (context->sequenceNumber == 0xffff) {
        LOG_ERROR(COMPONENT_OSCORE, ERR_SEQUENCE_NUMBER_OVERFLOW, (errorparameter_t) 0, (errorparameter_t) 0);
    } else {
        context->sequenceNumber++;
    }
    return context->sequenceNumber;
}

owerror_t oscore_parse_compressed_COSE(uint8_t *buffer,
                                     uint8_t bufferLen,
                                     uint16_t *sequenceNumber,
				     uint8_t **kidContext,
				     uint8_t *kidContextLen,
                                     uint8_t **kid,
                                     uint8_t *kidLen)
{
    uint8_t tmp;
    uint8_t *ptr;
    uint8_t index;
    uint8_t n;
    uint8_t k;
    uint8_t h;
    uint8_t reserved;

    if (bufferLen == 0) {
        tmp = 0x00;
	ptr = &tmp;
	bufferLen = 1;
    } else {
        ptr = buffer;
    }

    index = 0;
    n = (ptr[index] >> 0) & 0x07;
    k = (ptr[index] >> 3) & 0x01;
    h = (ptr[index] >> 4) & 0x01;
    reserved = (ptr[index] >> 5) & 0x07;

    if (reserved) {
        return E_FAIL;
    }

    index++;

    if (n > 2) {
        return 0;
    } else if (n == 1) {
        *sequenceNumber = ptr[index];
        index++;
    } else if (n == 2) {
        *sequenceNumber = packetfunctions_ntohs(&ptr[index]);
        index += 2;
    }

    if (h) {
        *kidContextLen = ptr[index];
	index++;
	*kidContext = &ptr[index];
	index += *kidContextLen;
    }

    if (k) {
        *kidLen = bufferLen - index;
        *kid = (*kidLen == 0) ? NULL : &ptr[index];
        index += *kidLen;
    }

    return E_SUCCESS;
}


//=========================== private =========================================

owerror_t hkdf_derive_parameter(uint8_t *buffer,
                                uint8_t *masterSecret,
                                uint8_t masterSecretLen,
                                uint8_t *masterSalt,
                                uint8_t masterSaltLen,
                                uint8_t *identifier,
                                uint8_t identifierLen,
				uint8_t *idContext,
				uint8_t idContextLen,
                                uint8_t algorithm,
                                oscore_derivation_t type,
                                uint8_t length)
{

    const uint8_t iv[] = "IV";
    const uint8_t key[] = "Key";
    uint8_t info[INFO_MAX_LEN];
    uint8_t infoLen;
    uint8_t ret;

    infoLen = 0;
    infoLen += cborencoder_put_array(&info[infoLen], 5);
    infoLen += cborencoder_put_bytes(&info[infoLen], identifier, identifierLen);
    if (idContextLen && idContext) {
        infoLen += cborencoder_put_bytes(&info[infoLen], idContext, idContextLen);
    } else {
        infoLen += cborencoder_put_null(&info[infoLen]);
    }
    infoLen += cborencoder_put_unsigned(&info[infoLen], algorithm);

    if (type == OSCOAP_DERIVATION_TYPE_KEY) {
        infoLen += cborencoder_put_text(&info[infoLen], (char *) key, sizeof(key) - 1);
    } else if (type == OSCOAP_DERIVATION_TYPE_IV) {
        infoLen += cborencoder_put_text(&info[infoLen], (char *) iv, sizeof(iv) - 1);
    } else {
        return E_FAIL;
    }

    infoLen += cborencoder_put_unsigned(&info[infoLen], length);

    ret = hkdf(SHA256, masterSalt, masterSaltLen, masterSecret, masterSecretLen, info, infoLen, buffer, length);

    if (ret == shaSuccess) {
        return E_SUCCESS;
    }
    return E_FAIL;
}

bool is_request(uint8_t code) {
    if (code == (uint8_t) COAP_CODE_REQ_GET ||
        code == (uint8_t) COAP_CODE_REQ_POST ||
        code == (uint8_t) COAP_CODE_REQ_PUT ||
        code == (uint8_t) COAP_CODE_REQ_DELETE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t oscore_construct_aad(uint8_t *buffer,
                             uint8_t version,
                             uint8_t aeadAlgorithm,
                             uint8_t *requestKid,
                             uint8_t requestKidLen,
                             uint8_t *requestSeq,
                             uint8_t requestSeqLen,
                             uint8_t *optionsSerialized,
                             uint8_t optionsSerializedLen
) {
    uint8_t externalAAD[EAAD_MAX_LEN];
    uint8_t externalAADLen;
    uint8_t ret;
    const uint8_t encrypt0[] = "Encrypt0";

    ret = 0;
    externalAADLen = 0;

    externalAADLen += cborencoder_put_array(&externalAAD[externalAADLen], 5);
    externalAADLen += cborencoder_put_unsigned(&externalAAD[externalAADLen], version);
    externalAADLen += cborencoder_put_array(&externalAAD[externalAADLen], 1);
    externalAADLen += cborencoder_put_unsigned(&externalAAD[externalAADLen], aeadAlgorithm);
    externalAADLen += cborencoder_put_bytes(&externalAAD[externalAADLen], requestKid, requestKidLen);
    externalAADLen += cborencoder_put_bytes(&externalAAD[externalAADLen], requestSeq, requestSeqLen);
    externalAADLen += cborencoder_put_bytes(&externalAAD[externalAADLen], optionsSerialized, optionsSerializedLen);

    if (externalAADLen > EAAD_MAX_LEN) {
        // corruption
        LOG_ERROR(COMPONENT_OSCORE, ERR_BUFFER_OVERFLOW, (errorparameter_t) 0, (errorparameter_t) 0);
        return 0;
    }

    ret += cborencoder_put_array(&buffer[ret], 3); // COSE Encrypt0 structure with 3 elements
    // first element: "Encrypt0"
    ret += cborencoder_put_text(&buffer[ret], (char *) encrypt0, sizeof(encrypt0) - 1);
    // second element: empty byte string
    ret += cborencoder_put_bytes(&buffer[ret], NULL, 0);
    // third element: external AAD from OSCOAP
    ret += cborencoder_put_bytes(&buffer[ret], externalAAD, externalAADLen);

    return ret;
}

//              <- nonce length minus 6 B -> <-- 5 bytes -->
//         +---+-------------------+--------+---------+-----+
//         | S |      padding      | ID_PIV | padding | PIV |----+
//         +---+-------------------+--------+---------+-----+    |
//                                                               |
//          <---------------- nonce length ---------------->     |
//         +------------------------------------------------+    |
//         |                   Common IV                    |->(XOR)
//         +------------------------------------------------+    |
//                                                               |
//          <---------------- nonce length ---------------->     |
//         +------------------------------------------------+    |
//         |                     Nonce                      |<---+
//         +------------------------------------------------+
void oscore_construct_nonce(uint8_t *buffer, // needs to hold AES_CCM_16_64_128_IV_LEN bytes
		            uint8_t *partialIV,
			    uint8_t partialIVLen,
			    uint8_t *idPiv,
			    uint8_t idPivLen,
			    uint8_t *commonIV,
			    uint8_t commonIVLen) {
    uint8_t temp[AES_CCM_16_64_128_IV_LEN];

    if (commonIVLen != AES_CCM_16_64_128_IV_LEN) {
        return;
    }

    memset(temp, 0x00, AES_CCM_16_64_128_IV_LEN);
    /* Step 1 */
    memcpy(&temp[commonIVLen - partialIVLen], partialIV, partialIVLen);
    /* Step 2 */
    memcpy(&temp[commonIVLen - 5 - idPivLen], idPiv, idPivLen);
    /* Step 3 */
    temp[0] = idPivLen;
    /* Now XOR with Common IV */
    xor_arrays(commonIV, temp, buffer, commonIVLen);

    return;
}

uint8_t oscore_encode_compressed_COSE(uint8_t *buf,
                                   uint8_t bufMaxLen,
                                   uint8_t *partialIV,
				   uint8_t partialIVLen,
				   uint8_t *kidContext,
				   uint8_t kidContextLen,
                                   uint8_t *kid,
                                   uint8_t kidLen) {
    // ciphertext is already encoded and of length msg->length
    uint8_t *tmp;
    uint8_t k;
    uint8_t h;

    tmp = buf;

    if (kid != NULL) {
        k = 1;
    } else {
        k = 0;
    }

    if (kidContext != NULL) {
        h = 1;
    } else {
        h = 0;
    }

    // flag byte
    *tmp = ((h << 4) | (k << 3) | (partialIVLen & 0x07));
    tmp++;

    if (partialIVLen) {
        memcpy(tmp, partialIV, partialIVLen);
	tmp += partialIVLen;
    }

    if (h) {
	*tmp = kidContextLen;
	tmp++;
	memcpy(tmp, kidContext, kidContextLen);
	tmp += kidContextLen;
    }

    if (k) {
        memcpy(tmp, kid, kidLen);
	tmp += kidLen;
    }

    if (tmp - buf > bufMaxLen) {
        // corruption
        LOG_ERROR(COMPONENT_OSCORE, ERR_BUFFER_OVERFLOW, (errorparameter_t) 0, (errorparameter_t) 1);
        return 0;
    }
    return tmp - buf;
}


void xor_arrays(uint8_t *s1, uint8_t *s2, uint8_t *dst, uint8_t len) {
    uint8_t i;
    for (i = 0; i < len; i++) {
        dst[i] = s1[i] ^ s2[i];
    }
}

void flip_first_bit(uint8_t *source, uint8_t *dst, uint8_t len) {
    memcpy(dst, source, len);
    dst[0] = dst[0] ^ 0x80;
}

bool replay_window_check(oscore_security_context_t *context, uint16_t sequenceNumber) {
    uint16_t delta;

    // packets lower than the left edge are rejected
    if ((int) sequenceNumber < (int) (context->window.rightEdge - 31)) {
        return FALSE;
    }

    // packets higher than the right edge are accepted
    if (sequenceNumber > context->window.rightEdge) {
        return TRUE;
    }

    // packet falls within the window, check if appropriate bit is set
    delta = context->window.rightEdge - sequenceNumber;
    if ((uint32_t)(1 << delta) & context->window.bitArray) {
        return FALSE;
    }

    return TRUE;
}

void replay_window_update(oscore_security_context_t *context, uint16_t sequenceNumber) {
    uint16_t delta;

    if (replay_window_check(context, sequenceNumber) == FALSE) {
        return;
    }

    if (sequenceNumber > context->window.rightEdge) {
        delta = sequenceNumber - context->window.rightEdge;
        context->window.rightEdge = sequenceNumber;
        if (delta < 32) {
            context->window.bitArray = context->window.bitArray << delta;
        } else {
            context->window.bitArray = 0x00;
        }
        context->window.bitArray |= 1; // update the right edge bit
    } else {
        delta = context->window.rightEdge - sequenceNumber;
        if (delta < 32) {
            context->window.bitArray |= 1 << delta;
        }
    }
}

uint8_t oscore_convert_sequence_number(uint16_t sequenceNumber, uint8_t **buffer) {
    packetfunctions_htons(sequenceNumber, *buffer);
    if (**buffer == 0x00) {
        (*buffer)++;
        return 1;
    } else {
        return 2;
    }
}
