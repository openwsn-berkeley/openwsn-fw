/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:06.845440.
*/
/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2012
*/

#include "openwsn_obj.h"
//===== drivers
#include "openserial_obj.h"
//===== stack
//-- cross-layer
#include "idmanager_obj.h"
#include "openqueue_obj.h"
#include "openrandom_obj.h"
#include "opentimers_obj.h"
//-- 02a-TSCH
#include "adaptive_sync_obj.h"
#include "IEEE802154E_obj.h"
//-- 02b-RES
#include "schedule_obj.h"
#include "sixtop_obj.h"
#include "neighbors_obj.h"
//-- 03a-IPHC
#include "openbridge_obj.h"
#include "iphc_obj.h"
//-- 03b-IPv6
#include "forwarding_obj.h"
#include "icmpv6_obj.h"
#include "icmpv6echo_obj.h"
#include "icmpv6rpl_obj.h"
//-- 04-TRAN
#include "opentcp_obj.h"
#include "openudp_obj.h"
#include "opencoap_obj.h"
//===== applications
//+++++ TCP
//- debug
#include "tcpecho_obj.h"
#include "tcpinject_obj.h"
#include "tcpprint_obj.h"
//- common
#include "ohlone_obj.h"
//- board-specific
//++++ UDP
//- debug
#include "udpecho_obj.h"
#include "udpinject_obj.h"
#include "udpprint_obj.h"
//- common
//#include "udprand_obj.h"
//#include "udplatency_obj.h"
#include "udpstorm_obj.h"
//- board-specific
//#include "imu.h"
//+++++ CoAP
//- debug
//- common
#include "rinfo_obj.h"
#include "rleds_obj.h"
#include "rwellknown_obj.h"
#include "r6t.h"
//#include "rrt.h"
//#include "rex.h"
//#include "rrube_obj.h"
//- board-specific
//#include "rheli_obj.h"
//#include "rt.h"
//#include "rxl1.h"
//#include "heli.h"
//#include "imu.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openwsn_init(OpenMote* self) {
   //===== drivers
 openserial_init(self);
   //heli_init();
   //imu_init();
   
   //===== stack
   //-- cross-layer
 idmanager_init(self);    // call first since initializes EUI64 and isDAGroot
 openqueue_init(self);
 openrandom_init(self);
 opentimers_init(self);
   //-- 02a-TSCH
 adaptive_sync_init(self);
 ieee154e_init(self);
   //START OF TELEMATICS CODE
   security_init();
   //END OF TELEMATICS CODE
   //-- 02b-RES
 schedule_init(self);
 sixtop_init(self);
 neighbors_init(self);
   //-- 03a-IPHC
 openbridge_init(self);
 iphc_init(self);
   //-- 03b-IPv6
 forwarding_init(self);
 icmpv6_init(self);
 icmpv6echo_init(self);
 icmpv6rpl_init(self);
   //-- 04-TRAN
 opentcp_init(self);
 openudp_init(self);
 opencoap_init(self);     // initialize before any of the CoAP applications
   
   //===== applications
   //+++++ TCP
   //- debug
   // tcpecho_init(self);
   // tcpinject_init(self);
   // tcpprint_init(self);
   //- common
   // ohlone_init(self);
   //- board-specific
   //+++++ UDP
   //- debug
   // udpecho_init(self);
   // udpinject_init(self);
   // udpprint_init(self);
   //- common
   // udprand_init(self);
 udplatency_init(self);
   // udpstorm_init(self);
   //- board-specific
   //imu_init();
   //+++++ CoAP
   //- debug
   //- core
   //- common
   // rinfo_init(self);
   // rrt_init(self);
   // rleds__init(self);
   // rwellknown_init(self);
   // r6t_init(self);
   // rreg_init(self);
   // rex_init(self);
   //rrube_init();
   // layerdebug_init(self);
   //- board-specific
   //rheli_init();
   //rt_init();
   //rxl1_init();
   
// openserial_printInfo(self, 
//      COMPONENT_OPENWSN,ERR_BOOTED,
//      (errorparameter_t)0,
//      (errorparameter_t)0
//   );
}
