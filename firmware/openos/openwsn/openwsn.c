/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
*/

#include "openwsn.h"
#include "scheduler.h"
//l7
#include "appudpsensor.h"
#include "appudpprint.h"
#include "appudpleds.h"
#include "appudptimer.h"
#include "appudpinject.h"
#include "appudpgina.h"
#include "appudpheli.h"
#include "appudpecho.h"
#include "appudpchannel.h"
#include "apptcpprint.h"
#include "apptcpohlone.h"
#include "apptcpinject.h"
#include "apptcpecho.h"
//l4
#include "openudp.h"
#include "opentcp.h"
//l3b
#include "icmpv6rpl.h"
#include "icmpv6router.h"
#include "icmpv6echo.h"
#include "icmpv6.h"
#include "forwarding.h"
//l3a
#include "iphc.h"
#include "openbridge.h"
//l2b
#include "neighbors.h"
#include "res.h"
#include "schedule.h"
//l2a
#include "IEEE802154E.h"
//cross-layer
#include "idmanager.h"
#include "openqueue.h"
#include "openrandom.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void openwsn_init();

//=========================== public ==========================================

void openwsn_init() {
   // cross-layer
   idmanager_init(); // call first since initializes e.g. EUI64
   openqueue_init();
   openrandom_init();
   // 02a-TSCH
   mac_init();
   // 02b-RES
   schedule_init();
   res_init();
   neighbors_init();
   // 03a-IPHC
   openbridge_init();
   iphc_init();
   // 03b-IPv6
   forwarding_init();
   icmpv6_init();
   icmpv6echo_init();
   icmpv6router_init();
   icmpv6rpl_init();
   // 04-TRAN
   opentcp_init();
   openudp_init();
   // 07-App
   apptcpecho_init();
   apptcpinject_init();
   apptcpohlone_init();
   apptcpprint_init();
   appudpchannel_init();
   appudpecho_init();
   //appudpheli_init(); remove heli application for now since we need TimerA for IEEE802.15.4e
   appudpgina_init();
   appudpinject_init();
   appudptimer_init();
   appudpleds_init();
   appudpprint_init();
   appudpsensor_init();
}

//=========================== private =========================================