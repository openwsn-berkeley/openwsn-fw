#include "IEEE802154.h"
#include "idmanager.h"
#include "security.h"
#include "TI_aes.h"
#include "packetfunctions.h"
#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void CCMstar(OpenQueueEntry_t*     pkt,
		     uint8_t* 			   key,
		     uint8_t*			   nonce);

void Input_Transformation(uint8_t* payload,
		                  uint8_t  length,
		                  uint8_t  authDataLength,
		                  uint8_t* authData,
		                  uint8_t* aData);

void Auth_Transformation(uint8_t 			    length,
		                 uint8_t* 				key,
		                 uint8_t				authentication_length,
		                 uint8_t*				nonce,
		                 uint8_t				authDataLength,
		                 uint8_t*				MACTag,
		                 uint8_t*               authData);

void Encr_Transformation(uint8_t* 				payload,
						 uint8_t   				length,
		                 uint8_t* 				key,
		                 uint8_t*  				T,
		                 uint8_t   				secLevel,
		                 uint8_t   				authentication_length,
		                 uint8_t*				nonce,
		                 uint8_t*				CipherText);

void CCMstarInverse(OpenQueueEntry_t* 	   pkt,
		            uint8_t* 			   key,
		            uint8_t*			   nonce);

void decr_Transformation(uint8_t* 			    cipherData,
		                 uint8_t                length,
						 uint8_t  				authenticationLength,
						 uint8_t* 				key,
						 uint8_t 				secLev,
						 uint8_t*				nonce,
						 uint8_t*				CipherText,
						 uint8_t*               T_reconstructed);

bool auth_checking(uint8_t* 			  ciphertext,
		           uint8_t  			  length,
		           uint8_t*				  key,
		           uint8_t				  authentication_length,
		           uint8_t*				  nonce,
		           uint8_t*				  CipherText,
		           uint8_t*               T_reconstructed,
		           uint8_t*				  MACTag,
		           uint8_t* 			  authData,
		           uint8_t* aData);
//----------------------------------------------------------------------
