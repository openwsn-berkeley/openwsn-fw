/**
\brief Entry point for accessing the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, October 2014.
*/

#include "opendefs.h"
//===== drivers
#include "openserial.h"
//===== stack
#include "openstack.h"
//-- cross-layer
#include "idmanager.h"
#include "openqueue.h"
#include "openrandom.h"
#include "opentimers.h"
//-- 02a-TSCH
#include "adaptive_sync.h"
#include "IEEE802154E.h"
//-- 02b-RES
#include "schedule.h"
#include "sixtop.h"
#include "neighbors.h"
//-- 03a-IPHC
#include "openbridge.h"
#include "iphc.h"
//-- 03b-IPv6
#include "forwarding.h"
#include "icmpv6.h"
#include "icmpv6echo.h"
#include "icmpv6rpl.h"
//-- 04-TRAN
#include "opentcp.h"
#include "openudp.h"
#include "opencoap.h"
//===== applications
#include "openapps.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openstack_init(void) {
   
   //===== drivers
   openserial_init();
   
   //===== stack
   //-- cross-layer
   idmanager_init();    // call first since initializes EUI64 and isDAGroot
   openqueue_init();
   openrandom_init();
   opentimers_init();
   //-- 02a-TSCH
   adaptive_sync_init();
   ieee154e_init();
   //-- 02b-RES
   schedule_init();
   sixtop_init();
   neighbors_init();
   //-- 03a-IPHC
   openbridge_init();
   iphc_init();
   //-- 03b-IPv6
   forwarding_init();
   icmpv6_init();
   icmpv6echo_init();
   icmpv6rpl_init();
   //-- 04-TRAN
   opentcp_init();
   openudp_init();
   opencoap_init();     // initialize before any of the CoAP applications
   
   //===== applications
   openapps_init();
   
   openserial_printInfo(
      COMPONENT_OPENWSN,ERR_BOOTED,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
}
