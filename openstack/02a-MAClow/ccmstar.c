/**
\brief CCMstar procedure

\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, July 2014.
*/

#include "ccmstar.h"

ccmstar_vars_t ccmstar_vars;

void CCMstar(OpenQueueEntry_t* 		pkt,
		     uint8_t 				key[16],
		     uint8_t*			    nonce){

	uint8_t i;
	//store length parameter to be used
	ccmstar_vars.length = pkt->l2_length;

	//copy the payload to be encrypted
	for(i=0;i<128;i++){
		ccmstar_vars.payloadToEncrypt[i] = 0;
	}
	for(i=0;i<ccmstar_vars.length;i++){
		ccmstar_vars.payloadToEncrypt[i] = pkt->l2_payload[i];
	}

	//calculate AuthData
	Input_Transformation(ccmstar_vars.payloadToEncrypt,
			             ccmstar_vars.length,
			             pkt->l2_authenticationLength,0);

	//calculate the authentication tag
	Auth_Transformation(ccmstar_vars.length,
			            key,
			            0,
			            pkt->l2_securityLevel,
			            pkt->l2_authenticationLength,
			            nonce);

	//encrypt the payload
	Encr_Transformation(ccmstar_vars.payloadToEncrypt,
			            ccmstar_vars.length,
			            key,
			            ccmstar_vars.T,
			            0,
			            pkt->l2_securityLevel,
			            pkt->l2_authenticationLength,
			            nonce);
	//reserve space for the authentication tag
	packetfunctions_reserveFooterSize(pkt,pkt->l2_authenticationLength);

	//copy the encrypted payload and the authentication tag
	for(i=0;i<ccmstar_vars.length+pkt->l2_authenticationLength;i++){
		pkt->l2_payload[i] = ccmstar_vars.CipherText[i];
	}

}


void Input_Transformation(uint8_t* payload,
		                  uint8_t  length,
		                  uint8_t  authentication_length,
		                  uint8_t  encOrdec){

	//initialize AuthData

	uint8_t i,l;
	for(i=0;i<148;i++){
		ccmstar_vars.authData[i] = 0;
	}
	ccmstar_vars.authDataLength = 0;

	uint8_t La[4];
	for(i=0;i<4;i++){
		La[i] = 0;
	}

	//no authentication
	if(authentication_length == 0){
		l = 0;
	}
	//authentication field of 32 bit
	if(authentication_length == 4){
		La[3] = authentication_length;
		l = authentication_length;
	}

	//authentication field of 64 bit
	if(authentication_length == 8){
		La[3] = 0xFF;
		La[2] = 0xFE;
		La[1] = 0;
		La[0] = authentication_length;
		l = authentication_length + 2;
	}

	//authentication field of 128 bit
	if(authentication_length == 16){
		La[3] = 0xFF;
		La[2] = 0xFF;
		La[1] = 0;
		La[0] = authentication_length;
		l = authentication_length + 2;
	}

	for(i=0; i<4;i++){
		ccmstar_vars.authData[i] = La[i];
	}

	ccmstar_vars.authData[i] = authentication_length;
	ccmstar_vars.authDataLength = i+1;

	if(encOrdec == 1){
		 length = length - authentication_length;
	}

	//the payload is part of data to be authenticated, with a length multiple of 16
	for(i=0;i< ccmstar_vars.authDataLength+length;i++){
		ccmstar_vars.authData[ccmstar_vars.authDataLength+i] = payload[i];
	}

	ccmstar_vars.authDataLength = ccmstar_vars.authDataLength+length;

	uint8_t count;
	count = 16-(ccmstar_vars.authDataLength-((ccmstar_vars.authDataLength/16)*16));

	ccmstar_vars.authDataLength = ccmstar_vars.authDataLength+count;

}

void Auth_Transformation(uint8_t 				length,
						 uint8_t 				key[16],
						 bool 					encOrDec,
						 uint8_t 				secLev,
						 uint8_t				authentication_length,
						 uint8_t*				nonce){

	if(secLev == 4) return; /*in case Security Level is 4, packet is only encrypted
							and not authenticated */

	uint8_t B[16];

	uint8_t in[16],out[16];
	uint8_t i;

	//determine Flags Field
	B[0] = 0;
	B[0] = B[0] <<1;//1b reserved
	if(length == 0){
		B[0]= B[0] <<1;
	}
	else{
		B[0] |= 1 <<1;
	}

	if(authentication_length == 0){
		B[0]= B[0] <<2;
	}
	else{
		B[0] = B[0] <<2;
		B[0] |= ((authentication_length-2)/2)&0x07;
	}


	B[0] = B[0] <<3;
	B[0] |= length &0x07;

	//determine B0 fields
	for(i=0;i<13;i++){
		B[i+1] = nonce[i];
	}

	//initialize T
	if(encOrDec == 0){
		for(i=0;i<16;i++){
			ccmstar_vars.T[i] = 0;
		}
	}

	uint16_t auxlen;
	auxlen = length;

	B[14] = auxlen;
	B[15] = auxlen <<8;

	//initialize X, for me IN
	for(i=0;i<16;i++){
		in[i] = 0;
	}

	//IV
	for(i=0;i<16;i++){
		in[i] = in[i]^B[i];
	}

//	//Key Expansion phase, before the crypto block
	//TODO
	for(i=0;i<16;i++){
			Key[i] = key[i];
		}
	for(i=16;i<64;i++){
		Key[i] = 0;
	}

	//initialize out
	for(i=0;i<16;i++){
		out[i] = in[i];
	}

	//determine the upper bound
	uint8_t limit;
	limit = (ccmstar_vars.authDataLength/16)+1;

	//crypto block
	uint8_t j;
	for(i=0;i<limit;i++){
		for(j=i*16;j<(i*16+16);j++){
			B[j-16*i] = ccmstar_vars.authData[j];
			in[j-16*i] = B[j-16*i]^in[j-16*i];
		}

		aes_encrypt(in,out,Key);

		for(j=0;j<16;j++){
			in[j] = out[j];
		}
	}

	//store the authentication tag
	if(encOrDec==0){
		for(i=0;i<authentication_length;i++){
			ccmstar_vars.T[i] = out[i];
		}
	}
	if(encOrDec==1){
		for(i=0;i<authentication_length;i++){
			ccmstar_vars.MACTag[i] = out[i];
		}
	}

}

void Encr_Transformation(uint8_t*  				payload,
						 uint8_t   				length,
		                 uint8_t 				key[16],
		                 uint8_t*  				Ta,
		                 bool      				cipher,
		                 uint8_t   				secLevel,
		                 uint8_t   				authentication_length,
/*CONTROLLA*/		                 uint8_t*				nonce){

	uint8_t PlainTextData[16];
	uint8_t i;
	uint16_t cnt;
	uint8_t in[16], out[16];

//	//Key Expansion phase, before the crypto block
	//TODO
	for(i=0;i<16;i++){
		Key[i] = key[i];
	}
	for(i=16;i<64;i++){
		Key[i] = 0;
	}

	cnt = 0;
	//initialization of CipherText
	for(i=0;i<128;i++){
		ccmstar_vars.CipherText[i] = 0;
	}

	//Ai
	in[0] = 0;

	in[0] |= length &0x07 ; //flags field
	for(i=0;i<13;i++){
		in[i+1] = nonce[i];
	}

	in[14] = cnt;
	in[15] = cnt << 8;

	for(i=0;i<16;i++){
		out[i] = in[i];
	}

	// auth tag
	if(secLevel > 3){
		aes_encrypt(in,out,Key);
	}

	if(cipher==0){
		for(i=0;i<authentication_length;i++){
			ccmstar_vars.U[i] = out[i] ^ Ta[i];
		}
	}

	if(cipher==1){
		for(i=0;i<authentication_length;i++){
			ccmstar_vars.W[i] = out[i] ^ Ta[i];
				}
	}

//	if(cipher == 1){
//	   openserial_printInfo(COMPONENT_SECURITY,ERR_SECURITY,
//							 (errorparameter_t)payload[0],
//							 (errorparameter_t)payload[1]);
//	}

	uint8_t j;
	uint8_t limit;
	limit = ((length/16)+1);

	for(i=0;i<limit;i++){
		for(j=i*16;j<i*16+16;j++){
			PlainTextData[j-16*i] = payload[j];
		}
		//update Nonce
		in[14] = cnt;
		in[15] = cnt << 8;

		uint8_t jj;
		for(jj=0;jj<16;jj++){
			out[jj] = in[jj];
		}

		if(secLevel > 3){
			aes_encrypt(in,out,Key);
		}

		cnt++;
		for(j=0;j<16;j++){
			out[j] = out[j] ^ PlainTextData[j];
			ccmstar_vars.CipherText[j+16*i] = out[j];
		}
	}


	uint8_t count;
	count = 16-(length-((length/16)*16));
	for(i=(count-1);i>(length-1);i--){
		ccmstar_vars.CipherText[i] = 0;
	}

	if(authentication_length !=0){
		if(cipher==0){
			for(i=0;i<authentication_length;i++){
				ccmstar_vars.CipherText[length+i] = ccmstar_vars.U[i];
			}
		}

		if(cipher==1){
			for(i=0;i<authentication_length;i++){
				ccmstar_vars.T[i] = ccmstar_vars.W[i];
			}
		}
	}

//	if(cipher == 0){
//	   openserial_printInfo(COMPONENT_SECURITY,ERR_SECURITY,
//							 (errorparameter_t)ccmstar_vars.CipherText[0],
//							 (errorparameter_t)ccmstar_vars.CipherText[1]);
//	}

}

void CCMstarInverse(OpenQueueEntry_t* 	   pkt,
		            uint8_t 			   key[16],
		            uint8_t*			   nonce){

	uint8_t i;
	ccmstar_vars.length = pkt->length;
	if(ccmstar_vars.length == 0) return;

	for(i=0;i<128;i++){
//		ccmstar_vars.payloadToEncrypt[i] = 0;
		ccmstar_vars.payloadToDecrypt[i] = 0;
	}

	for(i=0;i<ccmstar_vars.length;i++){
//			ccmstar_vars.payloadToEncrypt[i] = pkt->payload[i];
		ccmstar_vars.payloadToDecrypt[i] = pkt->payload[i];
		}

	for(i=0;i<16;i++){
		ccmstar_vars.MACTag[i] = 0;
	}

	decr_Transformation(ccmstar_vars.payloadToDecrypt,//ccmstar_vars.payloadToEncrypt,
			            ccmstar_vars.length,
			            pkt->l2_authenticationLength,
			            key,
			            pkt->l2_securityLevel,
			            nonce);

	if(pkt->l2_securityLevel != 4){
		if(auth_checking(ccmstar_vars.payloadToDecrypt,//ccmstar_vars.payloadToEncrypt,
				         ccmstar_vars.length,
				         key,
				         pkt->l2_securityLevel,
				         pkt->l2_authenticationLength,
				         nonce) == FALSE){
			pkt->l2_toDiscard = 5;
		}
	}

	//copy the decrypted payload
	for(i=0;i<ccmstar_vars.length;i++){
		pkt->payload[i] = ccmstar_vars.CipherText[i];
	}

	if(pkt->l2_securityLevel != 4){
		packetfunctions_tossFooter(pkt,pkt->l2_authenticationLength);
	}

}

void decr_Transformation(uint8_t* 				cipherData,
		                 uint8_t 				length,
						 uint8_t 				authentication_length,
						 uint8_t 				key[16],
						 uint8_t 				secLev,
						 uint8_t*				nonce){

	uint8_t i;
	//initialize U and T
	for(i=0;i<16;i++){
		ccmstar_vars.U[i] = 0;
		ccmstar_vars.T[i] = 0;
	}

	//U is the final part of the payload
	for(i=0;i<authentication_length;i++){
		ccmstar_vars.U[i] = cipherData[length-authentication_length+i];
	}

	uint8_t newlen;
	newlen = length - authentication_length;

	//copy the encrypted payload in a temporal vector
	uint8_t CipherTextdec[130];
	for(i=0;i<130;i++){
		CipherTextdec[i] = 0;
	}
	for(i=0;i<newlen;i++){
		CipherTextdec[i] = cipherData[i];
	}

	//padding
	uint8_t count;
	count = 16-(newlen-((newlen/16)*16));
	for(i=0;i<count;i++){
		CipherTextdec[newlen+i] = 0;
	}

	//Decrypt the payload
	Encr_Transformation(CipherTextdec,
			            newlen,
			            key,
			            ccmstar_vars.U,
			            1,
			            secLev,
			            authentication_length,
			            nonce);

	//copy the decrypted payload
	for(i=0;i<newlen;i++){
		cipherData[i] = ccmstar_vars.CipherText[i];
	}

}

bool auth_checking(uint8_t* ciphertext,
		           uint8_t  length,
		           uint8_t 	key[16],
		           uint8_t 	secLev,
		           uint8_t	authentication_length,
		           uint8_t*	nonce){

	uint8_t messageDecr[128];

	//copy the decrypted payload in temporal vector
	uint8_t i;
	for(i=0;i<128;i++){
		messageDecr[i] = 0;
	}
	for(i=0;i<length-authentication_length;i++){
		messageDecr[i] = ccmstar_vars.CipherText[i];
	}

	//recalculate AuthData
	Input_Transformation(messageDecr,length,authentication_length,1);

	//recalculate the authentication tag
	Auth_Transformation(length-authentication_length,
			            key,
			            1,
			            secLev,
			            authentication_length,
			            nonce);

	//verify the tag
	for(i=0;i<16;i++){
		if(ccmstar_vars.T[i] == ccmstar_vars.MACTag[i]){
		}
		else{
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[0],
//									(errorparameter_t)nonce[1]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[2],
//									(errorparameter_t)nonce[3]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[4],
//									(errorparameter_t)nonce[5]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[6],
//									(errorparameter_t)nonce[7]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[8],
//									(errorparameter_t)nonce[9]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[10],
//									(errorparameter_t)nonce[11]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[12],
//									(errorparameter_t)nonce[13]);
//			  openserial_printInfo(COMPONENT_IEEE802154,ERR_SECURITY,
//									(errorparameter_t)nonce[14],
//									(errorparameter_t)nonce[15]);
			return FALSE;
		}
	}

	return TRUE;
}

//---------------------------------------------------------------------------
