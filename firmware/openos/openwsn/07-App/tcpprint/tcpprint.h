#ifndef __TCPPRINT_H
#define __TCPPRINT_H

/**
\addtogroup AppTcp
\{
\addtogroup tcpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void tcpprint_init();
BOOL tcpprint_shouldIlisten();
void tcpprint_receive(OpenQueueEntry_t* msg);
void tcpprint_connectDone(owerror_t error);
void tcpprint_sendDone(OpenQueueEntry_t* msg, owerror_t error);
BOOL tcpprint_debugPrint();

/**
\}
\}
*/

#endif
