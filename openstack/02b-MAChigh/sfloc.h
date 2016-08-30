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
