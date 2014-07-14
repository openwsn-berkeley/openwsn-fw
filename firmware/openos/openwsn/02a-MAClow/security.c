/**
\brief General Security Operations

\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, July 2014.
*/

#include "security.h"

//=============================define==========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== admin ===========================================

void security_init(){

	//Setting UP Phase

	//MASTER KEY
	M_k = 249956789;


	//Initialization of Nonce String
	uint8_t i;
	for(i=0;i<13;i++){
		nonce[i] = 0;
	}

	//Initialization of the MAC Security Level Table
	for(i=0; i<2; i++){
		MacSecurityLevelTable.SecurityDescriptorEntry[i].FrameType = i;
		MacSecurityLevelTable.SecurityDescriptorEntry[i].CommandFrameIdentifier = 0;
		MacSecurityLevelTable.SecurityDescriptorEntry[i].DeviceOverrideSecurityMinimum = FALSE;
		MacSecurityLevelTable.SecurityDescriptorEntry[i].AllowedSecurityLevels = 7;
		MacSecurityLevelTable.SecurityDescriptorEntry[i].SecurityMinimum = 7;
	}

	//Initialization of MAC KEY TABLE
	for(i=0; i<MAXNUMKEYS;i++){
		MacKeyTable.KeyDescriptorElement[i].key = 0;
	}

	//Initialization of MAC DEVICE TABLE
	uint8_t j;
		for(i=0; i<MAXNUMNEIGHBORS; i++){
			for(j=0;j<8;j++){
				MacDeviceTable.DeviceDescriptorEntry[i].deviceAddress.addr_64b[j] = 0;
			}
		}


	//Initialization of Frame Counter
	m_macFrameCounter = 0;
	m_macFrameCounterMode = 0x04; //0x04 or 0x05

}

//=========================== public ==========================================

void security_outgoingFrame(OpenQueueEntry_t*   msg,
                            uint8_t             securityLevel,
                            uint8_t             keyIdMode,
                            open_addr_t*        keySource,
                            uint8_t          	keyIndex){

	uint8_t auxlen;
	uint8_t temp8b;
	uint8_t match;
	bool frameCounterSuppression;
	frameCounterSuppression = 0; //the frame counter is carried in the frame, otherwise 1;

	m_keyDescriptor* keypoint;

	if(m_macFrameCounter == (0xffffffffffffffff)){
		return;
	}

	//max length of MAC frames

	// length of authentication Tag
	authLengthChecking(securityLevel);
	msg->l2_authenticationLength = authlen;

	//length of auxiliary security header
	msg->l2_auxiliaryLength = auxLengthChecking(keyIdMode, frameCounterSuppression, m_macFrameCounterMode); //length of Key ID field


	if((msg->length+msg->l2_auxiliaryLength+msg->l2_authenticationLength+2) >= 130 ){ //2 bytes of CRC, 130 MaxPHYPacketSize
		return;
	}

	//check if SecurityLevel is not zero
	//in my impl. secLevel may not be zero if i'm here.
	/*if(securityLevel == ASH_SLF_TYPE_NOSEC){
		return;
	}*/

	open_addr_t* nextHop;
	nextHop = &msg->l2_nextORpreviousHop;

	//search for a key
	match = keyDescriptorLookup(keyIdMode,
								keySource,
								keyIndex,
								nextHop,
								(idmanager_getMyID(ADDR_PANID)),
								msg->l2_frameType);

	keypoint = &MacKeyTable.KeyDescriptorElement[match];

	if(match == 25){
//		openserial_printError(COMPONENT_RES,ERR_OK,
//							(errorparameter_t)msg->creator,
//							(errorparameter_t)400);
		return;
	}

	//start setting the Auxiliary Security Header
	if(keyIdMode !=0){//if the KeyIdMode is zero, keyIndex and KeySource are omitted
		temp8b = keyIndex; //key index field

		packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
		*((uint8_t*)(msg->payload)) = temp8b;
	}

	switch(msg->l2_keyIdMode){
	case 0: //no KeyIDMode field
	case 1:
		break;
	case 2: //keySource with 16b address
		packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
		*((uint8_t*)(msg->payload)) = keySource->addr_64b[6];

		packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
		*((uint8_t*)(msg->payload)) = keySource->addr_64b[7];
		break;
	case 3: //keySource with 64b address
		packetfunctions_writeAddress(msg,keySource,OW_LITTLE_ENDIAN);
		break;
	default:
		return;
	}

	//Frame Counter
	uint32_t temp;
	uint8_t i;

	uint8_t vectASN[5];

	if(frameCounterSuppression == 0){//the frame Counter is carried in the frame
		if(m_macFrameCounterMode == 0x04){ //it is a counter, as described in IEEE802.15.4
			temp = m_macFrameCounter;

			for(i=0;i<3;i++){
				packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
				*((uint8_t*)(msg->payload)) = temp;
				temp = temp >>8;
			}

			packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
			*((uint8_t*)(msg->payload)) = temp;
		} else{
			//here I have to insert the ASN
			ieee154e_getAsn(vectASN);//gets asn from mac layer.
			for(i=0;i<5;i++){
				packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
				*((uint8_t*)(msg->payload)) = vectASN[i];
			}

		}
	} //otherwise the frame counter is not in the frame


	//security control field
	packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));

	temp8b = 0;
	temp8b |= securityLevel << ASH_SCF_SECURITY_LEVEL;//3b
	temp8b |= keyIdMode << ASH_SCF_KEY_IDENTIFIER_MODE;//2b
	temp8b |= frameCounterSuppression << ASH_SCF_FRAME_CNT_MODE; //1b

	if(m_macFrameCounterMode == 0x04){
		temp8b |= 0 << ASH_SCF_FRAME_CNT_SIZE; //1b
	} else{
		temp8b |= 1 << ASH_SCF_FRAME_CNT_SIZE; //1b
	}

	temp8b |= 0 << 1;//1b reserved
	*((uint8_t*)(msg->payload)) = temp8b;

	//cryptographic block
	switch(keyIdMode){
	case 0:
	case 1:
		for(i=0; i<8; i++){
			nonce[i] = m_macDefaultKeySource.addr_64b[i];
			}
		break;
	case 2:
		for(i=0; i<2; i++){
			nonce[i] = keySource->addr_64b[6+i];
				}
		for(i=2; i<8; i++){
			nonce[i] = 0;
		}
		break;
	case 3:
		for(i=0; i<8; i++){
			 nonce[i] = keySource->addr_64b[i];
		}
		break;
	}

	if(m_macFrameCounterMode == 0x04){
		temp = m_macFrameCounter;

		nonce[8] = temp;
		nonce[9] = temp << 8;
		nonce[10] = temp <<16;
		nonce[11] = temp <<24;
		nonce[12] = securityLevel;
	} else{
		for(i=0;i<5;i++){
			nonce[8+i] = vectASN[i];
		}


	}

//		CRYPTO TEST
//		asn_t init;
//
//		if(msg->l2_frameType == IEEE154_TYPE_DATA){
//
//		uint8_t start[5];
//
//		ieee154e_getAsn(start);
//
//		init.bytes0and1 = start[0]+256*start[1];
//		init.bytes2and3 = start[2]+256*start[3];
//		init.byte4 = start[4];
//		}


	CCMstar(msg,keypoint->key);

//		if(msg->l2_frameType == IEEE154_TYPE_DATA){
//		uint16_t diff;
//		diff = ieee154e_asnDiff(&init);
//
//		openserial_printError(COMPONENT_RES,ERR_OK,
//							(errorparameter_t)diff,
//							(errorparameter_t)501);
//		}

	//h increment the Frame Counter and save.
	m_macFrameCounter++;

}

void retrieve_AuxiliarySecurityHeader(OpenQueueEntry_t*      msg,
                  	  	  	  	      ieee802154_header_iht* tempheader){

	//a check if security is enabled, for me it not useful. If I'm here, security is enabled.
//	if(tempheader->securityEnabled == TRUE){
//		msg->l2_security = IEEE154_SEC_YES_SECURITY;
//	}

	uint8_t frameCnt_Suppression;
	uint8_t frameCnt_Size;

	uint8_t temp8b;

	//b check if 802.15.4 header is valid
	//if the header is not valid, I'm not here..

	//c retrieve the Security Control field
	//1byte, Security Control Field

	temp8b = *((uint8_t*)(msg->payload)+tempheader->headerLength);

	msg->l2_securityLevel = (temp8b >> ASH_SCF_SECURITY_LEVEL)& 0x07;//3b

	authLengthChecking(msg->l2_securityLevel);
	msg->l2_authenticationLength = authlen;

	//retrieve the KeyIdMode field
	msg->l2_keyIdMode = (temp8b >> ASH_SCF_KEY_IDENTIFIER_MODE)& 0x03;//2b

	frameCnt_Suppression = (temp8b >> ASH_SCF_FRAME_CNT_MODE)& 0x01;//1b
	frameCnt_Size = (temp8b >> ASH_SCF_FRAME_CNT_SIZE)& 0x01;//1b

	tempheader->headerLength = tempheader->headerLength+1;

	//retrieve the FrameCounter field, if it is here, and control it is not in overflow

	//Frame Counter field, //l
	uint8_t temp,i;
	temp = 0;

	if(frameCnt_Suppression == 0){//the frame counter is here

		if(frameCnt_Size == 0){//the frame counter size is 4 bytes

			msg->l2_frameCounter = 0;
			for(i=0;i<3;i++){
				temp = *((uint8_t*)(msg->payload)+tempheader->headerLength);
				msg->l2_frameCounter |= temp;
				msg->l2_frameCounter = msg->l2_frameCounter <<8;
				tempheader->headerLength = tempheader->headerLength+1;
			}

			temp = *((uint8_t*)(msg->payload)+tempheader->headerLength);
			msg->l2_frameCounter |= temp;
			tempheader->headerLength = tempheader->headerLength+1;
		} else{ //the frame counter size is 5 bytes, because we have the ASN
			for(i=0;i<5;i++){
				msg->receivedASN[i] = *((uint8_t*)(msg->payload)+tempheader->headerLength);
				tempheader->headerLength = tempheader->headerLength+1;
			}
		}

		if(msg->l2_frameCounter == (0xffffffffffffffff)){
			msg->l2_toDiscard = TRUE;
			return; // frame counter overflow
			}
	}

   //retrieve the Key Identifier field
	switch(msg->l2_keyIdMode){
	case 0:
	case 1:
		break;
	case 2:
		packetfunctions_readAddress(
									((uint8_t*)(msg->payload)+tempheader->headerLength),
									ADDR_16B,
									&msg->l2_keySource,
									OW_LITTLE_ENDIAN);
		tempheader->headerLength = tempheader->headerLength+2;
		break;
	case 3:
		packetfunctions_readAddress(
							((uint8_t*)(msg->payload)+tempheader->headerLength),
							ADDR_64B,
							&msg->l2_keySource,
							OW_LITTLE_ENDIAN);
		tempheader->headerLength = tempheader->headerLength+8;
		break;
	default:
		msg->l2_toDiscard = TRUE;
		return;
	}

//	  openserial_printError(COMPONENT_RES,ERR_OK,
//	                           (errorparameter_t)msg->l2_keySource.addr_16b[0],
//	                           (errorparameter_t)21);

	//retrieve the KeyIndex
	if(msg->l2_keyIdMode != 0){
		temp8b = *((uint8_t*)(msg->payload)+tempheader->headerLength);
		msg->l2_keyIndex = (temp8b);
		tempheader->headerLength = tempheader->headerLength+1;
	}

}

void security_incomingFrame(OpenQueueEntry_t*      msg){
	uint8_t match;

	uint32_t tempfr;
	tempfr = m_macFrameCounter;

	m_deviceDescriptor			*devpoint;
	m_keyDescriptor 			*keypoint;
	m_securityLevelDescriptor	*secLevel;

	//check that Security Level is not zero, impossible for me
	/*if(msg->securityLevel == 0){
		return;
	}*/

	//f key descriptor lookup

	match = keyDescriptorLookup(msg->l2_keyIdMode,
								&msg->l2_keySource,
								msg->l2_keyIndex,
								&msg->l2_nextORpreviousHop,
								idmanager_getMyID(ADDR_PANID),
								msg->l2_frameType);

	keypoint = &MacKeyTable.KeyDescriptorElement[match];

	if(match == 25){

		msg->l2_toDiscard = TRUE;
		return;
	}

	//g device descriptor lookup

	open_addr_t Address;
	Address = msg->l2_nextORpreviousHop;

	match = deviceDescriptorLookup(&Address,
						   	       idmanager_getMyID(ADDR_PANID),
						   	   	   keypoint);


	devpoint = &MacDeviceTable.DeviceDescriptorEntry[match];

	//h Security Level lookup

	secLevel = securityLevelDescriptorLookup(msg->l2_frameType,
								  	  	  	 msg->commandFrameIdentifier,
								  	  	  	 secLevel);


	//i+j+k

	if(incomingSecurityLevelChecking(secLevel,msg->l2_securityLevel,devpoint->Exempt)==FALSE){
		//return;
	}

	//l+m Anti-Replay

	if(msg->l2_frameCounter < devpoint->FrameCounter){
		msg->l2_toDiscard = TRUE;
	}

	//n Control of key used
	if(incomingKeyUsagePolicyChecking(keypoint,
									  msg->l2_frameType,
									  0
									  )  ==FALSE){
		msg->l2_toDiscard = TRUE; // improper key used
	}

	uint8_t i;
	switch(msg->l2_keyIdMode){
		case 0:
		case 1:
			for(i=0; i<8; i++){
					 nonce[i] = m_macDefaultKeySource.addr_64b[i];
				}
			break;
		case 2:
			for(i=0; i<2; i++){
						 nonce[i] = msg->l2_keySource.addr_16b[i];
					}
			for(i=2; i<8; i++){
				nonce[i] = 0;
			}

			break;
		case 3:
			for(i=0; i<8; i++){
				 nonce[i] = msg->l2_keySource.addr_64b[i];
			}
			break;
		}

	if(m_macFrameCounterMode == 0x04){
		uint32_t temp;
		temp = msg->l2_frameCounter;

		nonce[8] = temp;
		nonce[9] = temp << 8;
		nonce[10] = temp <<16;
		nonce[11] = temp <<24;
		nonce[12] = msg->l2_securityLevel;
	} else{
		//ASN
		for(i=0;i<5;i++){
			nonce[8+i] = msg->receivedASN[4-i];
		}

	}


	CCMstarInverse(msg,keypoint->key);

	//q increment frame counter and save
	msg->l2_frameCounter +=1;

	devpoint->FrameCounter = msg->l2_frameCounter;
	m_macFrameCounter = tempfr;

}

void authLengthChecking(uint8_t securityLevel){

	switch (securityLevel) {
	 case 0 :
		 authlen = 0;
		 break;
	 case 1 :
		 authlen = 4;
		 break;
	 case 2 :
		 authlen = 8;
	 		 break;
	 case 3 :
		 authlen = 16; //udplatency not function!
	 		 break;
	 case 4 :
		 authlen = 0;
	 		 break;
	 case 5 :
		 authlen = 4;
	 		 break;
	 case 6 :
		 authlen = 8;
	 		 break;
	 case 7 :
		 authlen = 16; //udplatency not function!
	 		 break;
	}

}

uint8_t auxLengthChecking(uint8_t KeyIdMode, bool frameCounterSuppression, uint8_t frameCounterSize){

	uint8_t auxilary_len;
	uint8_t frameCntLength;
	if(frameCounterSuppression == 0){
		if(frameCounterSize == 0x04){
			frameCntLength = 4;
		} else{
			frameCntLength = 5;
		}
	} else{
		frameCntLength = 0;
	}

	switch(KeyIdMode){
		case 0:
			auxilary_len = frameCntLength + 1; //only SecLev and FrameCnt
			break;
		case 1:
			auxilary_len = frameCntLength + 1 + 1; //SecLev, FrameCnt, KeyIndex
			break;
		case 2:
			auxilary_len = frameCntLength + 1 + 3; //SecLev, FrameCnt, KeyIdMode (2 bytes) and KeyIndex
			break;
		case 3:
			auxilary_len = frameCntLength + 1 + 9; //SecLev, FrameCnt, KeyIdMode (8 bytes) and KeyIndex
			break;
		default:
			break;
	}

	return auxilary_len;
}

bool incomingKeyUsagePolicyChecking(m_keyDescriptor* keydesc,
									uint8_t frameType,
									uint8_t cfi){

	uint8_t i;
	for(i=0; i<MAXNUMNEIGHBORS; i++){
		if (frameType != IEEE154_TYPE_CMD && frameType == keydesc->KeyUsageList[i].FrameType){
			return TRUE;
		}

		if (frameType == IEEE154_TYPE_CMD && frameType == keydesc->KeyUsageList[i].FrameType && cfi == keydesc->KeyUsageList[i].CommandFrameIdentifier){
			return TRUE;
		}
	}

	return FALSE;
}

bool incomingSecurityLevelChecking(m_securityLevelDescriptor* seclevdesc,
								   uint8_t seclevel,
								   bool exempt){
	if (seclevdesc->AllowedSecurityLevels == 0){
		if(seclevel <= seclevdesc->SecurityMinimum){
			return TRUE;
		}
		else{
			return FALSE;
		}
	}

	if(seclevel <= seclevdesc->AllowedSecurityLevels){
		return TRUE;
	}

	if(seclevel == 0 && seclevdesc->DeviceOverrideSecurityMinimum ==TRUE ){
		if(exempt == FALSE){
							return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

m_securityLevelDescriptor* securityLevelDescriptorLookup( uint8_t frameType,
									uint8_t cfi,
									m_securityLevelDescriptor* answer){

	uint8_t i;
	for(i=0; i<4; i++){

		if(MacSecurityLevelTable.SecurityDescriptorEntry[i].FrameType != IEEE154_TYPE_CMD
			&& frameType == MacSecurityLevelTable.SecurityDescriptorEntry[i].FrameType){

			answer = &MacSecurityLevelTable.SecurityDescriptorEntry[i];

			return answer;
		}

		if(MacSecurityLevelTable.SecurityDescriptorEntry[i].FrameType == IEEE154_TYPE_CMD
			&& frameType == MacSecurityLevelTable.SecurityDescriptorEntry[i].FrameType
			&& cfi == MacSecurityLevelTable.SecurityDescriptorEntry[i].CommandFrameIdentifier)
		{

			answer = &MacSecurityLevelTable.SecurityDescriptorEntry[i];
			return answer;
		}
	}

	return answer;
}

uint8_t deviceDescriptorLookup(open_addr_t* Address,
							   open_addr_t* PANId,
							   m_keyDescriptor* keydescr){

	uint8_t i;

	for(i=0; i<MAXNUMNEIGHBORS; i++){

		if((packetfunctions_sameAddress(Address,keydescr->DeviceTable->DeviceDescriptorEntry[i].deviceAddress)== TRUE)
			&& (packetfunctions_sameAddress(PANId, MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.PANId))){
			return i;
		}
	}

	//return 25;
}

uint8_t keyDescriptorLookup(uint8_t  		KeyIdMode,
					     	open_addr_t*	keySource,
						 	uint8_t 		KeyIndex,
						 	open_addr_t* 	DeviceAddress,
						 	open_addr_t*	panID,
						 	uint8_t			frameType){

	uint8_t i;

	if(KeyIdMode == 0){

		for(i=0; i<MAXNUMKEYS; i++ ){
			if(packetfunctions_sameAddress(DeviceAddress,MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.Address)
					&& packetfunctions_sameAddress(panID,MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.PANId)){
				return i;
			}
		}

	}

	if (KeyIdMode == 1){
		for(i=0; i<MAXNUMKEYS; i++ ){

				if(KeyIndex == MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.KeyIndex
							&& packetfunctions_sameAddress(keySource,m_macDefaultKeySource)){
							return i;
							}
						}
			}

	if (KeyIdMode == 2){

			for(i=0; i<MAXNUMKEYS; i++ ){
					if(KeyIndex == MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.KeyIndex){

					if( keySource->addr_16b[0] == MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.KeySource.addr_16b[0] &&
							keySource->addr_16b[1] == MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.KeySource.addr_16b[1]
							&& packetfunctions_sameAddress(panID, MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.PANId)
							){
					return i;
					}

				}
			}
		}

	if (KeyIdMode == 3){

		for(i=0; i<MAXNUMKEYS; i++ ){
				if(KeyIndex == MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.KeyIndex){

				if( packetfunctions_sameAddress(keySource,MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.KeySource)
						&& packetfunctions_sameAddress(panID, MacKeyTable.KeyDescriptorElement[i].KeyIdLookupList.PANId)
						){
				return i;
				}

			}
		}
	}

	return 25;//no matches

}

/*
 * Bootstrap Phase for the Parent Node
 */

void coordinator_init(){

	open_addr_t*  my;
	my = idmanager_getMyID(ADDR_64B);
	//my = idmanager_getMyID(ADDR_16B);

	//Creation of the KeyDescriptor

	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIdMode = 3;
	//MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIdMode = 2;
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIndex = 1;
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeySource = *(my);
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.Address = *(my);
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.PANId = *(idmanager_getMyID(ADDR_PANID));

	MacKeyTable.KeyDescriptorElement[0].KeyUsageList[1].FrameType = IEEE154_TYPE_DATA;
	MacKeyTable.KeyDescriptorElement[0].key = M_k;

	MacDeviceTable.DeviceDescriptorEntry[0].deviceAddress = *(my);
	MacDeviceTable.DeviceDescriptorEntry[0].FrameCounter = 0;

	MacKeyTable.KeyDescriptorElement[0].DeviceTable = &MacDeviceTable;

	m_macDefaultKeySource = *(my);

	openserial_printError(COMPONENT_RES,ERR_OK,
						(errorparameter_t)M_k,
						(errorparameter_t)102);

}

void remote_init(ieee802154_header_iht ieee802514_header){

	open_addr_t* src;

	src= &ieee802514_header.src;

	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIdMode = 3;
	//MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIdMode = 2;
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeySource = *(src);

//	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeySource.type = ADDR_16B;
//	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeySource.addr_16b[0] = (src)->addr_16b[0];
//	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeySource.addr_16b[1] = (src)->addr_16b[1];

	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.PANId = ieee802514_header.panid;
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIndex = 1;
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.Address = (ieee802514_header.src);

	MacKeyTable.KeyDescriptorElement[0].KeyUsageList[1].FrameType = IEEE154_TYPE_DATA;


	MacKeyTable.KeyDescriptorElement[0].key = M_k;

	m_macDefaultKeySource = *(src);

	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.KeyIndex = 1;
	MacKeyTable.KeyDescriptorElement[0].KeyIdLookupList.Address = *(src);

	//DEVICE TABLE

	MacDeviceTable.DeviceDescriptorEntry[0].deviceAddress = *(src);
	MacDeviceTable.DeviceDescriptorEntry[0].FrameCounter = 0;

	MacKeyTable.KeyDescriptorElement[0].DeviceTable = &MacDeviceTable;

	openserial_printError(COMPONENT_RES,ERR_OK,
						(errorparameter_t)M_k,
						(errorparameter_t)201);

}

//=========================== private =========================================
