#ifndef __FRAG_H
#define __FRAG_H

#include "opendefs.h"
#include "openqueue.h"
#include "opentimers.h"

//=========================== define ==========================================


#define FRAGMENT_BUFFER_SIZE        15
#define MAX_FRAGMENT_SIZE           96
#define NUM_OF_VRBS                 3
#define MAX_PACKET_SIZE             (FRAGMENT_BUFFER_SIZE * MAX_FRAGMENT_SIZE)

#define FRAG1_HEADER_SIZE           4
#define FRAGN_HEADER_SIZE           5

#define DISPATCH_FRAG_FIRST         24
#define DISPATCH_FRAG_SUBSEQ        28

#define FRAG_REASSEMBLY_TIMEOUT     60000

// 6lowpan fragment1 header
typedef struct {
    uint16_t dispatch_size_field;
    uint16_t datagram_tag;
} frag1_t;

// 6lowpan fragmentN header
typedef struct {
    uint16_t dispatch_size_field;
    uint16_t datagram_tag;
    uint8_t datagram_offset;
} fragn_t;

/*
* Describes an entry in the fragment buffer, contains:
* - if lock is TRUE, fragment is scheduled for Tx (do not delete until cb sendDone!!)
* - the tag value used for this fragment
* - a pointer to the fragment
* - a pointer to the original 6lowpan packet
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
typedef struct {
    uint16_t tag;
    uint16_t left;
    uint16_t size;
    OpenQueueEntry_t *frag1;
    open_addr_t nexthop;
} vrb_t;

// state information for fragmentation
typedef struct {
    uint16_t tag;
    vrb_t vrbs[NUM_OF_VRBS];
    fragment fragmentBuf[FRAGMENT_BUFFER_SIZE];
} frag_vars_t;



//=========================== variables ======================================= 

//=========================== prototypes ======================================

void frag_init(void);

void frag_sendDone(OpenQueueEntry_t *msg, owerror_t sendError);

void frag_receive(OpenQueueEntry_t *msg);

owerror_t frag_fragment6LoPacket(OpenQueueEntry_t *msg);

#endif
