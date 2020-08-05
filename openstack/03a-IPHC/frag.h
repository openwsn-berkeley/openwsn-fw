/**
\brief Definition of the "6LoWPAN fragmentation" module.


\author Timothy Claeys <timothy.claeys@inria.fr>, January 2020.
*/

#ifndef __FRAG_H
#define __FRAG_H

/**
\addtogroup LoWPAN
\{
\addtogroup FRAG
\{
*/


#include "opendefs.h"
#include "openqueue.h"
#include "opentimers.h"
#include "IEEE802154_security.h"

//=========================== define ==========================================

/* Pick a conservative maximum payload size for the fragments in case L2 security is active
 * If L2 security is not active MAX_FRAGMENT_SIZE can be set to 96 bytes, but the corresponding variable in
 * openvisualizer must then also be updated to the same value.
 */
#define MAX_FRAGMENT_SIZE           96

#define FRAGMENT_BUFFER_SIZE        (((IPV6_PACKET_SIZE / MAX_FRAGMENT_SIZE) + 1) * BIGQUEUELENGTH)
#define NUM_OF_VRBS                 2
#define NUM_OF_CONCURRENT_TIMERS    (NUM_OF_VRBS + BIGQUEUELENGTH)

#define FRAG1_HEADER_SIZE           4
#define FRAGN_HEADER_SIZE           5

#define DISPATCH_FRAG_FIRST         24
#define DISPATCH_FRAG_SUBSEQ        28

#define OFFSET_MULTIPLE             8

// specifies how long we store fragments or keep vrb allocated
#define FRAG_REASSEMBLY_TIMEOUT     60000

// 6LoWPAN fragment1 header
typedef struct {
    uint16_t dispatch_size_field;
    uint16_t datagram_tag;
} frag1_t;

// 6LoWPAN fragmentN header
typedef struct {
    uint16_t dispatch_size_field;
    uint16_t datagram_tag;
    uint8_t datagram_offset;
} fragn_t;

/*
 * Describes an entry in the fragment buffer, contains:
 * - If lock is TRUE, fragment is scheduled for Tx (do not delete until cb sendDone!!).
 * - The fragment offset value (multiple of 8)
 * - The tag value used for this fragment.
 * - The reassembly timer (60s after the arrival of the first fragment, reassembly must be completed).
 * - A pointer to the fragment's location in the OpenQueue.
 * - A pointer to the original unfragmented 6LoWPAN packet in the OpenQueue.
*/
BEGIN_PACK
struct fragment_t {
    bool lock;
    uint8_t datagram_offset;
    uint16_t datagram_tag;
    opentimers_id_t reassembly_timer;
    OpenQueueEntry_t *pFragment;
    OpenQueueEntry_t *pOriginalMsg;
};
END_PACK

typedef struct fragment_t fragment;

// virtual reassembly buffer struct
BEGIN_PACK
typedef struct {
    uint16_t tag;
    uint16_t left;
    uint16_t size;
    opentimers_id_t forward_timer;
    OpenQueueEntry_t *frag1;
    open_addr_t nexthop;
} vrb_t;
END_PACK

// state information for fragmentation
typedef struct {
    uint16_t global_tag;
    vrb_t vrbs[NUM_OF_VRBS];
    fragment fragmentBuf[FRAGMENT_BUFFER_SIZE];
    opentimers_id_t frag_timerq[NUM_OF_CONCURRENT_TIMERS];
} frag_vars_t;



//=========================== variables ======================================= 

//=========================== prototypes ======================================

void frag_init(void);

void frag_sendDone(OpenQueueEntry_t *msg, owerror_t sendError);

void frag_receive(OpenQueueEntry_t *msg);

owerror_t frag_fragment6LoPacket(OpenQueueEntry_t *msg);

#endif
