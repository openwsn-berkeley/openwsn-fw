#ifndef __PID_H
#define __PID_H

/**
\addtogroup MAChigh
\{
\addtogroup otf
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

#define TARGET_CELL_USAGE      90        // unit: percentage. x% cell are used for transmission 
#define TARGET_USAGE_RANGE      5        // uint: percentage. +/- 5% fluctuation is allowed

#define TARGET_PACKET_IN_QUEUE  0        // ideal number of packet in queue

//#define PID_CELL_USAGE  // using packet to do pid calculation

//=========================== typedef =========================================

//=========================== module variables ================================


//=========================== prototypes ======================================

// admin
void pid_init();

// notification from sixtop
int16_t pid_compute_packetInQueue();
int16_t pid_compute_usageOfCell();

void pid_setNeighbor(open_addr_t* address);

/**
\}
\}
*/

#endif
