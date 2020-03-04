/**
\brief Applications running on top of the OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2014.
\author Timothy Claeys <timothy.claeys@inria.fr>, March 2020.
*/

#include "config.h"
#include "opendefs.h"

#if defined(OPENWSN_C6T_C)
#include "c6t.h"
#endif

#if defined(OPENWSN_CLED_C)
#include "cled.h"
#endif

#if defined(OPENWSN_CINFO_C)
#include "cinfo.h"
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
#include "cwellknown.h"
#endif

#if defined(OPENWSN_RRT_C)
#include "rrt.h"
#endif

#if defined(OPENWSN_UECHO_C)
#include "uecho.h"
#endif

#if defined(OPENWSN_UINJECT_C)
#include "uinject.h"
#endif

#if defined(OPENWSN_USERIALBRIDGE_C)
#include "userialbridge.h"
#endif

#if defined(OPENWSN_UEXPIRATION_C)
#include "uexpiration.h"
#endif

#if defined(OPENWSN_UEXPIRATION_MONITOR_C)
#include "uexpiration_monitor.h"
#endif

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================

void openapps_init(void) {

#if defined(OPENWSN_C6T_C)
    c6t_init();
#endif

#if defined(OPENWSN_CINFO_C)
    cinfo_init();
#endif

#if defined(OPENWSN_CLED_C)
    cled_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    cwellknown_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    rrt_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    uecho_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    uinject_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    userialbridge_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    uexpiration_init();
#endif

#if defined(OPENWSN_CWELLKNOWN_C)
    umonitor_init();
#endif

}
