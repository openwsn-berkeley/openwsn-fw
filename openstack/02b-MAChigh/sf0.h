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

typedef struct {
   uint8_t numAppPacketsPerSlotFrame;
} sf0_vars_t;

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin

/*void     otf_init(void);


// notification from sixtop
void     otf_notif_addedCell(void);
void     otf_notif_removedCell(void);

//called to possibly update the schedule by OTF (e.g. sixtop has finished an allocation, has timeouted, etc.)
void     otf_verifSchedule(void);

//a packet is pushed to the MAC layer -> OTF notification
void     otf_notif_pktTx(OpenQueueEntry_t* msg);

//the parent has changed, must now remove the corresponding cells
void     otf_notif_parentRemoved(open_addr_t *parent);
*/

void      sf0_init(void);
// notification from sixtop
void      sf0_notif_addedCell(void);
void      sf0_notif_removedCell(void);
// notification from schedule
void      sf0_notifyNewSlotframe(void);
void      sf0_appPktPeriod(uint8_t numAppPacketsPerSlotFrame);


/**
\}
\}
*/

#endif
