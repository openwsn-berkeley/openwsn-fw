#ifndef __ICMPv6RPL_H
#define __ICMPv6RPL_H

/**
\addtogroup IPv6
\{
\addtogroup ICMPv6RPL
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void icmpv6rpl_init();
void icmpv6rpl_trigger();
void icmpv6rpl_sendDone(OpenQueueEntry_t* msg, error_t error);
void icmpv6rpl_receive(OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
