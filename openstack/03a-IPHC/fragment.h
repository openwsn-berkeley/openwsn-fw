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

#define FRAGMENT_MAX_SIZE LARGE_PACKET_SIZE
#define MIN_PAYLOAD 81         // Min 6LowPAN payload
#define FRAGMENT_MAX_FRAGMENTS (FRAGMENT_MAX_SIZE/MIN_PAYLOAD +1)

#define FRAGQLENGTH 15

#define LOWPAN_DISPATCH    5
enum LOWPAN_DISPATCH_enums {
   LOWPAN_DISPATCH_FRAG1 = 6,
   LOWPAN_DISPATCH_FRAGN = 7
};

typedef enum fragment_states {
   FRAGMENT_NONE,      // fresh fragment
                       // free Fragment entry: first state
   // incoming values
   FRAGMENT_RECEIVED,  // received message fragment
   FRAGMENT_PROCESSED, // payload processed but not freed
// FRAGMENT_FINISHED,  // fragment resources freed
   // outgoing values
   FRAGMENT_ASSIGNED,  // assigned for an outgoing fragment
   FRAGMENT_RESERVING, // trying to acquire a OpenQueue packet
   FRAGMENT_RESERVED,  // payload copied to OpenQueue packet
                       // message fragment ready to be forwarded
                       // message fragment ready to be sent
   FRAGMENT_SENDING,   // packet attempted to be sent (on layer 2)
   FRAGMENT_FINISHED,  // message fragment sent
                       // no attached packet: last state
   // Fragment Entry value
   FRAGMENT_RX,        // incoming message
   FRAGMENT_TX,        // outgoing message
   FRAGMENT_FW,        // forwarding message
   FRAGMENT_FAIL,      // error on Tx, waiting for fragments on sending
   FRAGMENT_FAIL_FW
} FragmentState;

typedef enum fragment_actions {
   FRAGMENT_ACTION_NONE,
   FRAGMENT_ACTION_CANCEL,    // cancel message
   FRAGMENT_ACTION_ASSEMBLE,  // message for me, moving to upper layer
   FRAGMENT_ACTION_FORWARD,   // message to be forwarded
   FRAGMENT_ACTION_OPENBRIDGE // to openbridge
} FragmentAction;

#define FRAGMENT_TIMEOUT_MS     60000
#define FRAGMENT_TX_MAX_PACKETS     1

//=========================== typedef =========================================

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

typedef struct FragmentQueueEntry {
   FragmentState         in_use;        // Record state
   OpenQueueEntry_t*     msg;           // Initial fragment message
   uint16_t              datagram_size; // RFC 4944 Section 5.3
   uint16_t              datagram_tag;
   uint16_t              new_size;      // forwarded msg size & tag
   uint16_t              new_tag;
   open_addr_t           dst;           // i802.15.4 addresses or originator
   open_addr_t           src;           // and destination mesh addresses
   opentimer_id_t        timerId;
   FragmentAction        action;        // action to process fragments
   uint8_t               number;        // number of fragments in list
   uint8_t               processed;     // number of assembled or ready to forward
   uint8_t               sending;       // number on sending
   uint8_t               sent;          // number of sent
   uint8_t               offset;        // fragment offset
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
FragmentQueueEntry_t* fragment_searchBufferFromMsg(OpenQueueEntry_t* msg);
void fragment_assignAction(FragmentQueueEntry_t* buffer, FragmentAction action);

/**
\}
\}
*/

#endif
