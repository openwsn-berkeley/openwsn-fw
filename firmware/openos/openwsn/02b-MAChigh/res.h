#ifndef __RES_H
#define __RES_H

/**
\addtogroup MAChigh
\{
\addtogroup RES
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   uint16_t        periodMaintenance;
   bool            busySendingKa;        // TRUE when busy sending a keep-alive
   bool            busySendingAdv;       // TRUE when busy sending an advertisement
   uint8_t         dsn;                  // current data sequence number
   uint8_t         MacMgtTaskCounter;    // counter to determine what management task to do
   opentimer_id_t  timerId;
   uint16_t        kaPeriod;             // period of sending KA
} res_vars_t;

//=========================== prototypes ======================================

void    res_init();
bool    debugPrint_myDAGrank();
// from upper layer
owerror_t res_send(OpenQueueEntry_t *msg);
// from lower layer
void    task_resNotifSendDone();
void    task_resNotifReceive();
void    res_setKaPeriod(uint16_t kaPeriod);

/**
\}
\}
*/

#endif