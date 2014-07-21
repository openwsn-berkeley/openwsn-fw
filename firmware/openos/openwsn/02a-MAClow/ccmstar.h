#include "openwsn.h"
#include "IEEE802154.h"
#include "idmanager.h"
#include "security.h"
#include "TI_aes.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//uint8_t T[16];
//uint8_t MACTag[16];
//uint8_t U[16];
//uint8_t W[4];
//
//uint8_t authData[148];
//uint8_t authDataLength;
//
//uint8_t length;
//
//uint8_t payloadToEncrypt[128];
//uint8_t CipherText[128];

typedef struct{
	uint8_t T[16];
	uint8_t MACTag[16];
	uint8_t U[16];
	uint8_t W[4];
	uint8_t authData[148];
	uint8_t authDataLength;
	uint8_t length;
	uint8_t payloadToEncrypt[128];
	uint8_t CipherText[128];
}ccmstar_vars_t;

//=========================== prototypes ======================================

void CCMstar(OpenQueueEntry_t*     pkt,
		    unsigned long long int key);

void Input_Transformation(uint8_t* payload,
						  uint8_t  length,
						  uint8_t  authentication_length);

void Auth_Transformation(uint8_t 			    length,
		                 unsigned long long int key,
		                 bool	  			    encOrDec,
		                 uint8_t				secLev,
		                 uint8_t				authentication_length);

void Encr_Transformation(uint8_t* 				payload,
						 uint8_t   				length,
		                 unsigned long long int key,
		                 uint8_t*  				T,
		                 bool                   cipher,
		                 uint8_t   				secLevel,
		                 uint8_t   				authentication_length);

void CCMstarInverse(OpenQueueEntry_t* 	   pkt,
		            unsigned long long int key);

void decr_Transformation(uint8_t* 			    cipherData,
		                 uint8_t                length,
						 uint8_t  				authenticationLength,
						 unsigned long long int key,
						 uint8_t 				secLev);

bool auth_checking(uint8_t* 			  ciphertext,
		           uint8_t  			  length,
		           unsigned long long int key,
		           uint8_t				  secLev,
		           uint8_t				  authentication_length);

//----------------------------------------------------------------------
