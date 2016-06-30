#ifndef __SF0_H
#define __SF0_H

/**
\addtogroup MAChigh
\{
\addtogroup sf0
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      sf0_init(void);
// notification from sixtop
void      sf0_notif_addedCell(void);
void      sf0_notif_removedCell(void);
// notification from schedule
void      sf0_notifyNewSlotframe(void);
/**
\}
\}
*/

#endif
