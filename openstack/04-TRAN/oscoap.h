
#ifndef __OSCOAP_H
#define __OSCOAP_H

/**
\addtogroup Transport
\{
\addtogroup openCoap
\{
*/

//=========================== define ==========================================

#define OSCOAP_MAX_ID_LEN              10

#define OSCOAP_MASTER_SECRET_LEN       16

#define OSCOAP_MAX_MASTER_SALT_LEN     0

#define AES_CCM_16_64_128              10   // algorithm value as defined in COSE spec

#define AES_CCM_16_64_128_KEY_LEN      16

#define AES_CCM_16_64_128_IV_LEN       13

#define AES_CCM_16_64_128_TAG_LEN      8

//=========================== typedef =========================================
typedef struct {
   uint32_t              bitArray;
   uint16_t              base;
} replay_window_t;

typedef struct {
    // common context
   uint8_t               aeadAlgorithm;
    // sender context 
   uint8_t               senderID[OSCOAP_MAX_ID_LEN];
   uint8_t               senderIDLen;
   uint8_t               senderKey[AES_CCM_16_64_128_KEY_LEN];
   uint8_t               senderIV[AES_CCM_16_64_128_IV_LEN];
   uint16_t              sequenceNumber;
   // recipient context 
   uint8_t               recipientID[OSCOAP_MAX_ID_LEN];
   uint8_t               recipientIDLen;
   uint8_t*              recipientKey[AES_CCM_16_64_128_KEY_LEN];
   uint8_t*              recipientIV[AES_CCM_16_64_128_IV_LEN];
   replay_window_t       window; 
} oscoap_security_context_t;

//=========================== prototypes ======================================

void oscoap_init_security_context(oscoap_security_context_t *ctx, 
                                uint8_t* senderID, 
                                uint8_t senderIDLen,
                                uint8_t* recipientID,
                                uint8_t recipientIDLen,
                                uint8_t* masterSecret,
                                uint8_t masterSecretLen,
                                uint8_t* masterSalt,
                                uint8_t masterSaltLen);
/**
\}
\}
*/

#endif
