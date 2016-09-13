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


//tracks are handled by 6top
#define SFLOC_REMOVE_OBSOLETE_PARENTS    // when a node is removed from the parent list, its associated cells are removed
#define SFLOC_REMOVE_UNUSED_CELLS        // a cell is removed when it is not used for a sufficient long time
#define SFLOC_CELL_TIMEOUT_RX         (2.5 * TIMER_DAO_TIMEOUT)        //ms before a cell in RX is considered unused if nothing is received
#define SFLOC_CELL_TIMEOUT_TX         (2 * TIMER_DAO_TIMEOUT)        //ms before a cell in TX is considered unused if nothing is transmitted



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
