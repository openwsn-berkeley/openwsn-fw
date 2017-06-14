/**
\brief Security operations defined by IEEE802.15.4 standard

\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, June 2015.
\author Giuseppe Piro <giuseppe.piro@poliba.it>, June 2015
\author Gennaro Boggia <gennaro.boggia@poliba.it>, June 2015
\author Luigi Alfredo Grieco <alfredo.grieco@poliba.it>, June 2015
\author Malisa Vucinic <malishav@gmail.com>, June 2015.
*/

#ifndef __IEEE802154_SECURITY_H
#define __IEEE802154_SECURITY_H

/**
\addtogroup helpers
\{
\addtogroup IEEE802154
\{
*/

#include "opendefs.h"
#include "neighbors.h"

//=========================== define ==========================================

#define MAXNUMKEYS           MAXNUMNEIGHBORS+1

#ifdef L2_SECURITY_ACTIVE  // Configuring security levels
#define IEEE802154_SECURITY_SUPPORTED        1
#define IEEE802154_SECURITY_LEVEL            IEEE154_ASH_SLF_TYPE_ENC_MIC_32  // encryption + 4 byte authentication tag   
#define IEEE802154_SECURITY_LEVEL_BEACON     IEEE154_ASH_SLF_TYPE_MIC_32      // authentication tag len used for beacons must match the tag len of other frames 
#define IEEE802154_SECURITY_KEYIDMODE        IEEE154_ASH_KEYIDMODE_DEFAULTKEYSOURCE
#define IEEE802154_SECURITY_K1_KEY_INDEX     1
#define IEEE802154_SECURITY_K2_KEY_INDEX     2
#define IEEE802154_SECURITY_TAG_LEN          IEEE802154_security_authLengthChecking(IEEE802154_SECURITY_LEVEL)
#define IEEE802154_SECURITY_HEADER_LEN       IEEE802154_security_auxLengthChecking(IEEE802154_SECURITY_KEYIDMODE, IEEE154_ASH_FRAMECOUNTER_SUPPRESSED, 5) // For TSCH we always use implicit 5 byte ASN as Frame Counter
#define IEEE802154_SECURITY_TOTAL_OVERHEAD   IEEE802154_SECURITY_TAG_LEN + IEEE802154_SECURITY_HEADER_LEN
#define IEEE802154_SECURITY_MINIMAL_PROC     0                                // minimal processing for efficiency
#else /* L2_SECURITY_ACTIVE */
#define IEEE802154_SECURITY_SUPPORTED        0
#define IEEE802154_SECURITY_LEVEL            IEEE154_ASH_SLF_TYPE_NOSEC 
#define IEEE802154_SECURITY_LEVEL_BEACON     IEEE154_ASH_SLF_TYPE_NOSEC 
#define IEEE802154_SECURITY_KEYIDMODE        0
#define IEEE802154_SECURITY_K1_KEY_INDEX     0
#define IEEE802154_SECURITY_K2_KEY_INDEX     0
#define IEEE802154_SECURITY_TAG_LEN          0
#define IEEE802154_SECURITY_HEADER_LEN       0
#define IEEE802154_SECURITY_TOTAL_OVERHEAD   0
#define IEEE802154_SECURITY_MINIMAL_PROC     0
#endif /* L2_SECURITY_ACTIVE */

//=========================== typedef =========================================

typedef struct{//identifier of the device which is using the key
	open_addr_t       deviceAddress;
	macFrameCounter_t FrameCounter;
	bool              Exempt;
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
   bool    DeviceOverrideSecurityMinimum; //if true, this indicate that the minimum can be overridden
   uint8_t AllowedSecurityLevels[7]; //set of Security Levels Allowed for incoming MAC frames
} m_securityLevelDescriptor;

typedef struct{//defines what kind of frame the key is intended
   uint8_t FrameType;
   uint8_t CommandFrameIdentifier;
} m_keyUsageDescriptor;

typedef struct{//Table which contains the device that are currently using this key
   m_deviceDescriptor DeviceDescriptorEntry[MAXNUMNEIGHBORS-1];
} m_macDeviceTable;


typedef struct{//descriptor of the key
   m_keyIdLookupDescriptor KeyIdLookupList;
   m_macDeviceTable*       DeviceTable;
   m_keyUsageDescriptor    KeyUsageList[3];
   uint8_t                 key[16];
} m_keyDescriptor;

typedef struct{
   m_keyDescriptor KeyDescriptorElement[MAXNUMKEYS-1];
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
   uint8_t                 Key_1[16];
   uint8_t                 Key_2[16];
   uint8_t                 minimal;
} ieee802154_security_vars_t;

//=========================== prototypes ======================================

void IEEE802154_security_init(void);

void IEEE802154_security_prependAuxiliarySecurityHeader(OpenQueueEntry_t* msg);

void IEEE802154_security_retrieveAuxiliarySecurityHeader(OpenQueueEntry_t* msg, ieee802154_header_iht* tempheader);

owerror_t IEEE802154_security_outgoingFrameSecurity(OpenQueueEntry_t* msg);

owerror_t IEEE802154_security_incomingFrame(OpenQueueEntry_t* msg);

uint8_t IEEE802154_security_authLengthChecking(uint8_t);

uint8_t IEEE802154_security_auxLengthChecking(uint8_t keyIdMode, uint8_t frameCounterSuppression, uint8_t frameCounterSize);


/**
\}
\}
*/

#endif
