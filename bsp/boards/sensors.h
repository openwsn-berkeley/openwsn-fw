/**
    \brief Declaration of the "sensors" board-specif driver.
    \author Nicola Accettura <nicola.accettura@eecs.berkeley.edu>, March 2015.
*/

#ifndef __SENSORS_H__
#define __SENSORS_H__

//=========================== define ==========================================

/// define NUMSENSORS if not defined in board_info.h
#ifndef NUMSENSORS
#define NUMSENSORS 10
#endif  // NUMSENSORS

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

void sensors_init(void);

#endif // __SENSORS_H__
