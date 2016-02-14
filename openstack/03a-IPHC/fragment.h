#ifndef __FRAGMENT_H
#define __FRAGMENT_H

/**
\addtogroup LoWPAN
\{
\addtogroup Fragment
\{
*/

#include "opendefs.h"
#include "ieee802154_security_driver.h"

//=========================== define ==========================================
 
#define MIN_PAYLOAD 81         // Min 6LowPAN payload
#define FRAGMENT_MAX_FRAGMENTS (LARGE_PACKET_SIZE/MIN_PAYLOAD +1)
#define FRAGMENT_DATA_UTIL     (FRAME_DATA_DATA - IEEE802154_SECURITY_TAG_LEN)
#define FRAGMENT_DATA_INIT     (FRAME_DATA_PLOAD - IEEE802154_SECURITY_TAG_LEN)

#define FRAGQLENGTH 5

#define FRAGMENT_FRAG1_HL 4 // Fragment header length
#define FRAGMENT_FRAGN_HL 5

typedef enum fragment_states {
   FRAGMENT_NONE,      // fresh fragment
                       // free Fragment entry: first state
   // incoming values
   FRAGMENT_RECEIVED,  // received message fragment
   FRAGMENT_PROCESSED, // payload processed but not freed
   FRAGMENT_FINISHED,  // message fragment sent
                       // no attached packet: last state
   // Fragment Entry value
   FRAGMENT_RESERVED,  // reserved fragment for RX or TX
   FRAGMENT_RX,        // incoming message
   FRAGMENT_TX,        // outgoing message
   FRAGMENT_FW,        // forwarding message
} FragmentState;

typedef enum fragment_actions {
   FRAGMENT_ACTION_NONE,
   FRAGMENT_ACTION_CANCEL,    // cancel message
   FRAGMENT_ACTION_ASSEMBLE,  // message for me, moving to upper layer
   FRAGMENT_ACTION_FORWARD,   // message to be forwarded
   FRAGMENT_ACTION_OPENBRIDGE // to openbridge
} FragmentAction;

// SERFRAME_MOTE2PC_BRIDGE
// To add to https://openwsn.atlassian.net/wiki/display/OW/Serial+Format
// This type of frame is used by the DAGroot mote to inform about
// fragmented messages on openbridge. The payload is parsed by the
// OpenVisualizer and dispatch the corresponding message to Fragment.
// The format of this frame is:
// type of command (1B), type of message (1B), identifier (arbitrary)
// There exists two types of commands
// - 'F'- management is done here and it represents an error on communication
//   (duplicate fragment, time expiration or E_FAIL on transmission): discard
//   this message on openVisualizer
// - 'C' - inform to openVisualizer that last message has been sent
//   and a new one can be moved to the Mesh as only FRAGMENT_TX_MAX_PACKETS
//   are allowed simuoultaneously
// The type of message identifies its direction: fromMesh ('F') or
// toMesh ('T') :
// The identifier refers to the message:
// - tag (2B) + size (2b) + source (8B) when fromMesh
// - tag (2B) when toMesh

#define FRAGMENT_MOTE2PC_FAIL     ((uint8_t)'F')
#define FRAGMENT_MOTE2PC_SENDDONE ((uint8_t)'S')
#define FRAGMENT_MOTE2PC_FROMMESH ((uint8_t)'F')
#define FRAGMENT_MOTE2PC_TOMESH   ((uint8_t)'T')

#define FRAGMENT_TIMEOUT_MS     60000
#define FRAGMENT_NOTIMER        TOO_MANY_TIMERS_ERROR-1

//=========================== typedef =========================================

// Data to track forwarding and incoming fragmented messages
typedef struct {
   uint8_t           fragment_offset;
   uint8_t           fragment_size;
   FragmentState     state;
   OpenQueueEntry_t* fragment;
} FragmentOffsetEntry_t;

typedef union {
   // Data to track outgoing fragmented messages
   struct {
      // to fragment not big packets: it is too large; actually only 8B
      // are needed but implementation is simplier when copying message
      // to this buffer and making "big" points it
      uint8_t  excess[FRAME_DATA_DATA];
      uint8_t* payload;           // message payload
      uint8_t  max_fragment_size;
      uint16_t actual_sent;       // data sent
      uint8_t  size;              // next fragment size
      bool     fragn;             // True if not first fragment
   } data;
   FragmentOffsetEntry_t list[FRAGMENT_MAX_FRAGMENTS];
} FragmentOtherData_t;

typedef struct FragmentQueueEntry {
   FragmentState       in_use;        // Record state
   OpenQueueEntry_t*   msg;           // Initial fragment message
   uint8_t             creator;       // the message creator component
   uint16_t            datagram_size; // RFC 4944 Section 5.3
   uint16_t            datagram_tag;
   open_addr_t         dst;           // i802.15.4 addresses or originator
   open_addr_t         src;           // and destination mesh addresses
   opentimer_id_t      timerId;
   FragmentAction      action;        // action to process fragments
   uint8_t             number;        // number of fragments in list
   int8_t              offset;        // fragment offset
   FragmentOtherData_t other;
} FragmentQueueEntry_t;

typedef struct {
   FragmentQueueEntry_t queue[FRAGQLENGTH];
   uint16_t             tag;
} fragmentqueue_vars_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void fragment_init(void);
owerror_t fragment_prependHeader(OpenQueueEntry_t* msg);
bool fragment_retrieveHeader(OpenQueueEntry_t* msg);
void fragment_sendDone(OpenQueueEntry_t *msg, owerror_t error);
FragmentQueueEntry_t* fragment_searchBufferFromMsg(OpenQueueEntry_t* msg);
void fragment_assignAction(FragmentQueueEntry_t* buffer, FragmentAction action);
void fragment_checkOpenBridge(OpenQueueEntry_t *msg, owerror_t error);
void fragment_deleteNeighbor(open_addr_t* neighbor);

/**
\}
\}
*/

#endif
