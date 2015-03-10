/**
\brief CCMstar procedure

\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, July 2014.
*/

#include "ccmstar.h"

void CCMstar(OpenQueueEntry_t* 		pkt,
		     uint8_t* 				key,
		     uint8_t*			    nonce){

	uint8_t payloadToEncrypt[128];
	uint8_t CipherText[128];
	uint8_t authData[128];
	uint8_t T[16];
	uint8_t T_temp[16];

	memset(&authData[0], 0, sizeof(authData));
	uint8_t authDataLength = 0;

	memset(&T[0], 0, sizeof(T));
	memset(&T_temp[0], 0, sizeof(T_temp));

	//copy the payload to be encrypted
	memset(&payloadToEncrypt, 0,
			sizeof(payloadToEncrypt));

	memcpy(&payloadToEncrypt[0],
			&pkt->l2_payload[0],
			pkt->l2_length);

	//calculate AuthData
	Input_Transformation(payloadToEncrypt,
						 pkt->l2_length,
			             authDataLength,
			             authData,
			             pkt->aData);

	//calculate the authentication tag
	Auth_Transformation(pkt->l2_length,
			            key,
			            0,
			            pkt->l2_securityLevel,
			            pkt->l2_authenticationLength,
			            nonce,
			            authDataLength,
			            T,
			            authData);//unused here

	//encrypt the payload
	Encr_Transformation(payloadToEncrypt,
						pkt->l2_length,
			            key,
			            T,
			            0,
			            pkt->l2_securityLevel,
			            pkt->l2_authenticationLength,
			            nonce,
			            CipherText,
			            T_temp);

	//reserve space for the authentication tag
	packetfunctions_reserveFooterSize(pkt,pkt->l2_authenticationLength);

	//copy the encrypted payload and the authentication tag
	memcpy(&pkt->l2_payload[0],
			&CipherText[0],
			pkt->l2_length+pkt->l2_authenticationLength);

}


void Input_Transformation(uint8_t* payload,
		                  uint8_t  length,
		                  uint8_t authDataLength,
		                  uint8_t* authData,
		                  uint8_t* aData){

	// La || a || 0 || m || 0

    authDataLength = aData[0];
    memcpy(&authData[0],
    		&aData[0],
    		authDataLength);

	//PlainText data
	memcpy(&authData[authDataLength],
		   &payload[0],
		   authDataLength+length);

	authDataLength = authDataLength+length;

	//padding
	uint8_t count;
	count = 16-(authDataLength-((authDataLength/16)*16));
	authDataLength = authDataLength+count;

}

void Auth_Transformation(uint8_t 				length,
						 uint8_t* 				key,
						 bool 					encOrDec,
						 uint8_t 				secLev,
						 uint8_t				authentication_length,
						 uint8_t*				nonce,
						 uint8_t 				authDataLength,
						 uint8_t*               MACTag,
						 uint8_t*               authData){

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
	memcpy(&B[1],
		   &nonce[0],13);

	uint16_t auxlen;
	auxlen = length;

	B[14] = auxlen;
	B[15] = auxlen <<8;

	//initialize X, for me IN
	memset(&in[0], 0, sizeof(in));

	//IV
	for(i=0;i<16;i++){
		in[i] = in[i]^B[i];
	}

	//copy the key
	memset(&Key[0], 0, sizeof(Key));
	memcpy(&Key[0], &key[0], sizeof(key));

	//initialize out
	for(i=0;i<16;i++){
		out[i] = in[i];
	}
//	memcpy(&out[0], &in[0], 16);

	//determine the upper bound
	uint8_t limit;
	limit = (authDataLength/16)+1;

	//crypto block
	uint8_t j;
	for(i=0;i<limit;i++){
		for(j=i*16;j<(i*16+16);j++){
			B[j-16*i] = authData[j];
			in[j-16*i] = B[j-16*i]^in[j-16*i];
		}
		aes_encrypt(in,out,Key);

		memcpy(&in[0], &out[0], 16);
	}

	//store the authentication tag
	for(i=0;i<authentication_length;i++){
		MACTag[i] = out[i];
	}
//	memcpy(&MACTag[0], &out[0], authentication_length);

}

void Encr_Transformation(uint8_t*  				payload,
						 uint8_t   				length,
		                 uint8_t* 				key,
		                 uint8_t*  				Ta,
		                 bool      				cipher,
		                 uint8_t   				secLevel,
		                 uint8_t   				authentication_length,
		                 uint8_t*				nonce,
		                 uint8_t*				CipherText,
		                 uint8_t*               T_reconstructed){

	uint8_t PlainTextData[16];
	uint8_t i;
	uint16_t cnt;
	uint8_t in[16], out[16];
	uint8_t W[16];
	uint8_t U[16];

	memset(&W[0], 0, sizeof(W));
	memset(&U[0], 0, sizeof(U));

	//copy the key
	memset(&Key[0], 0, sizeof(Key));
	memcpy(&Key[0], &key[0], sizeof(key));

	cnt = 0;
	//initialization of CipherText
	for(i=0;i<128;i++){
		CipherText[i] = 0;
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
			U[i] = out[i] ^ Ta[i];
		}
	}

	if(cipher==1){
		for(i=0;i<authentication_length;i++){
			W[i] = out[i] ^ Ta[i];
				}
	}

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
			CipherText[j+16*i] = out[j];
		}
	}


	uint8_t count;
	count = 16-(length-((length/16)*16));
	for(i=(count-1);i>(length-1);i--){
		CipherText[i] = 0;
	}

	if(authentication_length !=0){
		if(cipher==0){
			for(i=0;i<authentication_length;i++){
				CipherText[length+i] = U[i];
			}
		}

		if(cipher==1){
			for(i=0;i<authentication_length;i++){
				T_reconstructed[i] = W[i];
			}
		}
	}

}

void CCMstarInverse(OpenQueueEntry_t* 	   pkt,
		            uint8_t* 			   key,
		            uint8_t*			   nonce){

	uint8_t payloadToDecrypt[128];
	uint8_t CipherText[128];
	uint8_t T_reconstructed[16];
	uint8_t MACTag[16];
    uint8_t authData[128];

	memset(&authData[0], 0, sizeof(authData));
	memset(&T_reconstructed[0], 0, sizeof(T_reconstructed));

	if(pkt->length == 0) return;

	memset(&payloadToDecrypt[0], 0,
			sizeof(payloadToDecrypt));

	memcpy(&payloadToDecrypt[0],
			&pkt->payload[0],
			pkt->length);

	memset(&MACTag[0],0, sizeof(MACTag));

	decr_Transformation(payloadToDecrypt,
						pkt->length,
			            pkt->l2_authenticationLength,
			            key,
			            pkt->l2_securityLevel,
			            nonce,
			            CipherText,
			            T_reconstructed);

	uint8_t payload_length;
	payload_length = pkt->length - pkt->l2_authenticationLength;

	if(pkt->l2_securityLevel != 4){
		if(auth_checking(payloadToDecrypt,//ccmstar_vars.payloadToEncrypt,
						 payload_length,
				         key,
				         pkt->l2_securityLevel,
				         pkt->l2_authenticationLength,
				         nonce,
				         CipherText,
				         T_reconstructed,
				         MACTag, authData,
				         pkt->aData) == FALSE){
			pkt->l2_toDiscard = 5;
		}
	}

	//copy the decrypted payload
	memcpy(&pkt->payload[0],
			&CipherText[0],
			pkt->length);

	if(pkt->l2_securityLevel != 4){
		packetfunctions_tossFooter(pkt,pkt->l2_authenticationLength);
	}

}

void decr_Transformation(uint8_t* 				cipherData,
		                 uint8_t 				length,
						 uint8_t 				authentication_length,
						 uint8_t* 				key,
						 uint8_t 				secLev,
						 uint8_t*				nonce,
						 uint8_t*               CipherText,
						 uint8_t*               T_reconstructed){

	uint8_t i;

	uint8_t U_rec[16];
	memset(&U_rec[0], 0, sizeof(U_rec));

	//U is the final part of the payload
	for(i=0;i<authentication_length;i++){
		U_rec[i] = cipherData[length-authentication_length+i];
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
			            U_rec,
			            1,
			            secLev,
			            authentication_length,
			            nonce,
			            CipherText,
			            T_reconstructed);

	//copy the decrypted payload
	for(i=0;i<newlen;i++){
		cipherData[i] = CipherText[i];
	}

}

bool auth_checking(uint8_t* ciphertext,
		           uint8_t  length,
		           uint8_t* key,
		           uint8_t 	secLev,
		           uint8_t	authentication_length,
		           uint8_t*	nonce,
		           uint8_t*  CipherText,
		           uint8_t* T_reconstructed,
		           uint8_t* MACTag,
		           uint8_t* authData,
		           uint8_t* aData){

	uint8_t messageDecr[128];
	uint8_t authDataLength = 0;

	//copy the decrypted payload in temporal vector
	uint8_t i;
	for(i=0;i<128;i++){
		messageDecr[i] = 0;
	}
	for(i=0;i<length;i++){
		messageDecr[i] = CipherText[i];
	}

	//recalculate AuthData
	Input_Transformation(messageDecr,
			             length,
			             authDataLength,
			             authData,
			             aData);

	//recalculate the authentication tag
	Auth_Transformation(length,
			            key,
			            1,
			            secLev,
			            authentication_length,
			            nonce, authDataLength, MACTag, authData);

	//verify the tag
	for(i=0;i<16;i++){
		if(T_reconstructed[i] == MACTag[i]){
		}
		else{
			return FALSE;
		}
	}

	return TRUE;
}


//---------------------------------------------------------------------------
