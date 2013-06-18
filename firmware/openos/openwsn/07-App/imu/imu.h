#ifndef __IMU_H
#define __IMU_H

/**
\addtogroup App
\{
\addtogroup imu
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void imu_init();
void imu_trigger();
void imu_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void imu_receive(OpenQueueEntry_t* msg);
bool imu_debugPrint();

/**
\}
\}
*/

#endif
