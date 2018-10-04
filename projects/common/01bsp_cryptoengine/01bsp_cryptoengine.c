/**
\brief This is a program which shows how to use the cryptoengine interface
       and runs some tests in order to verify different crypto implementations.

\note: You can use this project with any platform but be careful to implement the
       the necessary functions for the board that you want to test.

Load this program on your boards. Radio LED will stay on indefinitely if all
tests passed. If there was an error, we use the Error LED to signal.

\author Malisa Vucinic <malishav@gmail.com>, March 2015.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "cryptoengine.h"
#include "leds.h"
#include "sctimer.h"

#define TEST_AES_ECB                   1
#define TEST_AES_CCMS_ENC              1
#define TEST_AES_CCMS_DEC              1
#define TEST_AES_CCMS_AUTH_FORWARD     1
#define TEST_AES_CCMS_AUTH_INVERSE     1
#define TEST_BENCHMARK_CCMS            1
#define TEST_ECDSA_VERIFY              1

typedef struct {
   uint8_t key[16];
   uint8_t buffer[16];
   uint8_t expected_ciphertext[16];
} aes_ecb_suite_t;

typedef struct
{
    uint8_t key[16];
    uint8_t len_tag;
    uint8_t nonce[13];
    uint8_t a[15];
    uint8_t m[20 + 4];
    uint8_t len_a;
    uint8_t len_m;
    uint8_t l;
    uint8_t expected_ciphertext[24];
} aes_ccms_enc_suite_t;

typedef struct
{
    uint8_t key[16];
    uint8_t len_tag;
    uint8_t nonce[13];
    uint8_t a[15];
    uint8_t c[20 + 4];
    uint8_t len_a;
    uint8_t len_c;
    uint8_t l;
    uint8_t expected_plaintext[20];
} aes_ccms_dec_suite_t;

typedef struct
{
    uint8_t key[16];
    uint8_t len_tag;
    uint8_t nonce[13];
    uint8_t a[26];
    uint8_t m[8];
    uint8_t len_a;
    uint8_t len_m;
    uint8_t l;
    uint8_t expected_ciphertext[8];
} aes_ccms_auth_forward_suite_t;

typedef struct{
    ecdsa_verify_state_t ecdsa_state;
} ecdsa_verify_suite_t;

static int hang(uint8_t error_code) {

   error_code ? leds_error_on() : leds_radio_on();

   while (1);

   return 0;
}

#if TEST_AES_ECB
static owerror_t run_aes_ecb_suite(aes_ecb_suite_t* suite, uint8_t test_suite_len) {
   uint8_t i = 0;
   uint8_t success = 0;

   for(i = 0; i < test_suite_len; i++) {
      if(cryptoengine_aes_ecb_enc(suite[i].buffer, suite[i].key) == E_SUCCESS) {
         if (memcmp(suite[i].buffer, suite[i].expected_ciphertext, 16) == 0) {
            success++;
         }
      }
   }

   return success == test_suite_len ? E_SUCCESS : E_FAIL;
}
#endif /* TEST_AES_ECB */

#if TEST_AES_CCMS_ENC
static owerror_t run_aes_ccms_enc_suite(aes_ccms_enc_suite_t* suite, uint8_t test_suite_len) {
   uint8_t i = 0;
   uint8_t success = 0;

   for(i = 0; i < test_suite_len; i++) {
      if(cryptoengine_aes_ccms_enc(suite[i].a,
                  suite[i].len_a,
                  suite[i].m,
                  &suite[i].len_m,
                  suite[i].nonce,
                  suite[i].l,
                  suite[i].key,
                  suite[i].len_tag) == E_SUCCESS) {

         if(memcmp(suite[i].m, suite[i].expected_ciphertext, suite[i].len_m) == 0) {
            success++;
         }
      }
   }
   return success == test_suite_len ? E_SUCCESS : E_FAIL;
}
#endif /* TEST_AES_CCMS_ENC */

#if TEST_AES_CCMS_DEC
static owerror_t run_aes_ccms_dec_suite(aes_ccms_dec_suite_t* suite, uint8_t test_suite_len) {
   uint8_t i = 0;
   uint8_t success = 0;

   for(i = 0; i < test_suite_len; i++) {

       if(cryptoengine_aes_ccms_dec(suite[i].a,
                       suite[i].len_a,
                       suite[i].c,
                       &suite[i].len_c,
                       suite[i].nonce,
                       suite[i].l,
                       suite[i].key,
                       suite[i].len_tag) == E_SUCCESS) {

         if(memcmp(suite[i].c, suite[i].expected_plaintext, suite[i].len_c) == 0) {
            success++;
         }
      }
   }
   return success == test_suite_len ? E_SUCCESS : E_FAIL;
}
#endif /* TEST_AES_CCMS_DEC */

#if TEST_AES_CCMS_AUTH_FORWARD
static owerror_t run_aes_ccms_auth_forward_suite(aes_ccms_auth_forward_suite_t* suite,
                     uint8_t test_suite_len) {
   uint8_t i = 0;
   uint8_t success = 0;

   for(i = 0; i < test_suite_len; i++) {
      if(cryptoengine_aes_ccms_enc(suite[i].a,
                  suite[i].len_a,
                  suite[i].m,
                  &suite[i].len_m,
                  suite[i].nonce,
                  suite[i].l,
                  suite[i].key,
                  suite[i].len_tag) == E_SUCCESS) {

         if(memcmp(suite[i].m, suite[i].expected_ciphertext, suite[i].len_tag) == 0) {
            success++;
         }
      }
   }
   return success == test_suite_len ? E_SUCCESS : E_FAIL;
}
#endif /* TEST_AES_CCMS_AUTH_FORWARD */

#if TEST_AES_CCMS_AUTH_INVERSE
static owerror_t run_aes_ccms_auth_inverse_suite(aes_ccms_auth_forward_suite_t* suite,
                     uint8_t test_suite_len) {
   uint8_t i = 0;
   uint8_t success = 0;

   for(i = 0; i < test_suite_len; i++) {

       if(cryptoengine_aes_ccms_dec(suite[i].a,
                       suite[i].len_a,
                       suite[i].m,
                       &suite[i].len_m,
                       suite[i].nonce,
                       suite[i].l,
                       suite[i].key,
                       suite[i].len_tag) == E_SUCCESS) {

         if(memcmp(suite[i].m, suite[i].expected_ciphertext, suite[i].len_tag) == 0) {
            success++;
         }
      }
   }
   return success == test_suite_len ? E_SUCCESS : E_FAIL;
}
#endif /* TEST_AES_CCMS_AUTH_INVERSE */

#if TEST_ECDSA_VERIFY
static owerror_t run_ecdsa_verify_suite(ecdsa_verify_suite_t* suite, uint8_t test_suite_len ){
   uint8_t i = 0;
   uint8_t success = 0;

   for(i = 0; i < test_suite_len; i++) {
      //launch the signature verification
      if(cryptoengine_ecdsa_verify(&(suite[i].ecdsa_state)) == E_SUCCESS) {
         success++;
      }
   }

   return success == test_suite_len ? E_SUCCESS : E_FAIL;
}
#endif /* TEST_ECDSA_VERIFY */

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t fail = 0;

#if TEST_AES_ECB
   aes_ecb_suite_t aes_ecb_suite[] = {
      {
         { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
         { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a },
         { 0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97 },
      },
      {
         { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
         { 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51 },
         { 0xf5, 0xd3, 0xd5, 0x85, 0x03, 0xb9, 0x69, 0x9d, 0xe7, 0x85, 0x89, 0x5a, 0x96, 0xfd, 0xba, 0xaf },
      },
      {
         { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
         { 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef },
         { 0x43, 0xb1, 0xcd, 0x7f, 0x59, 0x8e, 0xce, 0x23, 0x88, 0x1b, 0x00, 0xe3, 0xed, 0x03, 0x06, 0x88 },
      },
      {
         { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
         { 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 },
         { 0x7b, 0x0c, 0x78, 0x5e, 0x27, 0xe8, 0xad, 0x3f, 0x82, 0x23, 0x20, 0x71, 0x04, 0x72, 0x5d, 0xd4 },
      },
      {
         { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
         { 0x6c, 0x5f, 0x51, 0x74, 0x53, 0x53, 0x77, 0x5a, 0x5a, 0x5f, 0x57, 0x58, 0x55, 0x53, 0x06, 0x0f },
         { 0x83, 0x78, 0x10, 0x60, 0x0e, 0x13, 0x93, 0x9b, 0x27, 0xe0, 0xd7, 0xe4, 0x58, 0xf0, 0xa9, 0xd1 },
      },
   };
#endif /* TEST_AES_ECB */

/* Test vectors from TI's example implementation */
#if TEST_AES_CCMS_ENC
   aes_ccms_enc_suite_t aes_ccms_enc_suite[] = {

      { /* example case len_a and Mval = 0 */
         { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
            0x0, /* tag_len */
         { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
         { 0x69, 0x98, 0x03, 0x33, 0x63, 0xbb, 0xaa, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03}, /* a vector */
         { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00 }, /* m vector */
         0, /* len_a */
         20, /* len_m */
         2, /* CCM L */
         { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0, 0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6, 0x2c,
            0x61, 0x01, 0x4e, 0x7b, 0x34, 0x4f, 0x09 } /* expected_ciphertext */
      },
      {
         { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
            4, /* tag_len */
         { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
         { 0x69, 0x98, 0x03, 0x33, 0x63, 0xbb, 0xaa, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03}, /* a vector */
         { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00 }, /* m vector + 4 octets for authentication tag */
         15, /* len_a */
         20, /* len_m */
         2, /* CCM L */
         { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0, 0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6, 0x2c,
            0x61, 0x01, 0x4e, 0x7b, 0x34, 0x4f, 0x09 } /* expected ciphertext */
      },
   };
#endif /* TEST_AES_CCMS_ENC */

#if TEST_AES_CCMS_DEC
   aes_ccms_dec_suite_t aes_ccms_dec_suite[] = {

    {
        { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
        0, /* tag len */
        { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
        { 0x69, 0x98, 0x03, 0x33, 0x63, 0xbb, 0xaa, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03}, /* a vector */
        { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0, 0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6, 0x2c,
           0x61, 0x01, 0x4e, 0x7b, 0x34, 0x4f, 0x09}, /* c vector (m + tag) */
        0, /* len_a */
        20, /* len_m */
        2, /* CCM L */
        { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f } /* expected plaintext */
    },
      {
        { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* key */
        4, /* tag len */
        { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x05 }, /* nonce */
        { 0x69, 0x98, 0x03, 0x33, 0x63, 0xbb, 0xaa, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03 }, /* a vector */
        { 0x92, 0xe8, 0xad, 0xca, 0x53, 0x81, 0xbf, 0xd0, 0x5b, 0xdd, 0xf3, 0x61, 0x09, 0x09, 0x82, 0xe6, 0x2c,
            0x61, 0x01, 0x4e, 0x7b, 0x34, 0x4f, 0x09 }, /* c vector (m + tag) */
        15, /* len_a */
        24, /* len_c */
        2, /* CCM L */
        { 0x14, 0xaa, 0xbb, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f } /* expected plaintext */
      },
   };
#endif /* TEST_AES_CCMS_DEC */

#if TEST_AES_CCMS_AUTH_FORWARD
   aes_ccms_auth_forward_suite_t aes_ccms_auth_forward_suite[] = {

    {
        { 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF }, /* key */
        8, /* tag len */
        { 0xAC, 0xDE, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x02 }, /* nonce */
        { 0x08, 0xD0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC, 0x02, 0x05, 0x00,
         0x00, 0x00, 0x55, 0xCF, 0x00, 0x00, 0x51, 0x52, 0x53, 0x54 }, /* a vector */
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* empty m vector with 8 octets to hold the tag*/
        26, /* len_a */
        0, /* len_m */
        2, /* CCM L */
        { 0x22, 0x3B, 0xC1, 0xEC, 0x84, 0x1A, 0xB5, 0x53 } /* expected tag */
    }
   };
#endif /* TEST_AES_CCMS_AUTH_FORWARD */

#if TEST_AES_CCMS_AUTH_INVERSE
   aes_ccms_auth_forward_suite_t aes_ccms_auth_inverse_suite[] = {
    {
        {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF}, /* key */
        8, /* tag len */
        {0xAC, 0xDE, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x02}, /* nonce */
        { 0x08, 0xD0, 0x84, 0x21, 0x43, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xDE, 0xAC, 0x02, 0x05, 0x00,
         0x00, 0x00, 0x55, 0xCF, 0x00, 0x00, 0x51, 0x52, 0x53, 0x54}, /* a vector */
        { 0x22, 0x3B, 0xC1, 0xEC, 0x84, 0x1A, 0xB5, 0x53 }, /* c vector*/
        26, /* len_a */
        8, /* len_c */
        2, /* CCM L */
        { 0x22, 0x3B, 0xC1, 0xEC, 0x84, 0x1A, 0xB5, 0x53 } /* expected tag */
    }
};
#endif /* TEST_AES_CCMS_AUTH_INVERSE */

#if TEST_ECDSA_VERIFY
   ecdsa_verify_suite_t ecdsa_verify_suite[] =
   {
      {
        {
          SECP256R1,    //group info (name, group size, prime, order, a, b, gx, gy)
          { 0x5366B1AB, 0x0F1DBF46, 0xB0C8D3C4, 0xDB755B6F, 0xB9BF9243, 0xE644A8BE, 0x55159A59, 0x6F9E52A6, 0x0, 0x0, 0x0, 0x0 },  // s
          8,    // s_lenght
          { 0xC3B4035F, 0x515AD0A6, 0xBF375DCA, 0x0CC1E997, 0x7F54FDCD, 0x04D3FECA, 0xB9E396B9, 0x515C3D6E, 0x0, 0x0, 0x0, 0x0 },  // r
          8,    // r_lenght
          { 0x65637572, 0x20612073, 0x68206F66, 0x20686173, 0x69732061, 0x68697320, 0x6F2C2054, 0x48616C6C, 0x0, 0x0, 0x0, 0x0 },  // hash
          8,    // h_lenght
          {
              { 0x5fa58f52, 0xe47cfbf2, 0x300c28c5, 0x6375ba10, 0x62684e91, 0xda0a9a8f, 0xf9f2ed29, 0x36dfe2c6, 0x0, 0x0, 0x0, 0x0 },  // Qx
              { 0xc772f829, 0x4fabc36f, 0x09daed0b, 0xe93f9872, 0x35a7cfab, 0x5a3c7869, 0xde1ab878, 0x71a0d4fc, 0x0, 0x0, 0x0, 0x0 },  // Qy
          }
        }
      },
   };
#endif /* TEST_ECDSA_VERIFY */

   board_init();

#if TEST_AES_ECB
   if (run_aes_ecb_suite(aes_ecb_suite, sizeof(aes_ecb_suite)/sizeof(aes_ecb_suite[0])) == E_FAIL) {
      fail++;
   }
#endif /* TEST_AES_ECB */

#if TEST_AES_CCMS_ENC
   if (run_aes_ccms_enc_suite(aes_ccms_enc_suite, sizeof(aes_ccms_enc_suite)/sizeof(aes_ccms_enc_suite[0])) == E_FAIL) {
      fail++;
   }
#endif /* TEST_AES_CCMS_ENC */

#if TEST_AES_CCMS_DEC
   if (run_aes_ccms_dec_suite(aes_ccms_dec_suite, sizeof(aes_ccms_dec_suite)/sizeof(aes_ccms_dec_suite[0])) == E_FAIL) {
      fail++;
   }
#endif /* TEST_AES_CCMS_DEC */

#if TEST_AES_CCMS_AUTH_FORWARD
   if (run_aes_ccms_auth_forward_suite(aes_ccms_auth_forward_suite,
            sizeof(aes_ccms_auth_forward_suite)/sizeof(aes_ccms_auth_forward_suite[0])) == E_FAIL) {
      fail++;
   }
#endif /* TEST_AES_CCMS_AUTH_FORWARD */

#if TEST_AES_CCMS_AUTH_INVERSE
   if (run_aes_ccms_auth_inverse_suite(aes_ccms_auth_inverse_suite,
            sizeof(aes_ccms_auth_inverse_suite)/sizeof(aes_ccms_auth_inverse_suite[0])) == E_FAIL) {
      fail++;
   }
#endif /* TEST_AES_CCMS_AUTH_INVERSE */

#if TEST_ECDSA_VERIFY
   if (run_ecdsa_verify_suite(ecdsa_verify_suite,
            sizeof(ecdsa_verify_suite)/sizeof(ecdsa_verify_suite[0])) == E_FAIL) {
      fail++;
   }
#endif /* TEST_ECDSA_VERIFY */

#if TEST_BENCHMARK_CCMS

#define A_LEN 30
#define M_LEN 91
#define TAG_LEN 4
#define L 2

   uint8_t a[A_LEN];
   uint8_t m[M_LEN + TAG_LEN];
   uint8_t nonce[] = { 0x00, 0x00, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x05 };
   uint8_t key[16] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
   uint8_t len_m = M_LEN;
   uint8_t ret;

   memset(a, 0xfe, A_LEN);
   memset(m, 0xab, M_LEN);

   PORT_TIMER_WIDTH time1 = 0;
   PORT_TIMER_WIDTH time2 = 0;
   PORT_TIMER_WIDTH enc = 0;
   PORT_TIMER_WIDTH dec = 0;

   time1 = sctimer_readCounter();
   ret = cryptoengine_aes_ccms_enc(a,
           A_LEN,
           m,
           &len_m,
           nonce,
           L,
           key,
           TAG_LEN);
   time2 = sctimer_readCounter();

   if (ret == E_SUCCESS) {
      enc = time2 - time1;
   }
   else {
      fail++;
   }

   time1 = sctimer_readCounter();
   ret = cryptoengine_aes_ccms_dec(a,
           A_LEN,
           m,
           &len_m,
           nonce,
           L,
           key,
           TAG_LEN);
   time2 = sctimer_readCounter();

   if (ret == E_SUCCESS) {
      dec = time2 - time1;
   }
   else {
      fail++;
   }

   time1 = enc + dec; // to avoid compiler warnings
#endif /* TEST_BENCHMARK_CCMS */

   return hang(fail);
}

