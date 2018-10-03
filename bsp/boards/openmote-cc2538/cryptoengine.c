/**
\brief Crypto engine implementation for OpenMote-CC2538
  
\author Malisa Vucinic <malishav@gmail.com>, March 2015.
\author Timothy Claeys <timothy.claeys@gmail.com>, October 2018.
*/
#include <stdint.h>

#include <headers/hw_sys_ctrl.h>

#include <source/sys_ctrl.h>
#include <source/aes.h>
#include <source/ccm.h>
#include <source/pka.h>

#include "cryptoengine.h"

#define DEFAULT_KEY_AREA KEY_AREA_0


//=========================== prototypes ======================================

static owerror_t load_key(uint8_t key[16], uint8_t* /* out */ key_location);

//=========================== public ==========================================

owerror_t cryptoengine_init(void) {
   //
   // Enable AES peripheral
   //
   SysCtrlPeripheralReset(SYS_CTRL_PERIPH_AES);
   SysCtrlPeripheralReset(SYS_CTRL_PERIPH_PKA);
   SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_AES);
   SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_PKA);
   return E_SUCCESS;
}

owerror_t cryptoengine_aes_ccms_enc(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {

   bool encrypt;
   uint8_t key_location;
  
   encrypt = *len_m > 0 ? true : false;

   if(load_key(key, &key_location) == E_SUCCESS) {
      if(CCMAuthEncryptStart(encrypt,
                              len_mac,
                              nonce,
                              m,
                              (uint16_t) *len_m,
                              a,
                              (uint16_t) len_a,
                              key_location,
                              &m[*len_m],
                              l,
                              /* polling */ 0) == AES_SUCCESS) {

         do {
            ASM_NOP;
         } while(CCMAuthEncryptCheckResult() == 0);
        
         if(CCMAuthEncryptGetResult(len_mac, 
                                    (uint16_t) *len_m,
                                    &m[*len_m]) == AES_SUCCESS) {

            *len_m += len_mac;
            return E_SUCCESS;
         }
      }
   }

   return E_FAIL;
}

owerror_t cryptoengine_aes_ccms_dec(uint8_t* a,
         uint8_t len_a,
         uint8_t* m,
         uint8_t* len_m,
         uint8_t* nonce,
         uint8_t l,
         uint8_t key[16],
         uint8_t len_mac) {

   bool decrypt;
   uint8_t key_location;
   uint8_t tag[CBC_MAX_MAC_SIZE];
  
   decrypt = *len_m - len_mac > 0 ? true : false;

   if(load_key(key, &key_location) == E_SUCCESS) {
      if(CCMInvAuthDecryptStart(decrypt,
                              len_mac,
                              nonce,
                              m,
                              (uint16_t) *len_m,
                              a,
                              (uint16_t) len_a,
                              key_location,
                              tag,
                              l,
                              /* polling */ 0) == AES_SUCCESS) {

         do {
            ASM_NOP;
         } while(CCMInvAuthDecryptCheckResult() == 0);
       
         if(CCMInvAuthDecryptGetResult(len_mac, 
                                       m,
                                       (uint16_t) *len_m,
                                       tag) == AES_SUCCESS) {

            *len_m -= len_mac;
            return E_SUCCESS;
         }
      }
   }
   return E_FAIL;
}

owerror_t cryptoengine_aes_ecb_enc(uint8_t* buffer, uint8_t* key) {
   uint8_t key_location;
   if(load_key(key, &key_location) == E_SUCCESS) {
      // Polling
      if(AESECBStart(buffer, buffer, key_location, 1, 0) == AES_SUCCESS) {
         do {
            ASM_NOP;
         } while(AESECBCheckResult() == 0);

         if(AESECBGetResult() == AES_SUCCESS) {
            return E_SUCCESS;
         }
      }
   }
   return E_FAIL;
}

owerror_t cryptoengine_load_group(uint8_t group, tECCCurveInfo* grp) {
    uint8_t ret;

    switch( group )
    {
        case SECP256R1:
            grp->name          = curve_name;
            grp->ui8Size       = 8;
            grp->pui32Prime    = nist_p_256_p;
            grp->pui32N        = nist_p_256_n;
            grp->pui32A        = nist_p_256_a;
            grp->pui32B        = nist_p_256_b;
            grp->pui32Gx       = nist_p_256_x;
            grp->pui32Gy       = nist_p_256_y;
            ret = E_SUCCESS;
            break;
        default:
            ret = E_FAIL;
    }

    return ( ret );
}


owerror_t cryptoengine_ecdsa_verify(
    tECCCurveInfo* grp,
    tECPt* Q,
    uint32_t* s,
    uint8_t s_length,
    uint32_t* r,
    uint8_t r_length,
    uint32_t * hash,
    uint8_t h_length)
{
    uint8_t result;
    uint32_t s_inv[grp->ui8Size];
    uint32_t resultLocation;
    uint32_t u1[ECC_MAX_MUL_SIZE];
    uint32_t u2[ECC_MAX_MUL_SIZE];
    uint32_t u1_length = ECC_MAX_MUL_SIZE, u2_length = ECC_MAX_MUL_SIZE;

    tECPt P1;
    uint32_t p1_x[grp->ui8Size];
    uint32_t p1_y[grp->ui8Size];
    P1.pui32X = p1_x;
    P1.pui32Y = p1_y;

    tECPt P2;
    uint32_t p2_x[grp->ui8Size];
    uint32_t p2_y[grp->ui8Size];
    P2.pui32X = p2_x;
    P2.pui32Y = p2_y;

    tECPt G;
    G.pui32X = grp->pui32Gx;
    G.pui32Y = grp->pui32Gy;

    /* Invert s mod n */
    PKABigNumInvModStart( s, s_length, grp->pui32N, grp->ui8Size, &resultLocation );

    do{
        ASM_NOP;
    } while (PKABigNumInvModGetResult( s_inv, grp->ui8Size, resultLocation ) == PKA_STATUS_OPERATION_INPRG);

    /* u1 = s_inv * hash mod ord */
    PKABigNumMultiplyStart( s_inv, grp->ui8Size, hash, h_length, &resultLocation );

    //u1_length gets the appropriate length after the operation completes
    do{
        ASM_NOP;
    } while (PKABigNumMultGetResult( u1, &u1_length, resultLocation ) == PKA_STATUS_OPERATION_INPRG);

    PKABigNumModStart( u1, (uint8_t)u1_length, grp->pui32N, grp->ui8Size, &resultLocation );

    do{
        ASM_NOP;
    } while (PKABigNumModGetResult( u1, grp->ui8Size, resultLocation ) == PKA_STATUS_OPERATION_INPRG);

    /* u2 = s_inv * r mod ord */
    PKABigNumMultiplyStart( s_inv, grp->ui8Size, r, r_length, &resultLocation );

    //u2_length gets the appropriate length after the operation completes
    do{
        ASM_NOP;
    } while (PKABigNumMultGetResult( u2, &u2_length, resultLocation ) == PKA_STATUS_OPERATION_INPRG);

    PKABigNumModStart( u2, (uint8_t)u2_length, grp->pui32N, grp->ui8Size, &resultLocation );

    do{
        ASM_NOP;
    } while (PKABigNumModGetResult( u2, grp->ui8Size, resultLocation ) == PKA_STATUS_OPERATION_INPRG);

    /* p1 = u1 * G */
    PKAECCMultiplyStart( u1, &G, grp, &resultLocation );

    do{
        ASM_NOP;
    } while( PKAECCMultiplyGetResult( &P1, resultLocation ) == PKA_STATUS_OPERATION_INPRG );

    PKAECCMultiplyStart( u2, Q, grp, &resultLocation );

    do{
        ASM_NOP;
    } while( PKAECCMultiplyGetResult( &P2, resultLocation ) == PKA_STATUS_OPERATION_INPRG );

    /* P = p1 + p2 */
    PKAECCAddStart( &P1, &P2, grp, &resultLocation );

    do{
        ASM_NOP;
    } while( PKAECCAddGetResult( &P1, resultLocation ) == PKA_STATUS_OPERATION_INPRG );

    /* verify the signature */
    PKABigNumCmpStart( r, P1.pui32X, r_length );

    do{
        ASM_NOP;
    } while( PKABigNumCmpGetResult() == PKA_STATUS_OPERATION_INPRG );

    if ( PKABigNumCmpGetResult() == PKA_STATUS_SUCCESS ){
        result = E_SUCCESS;
    }
    else{
        result = E_FAIL;
    }
    return ( result );
}


//=========================== private ==========================================

/**
\brief On success, returns by reference the location in key RAM where the 
   new/existing key is stored.
*/
static owerror_t load_key(uint8_t key[16], uint8_t* /* out */ key_location) {
   static uint8_t loaded_key[16];
   
   if(memcmp(loaded_key, key, 16) != 0) {
      memcpy(loaded_key, key, 16);
      // Load the key in key RAM
      if(AESLoadKey(loaded_key, DEFAULT_KEY_AREA) != AES_SUCCESS) {
         return E_FAIL;
      }
   }
   *key_location = DEFAULT_KEY_AREA;
   return E_SUCCESS;
}

