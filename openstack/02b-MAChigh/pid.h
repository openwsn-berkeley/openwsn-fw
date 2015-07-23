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

#define PID_PROPORTIONAL_GAIN_VALUE 1 //4.0
#define PID_INTEGRAL_GAIN_VALUE     0 //5.0
#define PID_DERIVATIVE_GAIN_VALUE   0 //3.0

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
int16_t pid_compute();

void pid_setNeighbor(open_addr_t* address);

/**
\}
\}
*/

#endif
