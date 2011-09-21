#ifndef __APPTCPPRINT_H
#define __APPTCPPRINT_H

/**
\addtogroup App
\{
\addtogroup AppTcpPrint
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void apptcpprint_init();
bool apptcpprint_shouldIlisten();
void apptcpprint_receive(OpenQueueEntry_t* msg);
void apptcpprint_connectDone(error_t error);
void apptcpprint_sendDone(OpenQueueEntry_t* msg, error_t error);
bool apptcpprint_debugPrint();

/**
\}
\}
*/

#endif
