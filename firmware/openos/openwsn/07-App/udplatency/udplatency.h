#ifndef __UDPLATENCY_H
#define __UDPLATENCY_H

/**
\addtogroup App

\addtogroup udpLatency
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udplatency_init();
void udplatency_trigger();
void udplatency_sendDone(OpenQueueEntry_t* msg, error_t error);
void udplatency_receive(OpenQueueEntry_t* msg);
bool udplatency_debugPrint();
void udplatency_task();

/**
\}
\}
*/

#endif
