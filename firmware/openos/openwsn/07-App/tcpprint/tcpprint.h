#ifndef __TCPPRINT_H
#define __TCPPRINT_H

/**
\addtogroup App
\{
\addtogroup tcpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void tcpprint_init();
bool tcpprint_shouldIlisten();
void tcpprint_receive(OpenQueueEntry_t* msg);
void tcpprint_connectDone(ow_error_t error);
void tcpprint_sendDone(OpenQueueEntry_t* msg, ow_error_t error);
bool tcpprint_debugPrint();

/**
\}
\}
*/

#endif
