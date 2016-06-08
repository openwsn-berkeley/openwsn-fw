#ifndef __OTF_H
#define __OTF_H

/**
\addtogroup MAChigh
\{
\addtogroup otf
\{
*/

#include "opendefs.h"
#include "opentimers.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   uint16_t             periodMaintenance;
   opentimer_id_t       maintenanceTimerId;
} neighbors_control_vars_t;

//=========================== prototypes ======================================

// admin
void      neighbors_control_init(void);

/**
\}
\}
*/

#endif
