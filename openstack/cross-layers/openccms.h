/**
\brief Definitions for AES CCMS implementation

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>, March 2015.
\author Malisa Vucinic <malishav@gmail.com>, June 2017.
*/
#ifndef __OPENCCMS_H__
#define __OPENCCMS_H__

//=========================== prototypes ======================================

owerror_t openccms_enc(uint8_t* a,uint8_t len_a,uint8_t* m,uint8_t* len_m,uint8_t* nonce, uint8_t l, uint8_t key[16], uint8_t len_mac);
owerror_t openccms_dec(uint8_t* a,uint8_t len_a,uint8_t* m,uint8_t* len_m,uint8_t* nonce, uint8_t l, uint8_t key[16], uint8_t len_mac);

#endif /* __OPENCCMS_H__ */
