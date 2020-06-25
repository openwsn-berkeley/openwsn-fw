#ifndef OPENWSN_OSCORE_H
#define OPENWSN_OSCORE_H

/**
\addtogroup Transport
\{
\addtogroup openCoap
\{
*/

#include "coap.h"

//=========================== define ==========================================

//=========================== typedef =========================================
typedef enum {
    OSCOAP_DERIVATION_TYPE_KEY = 0,
    OSCOAP_DERIVATION_TYPE_IV = 1,
} oscore_derivation_t;

//=========================== module variables ================================

//=========================== prototypes ======================================


/**
\brief Initialize OSCORE security context.

This function will derive the parameters needed to initialize OSCORE
security context.

\param[out] ctx OSCORE security context structure.
\param[in] senderID Pointer to the Byte array containing Sender ID.
\param[in] senderIDLen Length of the Sender ID byte array in bytes.
\param[in] recipientID Pointer to the Byte array contaning Recipient ID.
\param[in] recipientIDLen Length of the Recipient ID byte array in bytes.
\param[in] Pointer to the byte array contaning the Master Secret.
\param[in] Length of the Master Secret byte array in bytes.
\param[in] Pointer to the byte array contaning Master Salt.
\param[in] Length of the Master Salt byte array in bytes.
*/
void oscore_init_security_context(oscore_security_context_t *ctx,
                                  uint8_t *senderID,
                                  uint8_t senderIDLen,
                                  uint8_t *recipientID,
                                  uint8_t recipientIDLen,
                                  uint8_t *masterSecret,
                                  uint8_t masterSecretLen,
                                  uint8_t *masterSalt,
                                  uint8_t masterSaltLen);

owerror_t oscore_protect_message(oscore_security_context_t *context,
                                 uint8_t version,
                                 uint8_t code,
                                 coap_option_iht *options,
                                 uint8_t optionsLen,
                                 OpenQueueEntry_t *msg,
                                 uint16_t sequenceNumber);

owerror_t oscore_unprotect_message(oscore_security_context_t *context,
                                   uint8_t version,
                                   uint8_t code,
                                   coap_option_iht *options,
                                   uint8_t *optionsLen,
                                   OpenQueueEntry_t *msg,
                                   uint16_t sequenceNumber);

uint16_t oscore_get_sequence_number(oscore_security_context_t *context);

uint8_t oscore_parse_compressed_COSE(uint8_t *buffer,
                                     uint8_t bufferLen,
                                     uint16_t *sequenceNumber,
                                     uint8_t **kid,
                                     uint8_t *kidLen);
/**
\}
\}
*/

#endif /* OPENWSN_OSCORE_H*/
