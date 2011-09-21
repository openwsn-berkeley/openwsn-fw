#ifndef __APPUDPGINA_H
#define __APPUDPGINA_H

/**
\addtogroup App
\{
\addtogroup AppUdpGina
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpgina_init();
void appudpgina_trigger();
void appudpgina_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudpgina_receive(OpenQueueEntry_t* msg);
bool appudpgina_debugPrint();

/**
\}
\}
*/

#endif
