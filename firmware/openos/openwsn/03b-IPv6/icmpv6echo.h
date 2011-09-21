#ifndef __ICMPv6ECHO_H
#define __ICMPv6ECHO_H

/**
\addtogroup IPv6
\{
\addtogroup ICMPv6Echo
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void icmpv6echo_init();
void icmpv6echo_trigger();
void icmpv6echo_sendDone(OpenQueueEntry_t* msg, error_t error);
void icmpv6echo_receive(OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
