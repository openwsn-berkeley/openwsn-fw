#ifndef __APPUDPCHANNEL_H
#define __APPUDPCHANNEL_H

/**
\addtogroup App
\{
\addtogroup AppUdpChannel
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpchannel_init();
void appudpchannel_trigger();
void appudpchannel_sendDone(OpenQueueEntry_t* msg, error_t error);
void appudpchannel_receive(OpenQueueEntry_t* msg);
bool appudpchannel_debugPrint();

/**
\}
\}
*/

#endif
