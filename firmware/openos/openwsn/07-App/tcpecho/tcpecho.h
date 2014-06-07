#ifndef __TCPECHO_H
#define __TCPECHO_H

/**
\addtogroup AppTcp
\{
\addtogroup tcpEcho
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void tcpecho_init();
bool tcpecho_shouldIlisten();
void tcpecho_receive(OpenQueueEntry_t* msg);
void tcpecho_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void tcpecho_connectDone(owerror_t error);
bool tcpecho_debugPrint();

/**
\}
\}
*/

#endif
