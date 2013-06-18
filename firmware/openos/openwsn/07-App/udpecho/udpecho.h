#ifndef __UDPECHO_H
#define __UDPECHO_H

/**
\addtogroup App
\{
\addtogroup udpEcho
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udpecho_init();
void udpecho_receive(OpenQueueEntry_t* msg);
void udpecho_sendDone(OpenQueueEntry_t* msg, ow_error_t error);
bool udpecho_debugPrint();

/**
\}
\}
*/

#endif
