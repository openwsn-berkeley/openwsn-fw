/**
\brief Initializes the CoAP application layer and its security extensions.

\author Timothy Claeys <timothy.claeys@inria.fr>, March 2020.
*/

#include "config.h"

#include "applayer.h"

#if defined(OPENWSN_COAP_C)
#include "coap.h"
#endif

#if defined(OPENWSN_CJOIN_C)
#include "cjoin.h"
#endif

void applayer_init() {

#if defined(OPENWSN_COAP_C)
    coap_init();
#endif

#if defined(OPENWSN_CJOIN_C)
    cjoin_init();
#endif

}