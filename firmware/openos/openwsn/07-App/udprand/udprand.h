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

void udprand_init(void);
void udprand_trigger(void);
void udprand_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void udprand_receive(OpenQueueEntry_t* msg);
bool udprand_debugPrint(void);
void udprand_task(void);

/**
\}
\}
*/

#endif
