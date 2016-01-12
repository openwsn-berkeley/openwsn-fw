#ifndef __OTF_H
#define __OTF_H

/**
\addtogroup MAChigh
\{
\addtogroup otf
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      otf_init(void);
// notification from sixtop
void      otf_notif_addedCell(void);
void      otf_notif_removedCell(void);

//a packet is pushed to the MAC layer -> OTF notification
void otf_notif_transmit(OpenQueueEntry_t* msg);

//called to possibly update the schedule by OTF (e.g. sixtop has finished an allocation, has timeouted, etc.)
void     otf_update_schedule(void);

//the parent has changed, must now remove the corresponding cells
void     otf_notif_remove_parent(open_addr_t *parent);

/**
\}
\}
*/

#endif
