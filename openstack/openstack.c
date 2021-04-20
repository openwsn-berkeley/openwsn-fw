/**
\brief Entry point for accessing the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, October 2014.
*/

#include <stdio.h>
#include "config.h"

//===== stack
#include "openstack.h"
//-- cross-layer
#include "idmanager.h"
#include "openqueue.h"
#include "openrandom.h"
//-- 02a-TSCH
#include "adaptive_sync.h"
#include "IEEE802154E.h"
//-- 02b-RES
#include "schedule.h"
#include "sixtop.h"
#include "neighbors.h"
#include "msf.h"
//-- 03a-IPHC
#include "openbridge.h"
#include "iphc.h"
#include "frag.h"
//-- 03b-IPv6
#include "forwarding.h"
#include "icmpv6.h"
#include "icmpv6echo.h"
#include "icmpv6rpl.h"
//-- 04-TRAN
#include "sock/sock.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openstack_init(void) {

    //===== stack
    //-- cross-layer
    idmanager_init();    // call first since initializes EUI64 and isDAGroot
    openqueue_init();
    openrandom_init();

    //-- 02a-TSCH
#if OPENWSN_ADAPTIVE_SYNC_C
    adaptive_sync_init();
#endif

    ieee154e_init();
    //-- 02b-RES
    schedule_init();
    sixtop_init();
    neighbors_init();
    msf_init();
    //-- 03a-IPHC
    openbridge_init();
    iphc_init();

#if OPENWSN_6LO_FRAGMENTATION_C
    frag_init();
#endif

    //-- 03b-IPv6
    forwarding_init();
    icmpv6_init();

#if OPENWSN_ICMPV6_ECHO_C
    icmpv6echo_init();
#endif

    icmpv6rpl_init();
    //-- 04-TRAN

#if OPENWSN_UDP_C
    sock_udp_init();
#endif
}
