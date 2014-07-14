#ifndef __SECURITY_H
#define __SECURITY_H

/**
\addtogroup helpers
\{
\addtogroup IEEE802154
\{
*/

#include "openwsn.h"
#include "IEEE802154.h"
#include "neighbors.h"
#include "idmanager.h"

//=========================== define ==========================================

#define MAXNUMKEYS           MAXNUMNEIGHBORS+1


enum Auxiliary_Security_Header_scf_enums{ //Security Control Field
		ASH_SCF_SECURITY_LEVEL = 0,
		ASH_SCF_KEY_IDENTIFIER_MODE = 3,
		ASH_SCF_FRAME_CNT_MODE = 5,
		ASH_SCF_FRAME_CNT_SIZE = 6,
};

enum Auxiliary_Security_Header_slf_enums{ //Security Level Field
		ASH_SLF_TYPE_NOSEC = 0,
		ASH_SLF_TYPE_MIC_32 = 1,
		ASH_SLF_TYPE_MIC_64 = 2,
		ASH_SLF_TYPE_MIC_128 = 3,
		ASH_SLF_TYPE_ONLYCRYPTO = 4,
		ASH_SLF_TYPE_CRYPTO_MIC32 = 5,
		ASH_SLF_TYPE_CRYPTO_MIC64= 6,
		ASH_SLF_TYPE_CRYPTO_MIC128 = 7,
};

//=========================== typedef =========================================


typedef struct{//identifier of the device which is using the key
	open_addr_t	 deviceAddress;
	uint32_t 	 FrameCounter;
	bool 		 Exempt;
	} m_deviceDescriptor;

typedef struct{//descriptor of the key we are looking for
	uint8_t  	 KeyIdMode;
	uint8_t 	 KeyIndex;
	open_addr_t  KeySource;
	open_addr_t  PANId;
	open_addr_t  Address;//struct in which there are PANID,Short Address, Extended Address
	} m_keyIdLookupDescriptor;

typedef struct{//descriptor of the Security Level for this type of communication
	uint8_t 	FrameType;
	uint8_t 	CommandFrameIdentifier; //if the FrameType is a command, this specify what kind of command is
	uint8_t 	SecurityMinimum; //minimum required
	bool 		DeviceOverrideSecurityMinimum; //if true, this indicate that the minimum can be overridden
	uint8_t 	AllowedSecurityLevels; //set of Security Levels Allowed for incoming MAC frames, occhio al tipo di dato passato, lo imposto come un numero.. livelli di sicurezza inferiori sono ok.
	} m_securityLevelDescriptor;

typedef struct{//defines what kind of frame the key is intended
	uint8_t		FrameType;
	uint8_t		CommandFrameIdentifier;
	} m_keyUsageDescriptor;

typedef struct{//Table which contains the device that are currently using this key
	m_deviceDescriptor    DeviceDescriptorEntry[MAXNUMNEIGHBORS];
	} m_macDeviceTable;


typedef struct{//descriptor of the key
	m_keyIdLookupDescriptor 	KeyIdLookupList;
	m_macDeviceTable*			DeviceTable;
	m_keyUsageDescriptor		KeyUsageList[3];
	unsigned long long int		key;
	} m_keyDescriptor;

typedef struct{
	m_keyDescriptor   KeyDescriptorElement[MAXNUMKEYS];
	} m_macKeyTable;

typedef struct{
	m_securityLevelDescriptor     SecurityDescriptorEntry[5];
	} m_macSecurityLevelTable;


//=========================== variables =======================================

uint32_t 		m_macFrameCounter;
uint8_t			m_macFrameCounterMode;
uint8_t 		m_macAutoRequestKeyIdMode;
uint8_t			m_macAutoRequestSecurityLevel;
uint8_t 		m_macAutoReququestKeyIndex;
open_addr_t		m_macDefaultKeySource;


m_macKeyTable				MacKeyTable;
m_macDeviceTable			MacDeviceTable;
m_macSecurityLevelTable 	MacSecurityLevelTable;

uint8_t payloadToEncrypt[128];
uint8_t CipherText[128];

//Length of the Auth Field
uint8_t authlen;

//MASTER KEY
uint32_t M_k;

//NONCE STRING
uint8_t nonce[13];


//=========================== prototypes ======================================

//admin
void security_init();
//public
void security_outgoingFrameSec(OpenQueueEntry_t*  msg,
                                uint8_t           securityLevel,
                                uint8_t           keyIdMode,
                                open_addr_t*      keySource,
                                uint8_t           keyIndex);
void retrieve_ASH(OpenQueueEntry_t*      msg,
                  ieee802154_header_iht* tempheader);

void security_incomingFrameSec (OpenQueueEntry_t*      msg);

void authLengthChecking(uint8_t securityLevel); //it determines the length of the Authentication field(not standard)

uint8_t auxLengthChecking(uint8_t KeyIdMode,
					   bool frameCounterSuppression,
					   uint8_t frameCounterSize); //it determines the length of the Auxiliary Security Header (not standard)

bool incomingKeyUsagePolicyChecking(m_keyDescriptor* keydesc,
									uint8_t frameType,
									uint8_t cfi);

bool incomingSecurityLevelChecking(m_securityLevelDescriptor* seclevdesc,
								   uint8_t seclevel,
								   bool exempt);

m_securityLevelDescriptor* securityLevelDescriptorLookup(	uint8_t frameType,
														uint8_t cfi,
														m_securityLevelDescriptor* answer);

uint8_t deviceDescriptorLookup(open_addr_t* address,
								open_addr_t* PANID,
								m_keyDescriptor* keydescr);

uint8_t keyDescriptorLookup(uint8_t  		KeyIdMode,
						 	open_addr_t*	keySource,
						 	uint8_t 		KeyIndex,
						 	open_addr_t*	DeviceAddress,
						 	open_addr_t*	panID,
						 	uint8_t			frameType);

void remote_init(ieee802154_header_iht ieee802514_header);
void coordinator_init();
/**
\}
\}
*/

#endif
