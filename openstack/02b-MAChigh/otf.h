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
void     otf_init(void);


// notification from sixtop
void     otf_notif_addedCell(void);
void     otf_notif_removedCell(void);

//called to possibly update the schedule by OTF (e.g. sixtop has finished an allocation, has timeouted, etc.)
void     otf_verifSchedule(void);

//a packet is pushed to the MAC layer -> OTF notification
void     otf_notif_pktTx(OpenQueueEntry_t* msg);

//the parent has changed, must now remove the corresponding cells
void     otf_notif_parentRemoved(open_addr_t *parent);

/**
\}
\}
*/

#endif
