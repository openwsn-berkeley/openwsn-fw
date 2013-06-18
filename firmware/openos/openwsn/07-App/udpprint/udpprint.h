#ifndef __UDPPRINT_H
#define __UDPPRINT_H

/**
\addtogroup App
\{
\addtogroup udpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udpprint_init();
void udpprint_sendDone(OpenQueueEntry_t* msg, ow_error_t error);
void udpprint_receive(OpenQueueEntry_t* msg);
bool udpprint_debugPrint();

/**
\}
\}
*/

#endif
