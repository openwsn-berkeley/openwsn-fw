#include "opendefs.h"
#include "oscoap.h"

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

void oscoap_init_security_context(oscoap_security_context_t *ctx, 
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
    // invoke HKDF to get sender IV
    ctx->sequenceNumber = 0;

    // recipient context
    memcpy(ctx->recipientID, recipientID, recipientIDLen);
    ctx->recipientIDLen = recipientIDLen;
    // invoke HKDF to get recipient Key
    // invoke HKDF to get recipient IV
    ctx->window.bitArray = 0;
    ctx->window.base = 0;

}

