/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:38.255700.
*/
#include "openwsn_obj.h"
#include "IEEE802154_obj.h"
#include "idmanager_obj.h"
#include "security.h"
#include "TI_aes.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

uint8_t T[16];
uint8_t MACTag[16];
uint8_t U[16];
uint8_t W[4];

uint8_t authData[148];
uint8_t authDataLength;

uint8_t length;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void CCMstar(OpenMote*             self,
			 OpenQueueEntry_t*     pkt,
		     unsigned long long int key);

void Input_Transformation(uint8_t* payload,
						  uint8_t  length);

void Auth_Transformation(uint8_t 			    length,
		                 unsigned long long int key,
		                 bool	  			    encOrDec,
		                 uint8_t				secLev);

void Encr_Transformation(uint8_t* 				payload,
						 uint8_t   				length,
		                 unsigned long long int key,
		                 uint8_t*  				T,
		                 bool                   cipher,
		                 uint8_t   				secLevel);

void CCMstarInverse(OpenMote*              self,
					OpenQueueEntry_t* 	   pkt,
		            unsigned long long int key);

void decr_Transformation(uint8_t* 			    cipherData,
		                 uint8_t                length,
						 uint8_t  				authenticationLength,
						 unsigned long long int key,
						 uint8_t 				secLev);

bool auth_checking(uint8_t* 			  ciphertext,
		           uint8_t  			  length,
		           unsigned long long int key,
		           uint8_t				  secLev);

//----------------------------------------------------------------------
