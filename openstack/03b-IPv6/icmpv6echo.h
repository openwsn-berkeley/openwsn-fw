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

//=========================== module variables ================================

typedef struct {
   bool        busySending;
   open_addr_t hisAddress;
   uint16_t    seq;
   bool        isReplyEnabled;
} icmpv6echo_vars_t;

//=========================== prototypes ======================================

void icmpv6echo_init(void);
void icmpv6echo_trigger(void);
void icmpv6echo_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void icmpv6echo_receive(OpenQueueEntry_t* msg);
void icmpv6echo_setIsReplyEnabled(bool isEnabled);

/**
\}
\}
*/

#endif
