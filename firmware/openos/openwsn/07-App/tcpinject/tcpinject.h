#ifndef __TCPINJECT_H
#define __TCPINJECT_H

/**
\addtogroup AppTcp
\{
\addtogroup tcpInject
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   OpenQueueEntry_t*    pkt;
   BOOL                 sending;
   open_addr_t          hisAddress;
   uint16_t             hisPort;
} tcpinject_vars_t;

//=========================== prototypes ======================================

void tcpinject_init();
BOOL tcpinject_shouldIlisten();
void tcpinject_trigger();
void tcpinject_connectDone(owerror_t error);
void tcpinject_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void tcpinject_receive(OpenQueueEntry_t* msg);
BOOL tcpinject_debugPrint();

/**
\}
\}
*/

#endif
