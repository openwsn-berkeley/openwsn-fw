#ifndef __SFX_H
#define __SFX_H

/**
\addtogroup MAChigh
\{
\addtogroup sfx
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

#define CELL_USAGE_CALCULATION_WINDOWS  5

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
  uint8_t periodMaintenance;   // in slotframe
}sfx_vars_t;

//=========================== prototypes ======================================

// admin
void      sfx_init(void);
// notification from sixtop
void      sfx_notif_addedCell(void);
void      sfx_notif_removedCell(void);
void      sfx_notifyNewSlotframe(void);

/**
\}
\}
*/

#endif
