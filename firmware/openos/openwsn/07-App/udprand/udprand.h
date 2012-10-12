#ifndef __UDPRAND_H
#define __UDPRAND_H

/**
\addtogroup App

\addtogroup udpRand
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udprand_init();
void udprand_trigger();
void udprand_sendDone(OpenQueueEntry_t* msg, error_t error);
void udprand_receive(OpenQueueEntry_t* msg);
bool udprand_debugPrint();
void udprand_task();

/**
\}
\}
*/

#endif
