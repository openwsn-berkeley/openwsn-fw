/**
\brief Applications running on top of the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2014.
*/

#include "opendefs.h"

// CoAP
#include "opencoap.h"
#include "c6t.h"
#include "cinfo.h"
#include "cleds.h"
#include "cexample.h"
#include "cstorm.h"
#include "cwellknown.h"
#include "rrt.h"
// TCP
#include "techo.h"
// UDP
#include "uecho.h"
#include "uinject.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openapps_init(void) {
   //-- 04-TRAN
   opencoap_init();     // initialize before any of the CoAP applications

   // CoAP
   c6t_init();
   cinfo_init();
   //cexample_init();
   cleds__init();
   cstorm_init();
   cwellknown_init();
   rrt_init();
   // TCP
   techo_init();
}
