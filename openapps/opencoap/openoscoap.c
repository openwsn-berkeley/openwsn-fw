#include "opendefs.h"
#include "openoscoap.h"
#include "cborencoder.h"
#include "sha.h"


//=========================== defines =========================================

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

