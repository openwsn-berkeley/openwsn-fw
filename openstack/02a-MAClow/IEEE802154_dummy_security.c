/**
\brief Dummy IEEE802154 security implementation that is linked when security is disabled.
  
\author Malisa Vucinic <malishav@gmail.com>, June 2015.
*/

#include "IEEE802154_dummy_security.h"

static void init(void) {
   return;
}

static void prependAuxiliarySecurityHeader(OpenQueueEntry_t* msg){
   return;
}

static void retrieveAuxiliarySecurityHeader(OpenQueueEntry_t* msg, ieee802154_header_iht* tempheader) {
   return;
}

static owerror_t outgoingFrame(OpenQueueEntry_t* msg) {
   return E_SUCCESS;
}

static owerror_t incomingFrame(OpenQueueEntry_t* msg) {
   return E_SUCCESS;
}

static uint8_t authenticationTagLen(uint8_t sec_level) {
   return (uint8_t) 0;
}

static uint8_t auxiliaryHeaderLen(uint8_t kid, uint8_t sup, uint8_t size) {
   return (uint8_t) 0;
}

/*---------------------------------------------------------------------------*/
const struct ieee802154_security_driver IEEE802154_dummy_security = {
   init,
   prependAuxiliarySecurityHeader,
   retrieveAuxiliarySecurityHeader,
   outgoingFrame,
   incomingFrame,
   authenticationTagLen,
   auxiliaryHeaderLen,
};
/*---------------------------------------------------------------------------*/

