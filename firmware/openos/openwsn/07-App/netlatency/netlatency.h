#ifndef __NETLATENCY_H
#define __NETLATENCY_H

/**
\addtogroup App

\addtogroup NetLatency
\{
*/

//=========================== define ==========================================

/// inter-packet period (in mseconds)
#define NETLATENCYPERIOD     3000
#define NUMPKTTEST           299 // counter start from 0

static const uint8_t ipAddr_netLat_Serv[]  = {0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                              0x14, 0x15, 0x92, 0x00, 0x00, 0x15, 0x08, 0xaa};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void netlatency_init();
void netlatency_sendDone(OpenQueueEntry_t* msg, error_t error);
void netlatency_receive(OpenQueueEntry_t* msg);
bool debugPrint_netlatency();
void netlatency_task();

/**
\}
\}
*/

#endif
