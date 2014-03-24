#ifndef __UDPPRINT_H
#define __UDPPRINT_H

/**
\addtogroup AppUdp
\{
\addtogroup udpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udpprint_init();
void udpprint_receive(OpenQueueEntry_t* msg);
void udpprint_sendDone(OpenQueueEntry_t* msg, owerror_t error);
bool udpprint_debugPrint();

/**
\}
\}
*/

#endif
