/**
\brief Initializes the CoAP application layer and its security extensions.

\author Timothy Claeys <timothy.claeys@inria.fr>, March 2020.
*/

#include "config.h"

#include "applayer.h"

#include "coap.h"
#include "cjoin.h"

void applayer_init() {
    coap_init();
    cjoin_init();
}