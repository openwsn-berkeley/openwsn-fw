#ifndef __UDPINJECT_H
#define __UDPINJECT_H

/**
\addtogroup App
\{
\addtogroup udpInject
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udpinject_init();
void udpinject_trigger();
void udpinject_sendDone(OpenQueueEntry_t* msg, ow_error_t error);
void udpinject_receive(OpenQueueEntry_t* msg);
bool udpinject_debugPrint();

/**
\}
\}
*/

#endif
