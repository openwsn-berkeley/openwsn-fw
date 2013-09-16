#ifndef __HELI_H
#define __HELI_H

/**
\addtogroup AppUdp
\{
\addtogroup Heli
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void heli_init();
void heli_trigger();
void heli_sendDone(OpenQueueEntry_t* msg, error_t error);
void heli_receive(OpenQueueEntry_t* msg);
bool heli_debugPrint();

/**
\}
\}
*/

#endif
