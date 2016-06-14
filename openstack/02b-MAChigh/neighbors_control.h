#ifndef __NEIGHBORS_CONTROL_H
#define __NEIGHBORS_CONTROL_H

/**
\addtogroup MAChigh
\{
\addtogroup NEIGHBORS_CONTROL
\{
*/

#include "opendefs.h"
#include "opentimers.h"
#include "neighbors.h"

//=========================== define ==========================================

#define NEIGHBORSCONTROL  3
#define NEIGHBORSCONTROL_TIMERPERIOD        35000

//=========================== typedef =========================================

typedef struct {
  bool           used;  
  opentimer_id_t id;
  open_addr_t   neighbor;
}controlTimer_t;

//=========================== module variables ================================

typedef struct {
   uint16_t             periodMaintenance;
   controlTimer_t       timers[MAXNUMNEIGHBORS];
   uint16_t             periodCleanBlockedNeighbor;
   opentimer_id_t       cleanTimerId;
} neighbors_control_vars_t;

//=========================== prototypes ======================================

// admin
void      neighbors_control_init(void);
void      neighbors_control_startTimer(open_addr_t*   neighbor);
void      neighbors_control_cancelTimer(open_addr_t*  neighbor);

uint8_t   neighbors_control_getMyTimer(open_addr_t*   neighbor);
void      neighbors_control_removeTimer(open_addr_t*   neighbor);

/**
\}
\}
*/

#endif
