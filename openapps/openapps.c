/**
\brief Applications running on top of the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2014.
*/

#include "opendefs.h"

//=== default applications
// CoAP
#include "c6t.h"
#include "cinfo.h"
#include "cleds.h"
#include "cwellknown.h"
// TCP
#include "techo.h"
// UDP
#include "uecho.h"

//=== user applications
// MARKER_USER_APPS_INCLUDES_HERE (don't remove this line)

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openapps_init(void) {
   
   //=== default applications
   // CoAP
   c6t_init();
   cinfo_init();
   cleds__init();
   cwellknown_init();
   // TCP
   techo_init();
   // UDP
   uecho_init();
   
   //=== user applications
   // These applications are added on-the-fly by the SCons build system by 
   // using the 'apps=udprand,ohlone' switch
   // MARKER_USER_APPS_INITS_HERE (don't remove this line)
}
