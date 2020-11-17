/**
\brief Applications running on top of the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2014.
\author Timothy Claeys <timothy.claeys@inria.fr>, March 2020.
*/

#include "config.h"
#include "opendefs.h"

#if OPENWSN_C6T_C
#include "c6t.h"
#endif

#if OPENWSN_CINFO_C
#include "cinfo.h"
#endif

#if OPENWSN_CLED_C
#include "cled.h"
#endif

#if OPENWSN_CWELLKNOWN_C
#include "cwellknown.h"
#endif
 
#if OPENWSN_CSENSORS_C
#include "csensors.h"
#endif

#if OPENWSN_RRT_C
#include "rrt.h"
#endif

#if OPENWSN_UECHO_C
#include "uecho.h"
#endif

#if OPENWSN_UINJECT_C
#include "uinject.h"
#endif

#if OPENWSN_USERIALBRIDGE_C
#include "userialbridge.h"
#endif

#if OPENWSN_UEXPIRATION_C
#include "uexpiration.h"
#endif

#if OPENWSN_UEXP_MONITOR_C
#include "uexpiration_monitor.h"
#endif

#if OPENWSN_CJOIN_C
#include "cjoin.h"
#endif

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openapps_init(void) {
#if OPENWSN_CJOIN_C
    cjoin_init();
#endif

#if OPENWSN_C6T_C
    c6t_init();
#endif

#if OPENWSN_CINFO_C
    cinfo_init();
#endif

#if OPENWSN_CLED_C
    cled_init();
#endif

#if OPENWSN_CWELLKNOWN_C
    cwellknown_init();
#endif

#if OPENWSN_CSENSORS_C
    csensors_init();
#endif

#if OPENWSN_RRT_C
    rrt_init();
#endif

#if OPENWSN_UECHO_C
    uecho_init();
#endif

#if OPENWSN_UINJECT_C
    uinject_init();
#endif

#if OPENWSN_USERIALBRIDGE_C
    userialbridge_init();
#endif

#if OPENWSN_UEXPIRATION_C
    uexpiration_init();
#endif

#if OPENWSN_UEXP_MONITOR_C
    umonitor_init();
#endif

}
