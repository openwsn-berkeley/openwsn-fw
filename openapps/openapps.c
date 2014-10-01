/**
\brief Applications running on top of the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2014.
*/

#include "openwsn.h"

//=== default applications
// TCP
#include "tcpecho.h"
// UDP
#include "udpecho.h"
// CoAP
#include "rinfo.h"
#include "rleds.h"
#include "rwellknown.h"
#include "r6t.h"

//=== user applications
// MARKER_USER_APPS_INCLUDES_HERE (don't remove this line)

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openapps_init(void) {
   
   //=== default applications
   // TCP
   tcpecho_init();
   // UDP
   udpecho_init();
   // CoAP
   r6t_init();
   rinfo_init();
   rleds__init();
   rwellknown_init();
   
   //=== user applications
   // These applications are added on-the-fly by the SCons build system by 
   // using the 'apps=udprand,ohlone' switch
   // MARKER_USER_APPS_INITS_HERE (don't remove this line)
}
