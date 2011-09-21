#ifndef __APPUDPSENSOR_H
#define __APPUDPSENSOR_H

/**
\addtogroup App
\{
\addtogroup AppUdpSensor
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpsensor_init();
void appudpsensor_trigger();
void appudpsensor_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudpsensor_receive(OpenQueueEntry_t* msg);
bool appudpsensor_debugPrint();

/**
\}
\}
*/

#endif
