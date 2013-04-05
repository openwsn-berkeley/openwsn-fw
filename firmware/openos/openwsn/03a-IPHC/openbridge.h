#ifndef __OPENBRIDGE_H
#define __OPENBRIDGE_H

/**
\addtogroup LoWPAN
\{
\addtogroup OpenBridge
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void openbridge_init();
void openbridge_triggerData();
void openbridge_sendDone(OpenQueueEntry_t* msg, error_t error);
void openbridge_receive(OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
