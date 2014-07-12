/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2012
*/

#include "openwsn.h"
//===== drivers
#include "openserial.h"
//===== stack
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
#include "res.h"
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
//+++++ TCP
//- debug
#include "tcpecho.h"
#include "tcpinject.h"
#include "tcpprint.h"
//- common
#include "ohlone.h"
//- board-specific
//++++ UDP
//- debug
#include "udpecho.h"
#include "udpinject.h"
#include "udpprint.h"
//- common
//#include "udprand.h"
//#include "udplatency.h"
#include "udpstorm.h"
//- board-specific
//#include "imu.h"
//+++++ CoAP
//- debug
//- common
#include "rinfo.h"
#include "rleds.h"
#include "rwellknown.h"
#include "r6t.h"
//#include "rrt.h"
//#include "rex.h"
//#include "rrube.h"
//#include "layerdebug.h"
//- board-specific
//#include "rheli.h"
//#include "rt.h"
//#include "rxl1.h"
//#include "heli.h"
//#include "imu.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openwsn_init() {
   //===== drivers
   openserial_init();
   //heli_init();
   //imu_init();
   
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
   res_init();
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
   //+++++ TCP
   //- debug
   tcpecho_init();
   tcpinject_init();
   tcpprint_init();
   //- common
   ohlone_init();
   //- board-specific
   //+++++ UDP
   //- debug
   udpecho_init();
   udpinject_init();
   udpprint_init();
   //- common
   //udprand_init();
   //udplatency_init();
   udpstorm_init();
   //- board-specific
   //imu_init();
   //+++++ CoAP
   //- debug
   //- core
   //- common
   rinfo_init();
   rrt_init();
   rleds__init();
   rwellknown_init();
   r6t_init();
   //rreg_init();
   //rex_init();
   //rrube_init();
   //layerdebug_init();
   //- board-specific
   //rheli_init();
   //rt_init();
   //rxl1_init();
   
   openserial_printInfo(
      COMPONENT_OPENWSN,ERR_BOOTED,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
}
