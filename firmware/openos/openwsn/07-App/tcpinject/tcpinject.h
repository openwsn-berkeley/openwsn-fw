#ifndef __TCPINJECT_H
#define __TCPINJECT_H

/**
\addtogroup App
\{
\addtogroup tcpInject
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void tcpinject_init();
bool tcpinject_shouldIlisten();
void tcpinject_trigger();
void tcpinject_connectDone(error_t error);
void tcpinject_sendDone(OpenQueueEntry_t* msg, error_t error);
void tcpinject_receive(OpenQueueEntry_t* msg);
bool tcpinject_debugPrint();

/**
\}
\}
*/

#endif
