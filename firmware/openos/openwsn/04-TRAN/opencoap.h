#ifndef __OPENCOAP_H
#define __OPENCOAP_H

/**
\addtogroup Transport
\{
\addtogroup openCoap
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void opencoap_init();
void opencoap_receive(OpenQueueEntry_t* msg);
void opencoap_sendDone(OpenQueueEntry_t* msg, error_t error);

/**
\}
\}
*/

#endif
