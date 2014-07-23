#ifndef __UDPLATENCY_H
#define __UDPLATENCY_H

/**
\addtogroup AppUdp
\{
\addtogroup UdpLatency
\{
*/

//=========================== define ==========================================

/// inter-packet period (in mseconds)
#define UDPLATENCYPERIOD     2000
#define NUMPKTTEST           300

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void udplatency_init(void);
void udplatency_trigger(void);
void udplatency_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void udplatency_receive(OpenQueueEntry_t* msg);
bool udplatency_debugPrint(void);
void udplatency_task(void);

/**
\}
\}
*/

#endif
