#ifndef __APPUDPECHO_H
#define __APPUDPECHO_H

/**
\addtogroup App
\{
\addtogroup AppUdpEcho
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpecho_init();
void appudpecho_receive(OpenQueueEntry_t* msg);
void appudpecho_sendDone(OpenQueueEntry_t* msg, error_t error);
bool appudpecho_debugPrint();

/**
\}
\}
*/

#endif
