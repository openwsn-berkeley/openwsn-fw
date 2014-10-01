#ifndef __UECHO_H
#define __UECHO_H

/**
\addtogroup AppUdp
\{
\addtogroup uecho
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void uecho_init(void);
void uecho_receive(OpenQueueEntry_t* msg);
void uecho_sendDone(OpenQueueEntry_t* msg, owerror_t error);
bool uecho_debugPrint(void);

/**
\}
\}
*/

#endif
