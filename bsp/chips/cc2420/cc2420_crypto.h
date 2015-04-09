/**
\brief CC2420-specific crypto library with hardware acceleration.

\author Malisa Vucinic <malishav@gmail.com>, April 2015.
*/
#ifndef __CC2420_CRYPTO_H__
#define __CC2420_CRYPTO_H__

//=========================== prototypes ======================================

owerror_t cc2420_crypto_aes_ecb_enc(uint8_t* buffer, uint8_t* key);

#endif /* __CC2420_CRYPTO_H__ */
