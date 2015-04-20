/**
\brief Security operations defined by IEEE802.15.4 standard

\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, April 2015.
\author Giuseppe Piro <giuseppe.piro@poliba.it>,
\author Gennaro Boggia <gennaro.boggia@poliba.it>,
\author Luigi Alfredo Grieco <alfredo.grieco@poliba.it>
*/

#ifndef __SECURITY_H
#define __SECURITY_H

/**
\addtogroup helpers
\{
\addtogroup IEEE802154
\{
*/

#include "IEEE802154.h"
#include "neighbors.h"
#include "idmanager.h"
#include "openserial.h"
#include "opendefs.h"
#include "packetfunctions.h"
#include "crypto_engine.h"

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
	open_addr_t	      deviceAddress;
	macFrameCounter_t FrameCounter;
	bool 		      Exempt;
} m_deviceDescriptor;

typedef struct{//descriptor of the key we are looking for
	uint8_t      KeyIdMode;
	uint8_t      KeyIndex;
	open_addr_t  KeySource;
	open_addr_t  PANId;
	open_addr_t  Address;
} m_keyIdLookupDescriptor;

typedef struct{//descriptor of the Security Level for this type of communication
	uint8_t FrameType;
	uint8_t CommandFrameIdentifier; //if the FrameType is a command, this specify what kind of command is
	uint8_t SecurityMinimum; //minimum required
	bool 	DeviceOverrideSecurityMinimum; //if true, this indicate that the minimum can be overridden
	uint8_t AllowedSecurityLevels; //set of Security Levels Allowed for incoming MAC frames
} m_securityLevelDescriptor;

typedef struct{//defines what kind of frame the key is intended
	uint8_t	FrameType;
	uint8_t	CommandFrameIdentifier;
} m_keyUsageDescriptor;

typedef struct{//Table which contains the device that are currently using this key
	m_deviceDescriptor    DeviceDescriptorEntry[MAXNUMNEIGHBORS];
} m_macDeviceTable;


typedef struct{//descriptor of the key
	m_keyIdLookupDescriptor KeyIdLookupList;
	m_macDeviceTable*       DeviceTable;
	m_keyUsageDescriptor    KeyUsageList[3];
	uint8_t                 key[16];
} m_keyDescriptor;

typedef struct{
	m_keyDescriptor KeyDescriptorElement[MAXNUMKEYS];
} m_macKeyTable;

typedef struct{
	m_securityLevelDescriptor SecurityDescriptorEntry[5];
} m_macSecurityLevelTable;


//=========================== variables =======================================
typedef struct{
	macFrameCounter_t       m_macFrameCounter;
	uint8_t                 m_macFrameCounterMode;
	uint8_t                 m_macAutoRequestKeyIdMode;
	uint8_t                 m_macAutoRequestSecurityLevel;
	uint8_t                 m_macAutoReququestKeyIndex;
	open_addr_t             m_macDefaultKeySource;
	m_macKeyTable           MacKeyTable;
	m_macDeviceTable        MacDeviceTable;
	m_macSecurityLevelTable MacSecurityLevelTable;
	uint8_t                 M_k[16];
}security_vars_t;

//=========================== prototypes ======================================

//admin
void IEEE802154security_init(void);
//public
void IEEE802154security_prependAuxiliarySecurityHeader(OpenQueueEntry_t* msg);
void IEEE802154security_outgoingFrameSecurity(OpenQueueEntry_t* msg);
void IEEE802154security_retrieveAuxiliarySecurityHeader(OpenQueueEntry_t*      msg,
                                                        ieee802154_header_iht* tempheader);

void IEEE802154security_incomingFrame(OpenQueueEntry_t* msg);

void IEEE802154security_ChildsInit(ieee802154_header_iht ieee802514_header);
void IEEE802154security_DAGRoot_init(void);
/**
\}
\}
*/

#endif
