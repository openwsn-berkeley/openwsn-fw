#ifndef __UDPRAND_H
#define __UDPRAND_H

/**
\addtogroup AppUdp
\{
\addtogroup UdpRand
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udprand_init();
void udprand_trigger();
void udprand_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void udprand_receive(OpenQueueEntry_t* msg);
BOOL udprand_debugPrint();
void udprand_task();

/**
\}
\}
*/

#endif
