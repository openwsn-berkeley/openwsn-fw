/**
\brief Definitions for IEEE802154_security driver

\author Malisa Vucinic <malishav@gmail.com>, June 2015.
*/
#ifndef __IEEE802154_SECURITY_DRIVER_H__
#define __IEEE802154_SECURITY_DRIVER_H__

#include "opendefs.h"
#include "IEEE802154.h"

//=========================== define ==========================================
#ifdef L2_SECURITY_ACTIVE  // Configuring security levels
#define IEEE802154_SECURITY                  IEEE802154_security              // implementation
#define IEEE802154_SECURITY_SUPPORTED        1
#define IEEE802154_SECURITY_LEVEL            IEEE154_ASH_SLF_TYPE_ENC_MIC_32  // encryption + 4 byte authentication tag   
#define IEEE802154_SECURITY_LEVEL_BEACON     IEEE154_ASH_SLF_TYPE_MIC_32      // authentication tag len used for beacons must match the tag len of other frames 
#define IEEE802154_SECURITY_KEYIDMODE        IEEE154_ASH_KEYIDMODE_DEFAULTKEYSOURCE
#define IEEE802154_SECURITY_K1_KEY_INDEX     1
#define IEEE802154_SECURITY_K2_KEY_INDEX     2
#define IEEE802154_SECURITY_TAG_LEN          IEEE802154_SECURITY.authenticationTagLen(IEEE802154_SECURITY_LEVEL)
#define IEEE802154_SECURITY_HEADER_LEN       IEEE802154_SECURITY.auxiliaryHeaderLen(IEEE802154_SECURITY_KEYIDMODE, IEEE154_ASH_FRAMECOUNTER_SUPPRESSED, 5) // For TSCH we always use implicit 5 byte ASN as Frame Counter
#define IEEE802154_SECURITY_TOTAL_OVERHEAD   IEEE802154_SECURITY_TAG_LEN + IEEE802154_SECURITY_HEADER_LEN
#define IEEE802154_SECURITY_MINIMAL_PROC     0                                // minimal processing for efficiency
#else /* L2_SECURITY_ACTIVE */
#define IEEE802154_SECURITY                  IEEE802154_dummy_security        // dummy implementation that always returns success
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

//=========================== module variables ================================

struct ieee802154_security_driver {

   void (* init)(void);

   void (* prependAuxiliarySecurityHeader)(OpenQueueEntry_t* msg);

   void (* retrieveAuxiliarySecurityHeader)(OpenQueueEntry_t* msg, ieee802154_header_iht* tempheader);

   owerror_t (* outgoingFrame)(OpenQueueEntry_t* msg);

   owerror_t (* incomingFrame)(OpenQueueEntry_t* msg);

   uint8_t (* authenticationTagLen)(uint8_t);

   uint8_t (* auxiliaryHeaderLen)(uint8_t keyIdMode, uint8_t frameCounterSuppression, uint8_t frameCounterSize);

}; // struct ieee802154_security_driver 

extern const struct ieee802154_security_driver IEEE802154_SECURITY;

#endif /* __IEEE802154_SECURITY_DRIVER_H__ */

