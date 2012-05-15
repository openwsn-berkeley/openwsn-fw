#ifndef __OHLONE_H
#define __OHLONE_H

/**
\addtogroup App
\{
\addtogroup ohlone
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void ohlone_init();
bool ohlone_shouldIlisten();
void ohlone_receive(OpenQueueEntry_t* msg);
void ohlone_sendDone(OpenQueueEntry_t* msg, error_t error);
void ohlone_connectDone(error_t error);
bool ohlone_debugPrint();

/**
\}
\}
*/

#endif
