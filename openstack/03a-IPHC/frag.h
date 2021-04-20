/**
\brief Definition of the "6LoWPAN fragmentation" module.


\author Timothy Claeys <timothy.claeys@inria.fr>, January 2020.
*/

#ifndef OPENWSN_FRAG_H
#define OPENWSN_FRAG_H

/**
\addtogroup LoWPAN
\{
\addtogroup FRAG
\{
*/

#include "config.h"

#include "opendefs.h"
#include "openqueue.h"
#include "opentimers.h"
#include "IEEE802154_security.h"

//=========================== define ==========================================

/* Pick a conservative maximum payload size for the fragments in case L2 security is active
 * If L2 security is not active MAX_FRAGMENT_SIZE can be set to 96 bytes, but the corresponding variable in
 * openvisualizer must then also be updated to the same value.
 */
#define MAX_FRAGMENT_SIZE           80

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

//=========================== variables ======================================= 

//=========================== prototypes ======================================

void frag_init(void);

void frag_sendDone(OpenQueueEntry_t *msg, owerror_t sendError);

void frag_receive(OpenQueueEntry_t *msg);

owerror_t frag_fragment6LoPacket(OpenQueueEntry_t *msg);

#endif /* OPENWSN_FRAG_H */
