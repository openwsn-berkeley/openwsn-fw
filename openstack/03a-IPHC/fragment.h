#ifndef __FRAGMENT_H
#define __FRAGMENT_H

/**
\addtogroup LoWPAN
\{
\addtogroup Fragment
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

#define FRAGMENT_MAX_SIZE 1280 // 1280 = IPv6 required
#define MIN_PAYLOAD 81         // Min 6LowPAN payload
#define FRAGMENT_MAX_FRAGMENTS (FRAGMENT_MAX_SIZE/MIN_PAYLOAD +1)

#define FRAGQLENGTH 5

#define LOWPAN_DISPATCH    5
enum LOWPAN_DISPATCH_enums {
   LOWPAN_DISPATCH_FRAG1 = 6,
   LOWPAN_DISPATCH_FRAGN = 7
};

typedef enum fragment_states {
   FRAGMENT_NONE,      // fresh fragment
                       // free Fragment entry
   FRAGMENT_ASSIGNED,  // assigned for an incoming or outgoing fragment
   // outgoing values
   FRAGMENT_RESERVING, // trying to acquire a OpenQueue packet
   FRAGMENT_RESERVED,  // payload copied to OpenQueue packet
                       // Fragment entry in use
   FRAGMENT_SENDING,   // packet attempted to be sent (on layer 2)
   FRAGMENT_SENT,      // fragment sent
   // incoming values
   FRAGMENT_PROCESSED, // payload processed but not freed
   FRAGMENT_FINISHED,  // fragment resources freed
   // Fragment Entry value
   FRAGMENT_RX,        // incoming message
   FRAGMENT_TX,        // outgoing message
   FRAGMENT_FAIL       // error on Tx, waiting for fragments on sending
} FragmentState;

#define FRAGMENT_TIMEOUT_MS     60000
#define FRAGMENT_TX_MAX_PACKETS     2

//=========================== typedef =========================================

typedef void (*fragment_action)(uint8_t buf);

typedef struct {
   opentimer_id_t    timer;
   OpenQueueEntry_t* pkt;
} fragment_timers_t[FRAGMENT_TX_MAX_PACKETS];

typedef struct {
   uint8_t           fragment_offset;
   uint8_t           fragment_size;
   FragmentState     state;
   OpenQueueEntry_t* fragment;
} FragmentOffsetEntry_t;

typedef struct FragmentQueueEntry_t {
   FragmentState         in_use;        // Record state
   OpenQueueEntry_t*     msg;           // Initial fragment message
   uint16_t              datagram_size; // RFC 4944 Section 5.3
   uint16_t              datagram_tag;
   open_addr_t           dst;           // i802.15.4 addresses or originator
   open_addr_t           src;           // and destination mesh addresses
   opentimer_id_t        timerId;
   fragment_action       action;
   uint8_t               number;        // number of fragments in list
   uint8_t               processed;     // number of assembled or sent
   uint8_t               sending;       // number on sending
   FragmentOffsetEntry_t list[FRAGMENT_MAX_FRAGMENTS];
} FragmentQueueEntry_t;

typedef struct {
   FragmentQueueEntry_t queue[FRAGQLENGTH];
   uint16_t             tag;
} fragmentqueue_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void fragment_init(void);
owerror_t fragment_prependHeader(OpenQueueEntry_t* msg);
void fragment_retrieveHeader(OpenQueueEntry_t* msg);
void fragment_sendDone(OpenQueueEntry_t *msg, owerror_t error);
FragmentQueueEntry_t* fragment_indexBuffer(uint8_t id);

/**
\}
\}
*/

#endif
