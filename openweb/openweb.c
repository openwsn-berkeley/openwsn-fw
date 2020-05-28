/**
\brief Initializes the CoAP application layer and its security extensions.

\author Timothy Claeys <timothy.claeys@inria.fr>, March 2020.
*/

#include "config.h"

#include "openweb.h"

#if defined(OPENWSN_COAP_C)
#include "coap.h"
#endif

void openweb_init() {

#if defined(OPENWSN_COAP_C)
    coap_init();
#endif

}