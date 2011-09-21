#ifndef __APPUDPLEDS_H
#define __APPUDPLEDS_H

/**
\addtogroup App
\{
\addtogroup AppUdpLeds
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpleds_init();
void appudpleds_trigger();
void appudpleds_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudpleds_receive(OpenQueueEntry_t* msg);
bool appudpleds_debugPrint();

/**
\}
\}
*/

#endif
