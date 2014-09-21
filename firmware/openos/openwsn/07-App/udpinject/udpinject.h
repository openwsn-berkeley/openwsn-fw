#ifndef __UDPINJECT_H
#define __UDPINJECT_H

/**
\addtogroup AppUdp
\{
\addtogroup udpInject
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udpinject_init(void);
void udpinject_trigger(void);
void udpinject_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void udpinject_receive(OpenQueueEntry_t* msg);
bool udpinject_debugPrint(void);

/**
\}
\}
*/

#endif
