/**
\brief Definitions for IEEE802154_security driver

\author Malisa Vucinic <malishav@gmail.com>, June 2015.
*/
#ifndef __IEEE802154_SECURITY_DRIVER_H__
#define __IEEE802154_SECURITY_DRIVER_H__

#include "opendefs.h"
#include "IEEE802154.h"

//=========================== define ==========================================

#ifdef L2_SECURITY_ACTIVE
#define IEEE802154_SECURITY IEEE802154_security
#define IEEE802154_SECURITY_ENABLED 1
#else /* L2_SECURITY_ACTIVE */
#define IEEE802154_SECURITY IEEE802154_dummy_security
#define IEEE802154_SECURITY_ENABLED 0
#endif /* L2_SECURITY_ACTIVE */

//=========================== module variables ================================

struct ieee802154_security_driver {

   void (* init)(void);

   void (* prependAuxiliarySecurityHeader)(OpenQueueEntry_t* msg);

   void (* retrieveAuxiliarySecurityHeader)(OpenQueueEntry_t* msg, ieee802154_header_iht* tempheader);

   owerror_t (* outgoingFrame)(OpenQueueEntry_t* msg);

   owerror_t (* incomingFrame)(OpenQueueEntry_t* msg);

}; // struct ieee802154_security_driver 

extern const struct ieee802154_security_driver IEEE802154_SECURITY;

#endif /* __IEEE802154_SECURITY_DRIVER_H__ */

