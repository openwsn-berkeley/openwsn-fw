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
#include "ieee802154_security_driver.h"
#include "neighbors.h"

//=========================== define ==========================================

#define MAXNUMKEYS           MAXNUMNEIGHBORS+1

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
} ieee802154_security_vars_t;

extern const struct ieee802154_security_driver IEEE802154_security;

//=========================== prototypes ======================================

/**
\}
\}
*/

#endif
