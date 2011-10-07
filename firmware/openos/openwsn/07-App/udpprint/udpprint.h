#ifndef __APPUDPPRINT_H
#define __APPUDPPRINT_H

/**
\addtogroup App
\{
\addtogroup AppUdpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpprint_init();
void appudpprint_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudpprint_receive(OpenQueueEntry_t* msg);
bool appudpprint_debugPrint();

/**
\}
\}
*/

#endif
