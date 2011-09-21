/**
\brief UDP WARPWING application

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, October 2010
*/

#ifndef __APPUDPWARPWING_H
#define __APPUDPWARPWING_H

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpwarpwing_init();
void appudpwarpwing_trigger();
void appudpwarpwing_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudpwarpwing_receive(OpenQueueEntry_t* msg);
bool appudpwarpwing_debugPrint();

#ifdef TASK_APPLICATION
void appudpwarpwing_task(uint16_t count);
#endif

#endif
