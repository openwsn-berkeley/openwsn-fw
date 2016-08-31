#ifndef __sfloc_H
#define __sfloc_H

/**
\addtogroup MAChigh
\{
\addtogroup sfloc
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct {
   uint8_t numAppPacketsPerSlotFrame;
} sfloc_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      sfloc_init(void);
// notification from sixtop
void      sfloc_notif_addedCell(void);
void      sfloc_notif_removedCell(void);
// notification from schedule
void      sfloc_notifyNewSlotframe(void);
void      sfloc_appPktPeriod(uint8_t numAppPacketsPerSlotFrame);
void      sfloc_verifSchedule(void);
void      sfloc_notif_pktTx(OpenQueueEntry_t* msg);



/**
\}
\}
*/

#endif
