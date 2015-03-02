/**
\brief Defitions for crypto driver initialization

\author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
*/
#ifndef __CRYPTO_DRIVER_H__
#define __CRYPTO_DRIVER_H__

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
    #define port_INLINE   __inline
    #define BEGIN_PACK    __pragma(pack(1))
    #define END_PACK      __pragma(pack())
#else /* GCC compiler */
    #define port_INLINE   inline
    #define BEGIN_PACK    _Pragma("pack(1)")
    #define END_PACK      _Pragma("pack()")
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CBC_MAC_SIZE  4

// types of addresses
enum {
    ADDR_NONE = 0,
    ADDR_16B = 1,
    ADDR_64B = 2,
    ADDR_128B = 3,
    ADDR_PANID = 4,
    ADDR_PREFIX = 5,
    ADDR_ANYCAST = 6,
};

enum {
    OW_LITTLE_ENDIAN = TRUE,
    OW_BIG_ENDIAN = FALSE,
};

BEGIN_PACK;
typedef struct {                                 // always written big endian, i.e. MSB in addr[0]
    uint8_t type;
    union {
        uint8_t addr_16b[2];
        uint8_t addr_64b[8];
        uint8_t addr_128b[16];
        uint8_t panid[2];
        uint8_t prefix[8];
    };
} open_addr_t;
END_PACK;

BEGIN_PACK;
typedef struct {
    uint8_t  byte4;
    uint16_t bytes2and3;
    uint16_t bytes0and1;
} asn_t;
END_PACK;

typedef int (*fp_aes_ccms_enc)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t *len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key);
typedef int (*fp_aes_ccms_dec)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t *len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key);
typedef int (*fp_aes_cbc_mac_enc)(uint8_t *a, uint8_t len_a, uint8_t *m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key, uint8_t *mac, uint8_t len_mac);
typedef int (*fp_aes_cbc_mac_enc_raw)(uint8_t *buffer, uint8_t len, uint8_t key[16]);
typedef int (*fp_aes_ctr_enc)(uint8_t *m, uint8_t len_m, uint8_t saddr[8], uint8_t asn[5], uint8_t *key, uint8_t *mac, uint8_t len_mac);
typedef int (*fp_aes_ctr_enc_raw)(uint8_t *buffer, uint8_t len, uint8_t *key, uint8_t iv[16]);
typedef int (*fp_aes_ecb_enc)(uint8_t *buffer, uint8_t *key);

typedef struct crypto_driver_s
{
    fp_aes_ccms_enc aes_ccms_enc;
    fp_aes_ccms_dec aes_ccms_dec;
    fp_aes_cbc_mac_enc aes_cbc_mac_enc;
    fp_aes_cbc_mac_enc_raw aes_cbc_mac_enc_raw;
    fp_aes_ctr_enc aes_ctr_enc;
    fp_aes_ctr_enc_raw aes_ctr_enc_raw;
    fp_aes_ecb_enc aes_ecb_enc;
} crypto_driver_t;

int crypto_driver_init(void);
const crypto_driver_t* crypto_driver_get(void);

#ifdef  __cplusplus
}
#endif

#endif /* __CRYPTO_DRIVER_H__ */
