#ifndef __UDPTIMER_H
#define __UDPTIMER_H

/**
\addtogroup App
\{
\addtogroup udpTimer
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udptimer_init();
void udptimer_sendDone(OpenQueueEntry_t* msg, error_t error);
void udptimer_receive(OpenQueueEntry_t* msg);
bool udptimer_debugPrint();

/**
\}
\}
*/

#endif
