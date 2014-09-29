#ifndef __HELI_H
#define __HELI_H

/**
\addtogroup AppUdp
\{
\addtogroup Heli
\{
*/


#include "openwsn.h"
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void heli_init(void);
void heli_trigger(void);
void heli_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void heli_receive(OpenQueueEntry_t* msg);
bool heli_debugPrint(void);

/**
\}
\}
*/

#endif
