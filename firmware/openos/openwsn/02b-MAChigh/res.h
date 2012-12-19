#ifndef __RES_H
#define __RES_H

/**
\addtogroup MAChigh
\{
\addtogroup RES
\{
*/
#include "openwsn.h"                                // needed for uin8_t, uint16_t
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void    res_init();
bool    debugPrint_myDAGrank();
// from upper layer
error_t res_send(OpenQueueEntry_t *msg);
// from lower layer
void    task_resNotifSendDone();
void    res_sendDone(OpenQueueEntry_t* msg, error_t error);
void    task_resNotifReceive();
void    res_receive(OpenQueueEntry_t* msg);
// from processIE
asn_t      res_getADVasn();
uint8_t    res_getJoinPriority();
void    res_notifRetrieveIEDone(OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif