#ifndef __APPUDPTIMER_H
#define __APPUDPTIMER_H

/**
\addtogroup App
\{
\addtogroup AppUdpTimer
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudptimer_init();
void appudptimer_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudptimer_receive(OpenQueueEntry_t* msg);
bool appudptimer_debugPrint();

/**
\}
\}
*/

#endif
