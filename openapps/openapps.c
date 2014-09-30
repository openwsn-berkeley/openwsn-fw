/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2012
*/

#include "openwsn.h"
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

void openapps_init(void) {
   
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
   //rrt_init();
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
}
