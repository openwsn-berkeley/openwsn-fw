#ifndef __TECHO_H
#define __TECHO_H

/**
\addtogroup AppTcp
\{
\addtogroup techo
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void techo_init(void);
bool techo_shouldIlisten(void);
void techo_receive(OpenQueueEntry_t* msg);
void techo_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void techo_connectDone(owerror_t error);
bool techo_debugPrint(void);

/**
\}
\}
*/

#endif
