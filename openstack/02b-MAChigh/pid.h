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
 
#define PID_NUM_INTEGRAL_HISTORY    10

#define TARGET_CELL_USAGE      90        // unit: percentage. x% cell are used for transmission 
#define TARGET_USAGE_RANGE      5        // uint: percentage. +/- 5% fluctuation is allowed

#define TARGET_PACKET_IN_QUEUE  2        // ideal number of packet in queue
#define TARGET_PACKET_RANGE     1        // uint: percentage. +/- 5% fluctuation is allowed

//#define PID_CELL_USAGE  // using packet to do pid calculation

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   int16_t prevError;
   int16_t errorHistory[PID_NUM_INTEGRAL_HISTORY]; // It is already multiplied by dt
   open_addr_t address;    
   int16_t control;
} pid_vars_t;


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
