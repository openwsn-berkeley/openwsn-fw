#ifndef __NETLEDS_H
#define __NETLEDS_H

/**
\addtogroup App
\{
\addtogroup netLeds
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void netleds_init();
void netleds_trigger();
void netleds_sendDone(OpenQueueEntry_t* msg, error_t error);
void netleds_receive(OpenQueueEntry_t* msg);
bool netleds_debugPrint();

/**
\}
\}
*/

#endif
