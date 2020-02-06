/**
\brief Definition of the "6LoWPAN fragmentation" module.

This module implements 6LoWPAN fragmentation according to RFC 4944,
 RFC 6282 and https://hal.inria.fr/hal-02061838/document.

\author Timothy Claeys <timothy.claeys@inria.fr>, January 2020.
*/


#include "opendefs.h"
#include "frag.h"
#include "iphc.h"
#include "openbridge.h"
#include "sixtop.h"
#include "openrandom.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "board.h"
#include "opentimers.h"
#include "scheduler.h"

//============================ defines ========================================

#define DISPATCH_SHIFT      11
#define DISPATCH_MASK       0x1F
#define SIZE_MASK           0x7FF

//============================= macros ========================================

#define LOCK(fragment)      ((fragment).lock = TRUE)
#define UNLOCK(fragment)    ((fragment).lock = FALSE)
#define ISLOCKED(fragment)  ((fragment).lock == TRUE)

#define FRAG_OFFSET(fragment) (

#define RESET_FRAG_BUFFER_ENTRY(i) \
    do { \
        if (ISLOCKED(frag_vars.fragmentBuf[i]) == FALSE) { \
            openqueue_freePacketBuffer(frag_vars.fragmentBuf[i].pFragment); \
            memset(&frag_vars.fragmentBuf[i], 0, sizeof(fragment)); \
        } \
    } while (0)


#define CHECK_OVERSIZED(size) \
    do { \
        if ((size) > (IPV6_PACKET_SIZE)) { \
            openqueue_freePacketBuffer(msg); \
            openserial_printError( \
                COMPONENT_FRAG, \
                ERR_FRAG_INVALID_SIZE, \
                (errorparameter_t) (size), \
                (errorparameter_t) IPV6_PACKET_SIZE \
            ); \
            return; \
        } \
    } while (0)

//=========================== variables =======================================

frag_vars_t frag_vars;

//=========================== prototypes ======================================

static void cleanup_fragments(uint16_t datagram_tag);

static void store_fragment(OpenQueueEntry_t *msg, uint16_t size, uint16_t tag, uint8_t offset);

static void reassemble_fragments(uint16_t tag, uint16_t size, OpenQueueEntry_t *reassembled_msg);

static owerror_t allocate_vrb(OpenQueueEntry_t *frag1, uint16_t size, uint16_t tag);

static void prepend_frag1_header(OpenQueueEntry_t *frag1, uint16_t size, uint16_t tag);

static void prepend_fragn_header(OpenQueueEntry_t *fragn, uint16_t size, uint16_t tag, uint8_t offset);

static void fast_forward_frags(uint16_t tag, uint16_t size, uint8_t vrb_pos);

void frag_timeout_cb(opentimers_id_t id);

owerror_t frag_timerq_enqueue(opentimers_id_t id);

owerror_t frag_timerq_remove(opentimers_id_t id);

opentimers_id_t frag_timerq_dequeue(void);
//============================= public ========================================

void frag_init() {
    memset(&frag_vars, 0, sizeof(frag_vars_t));

    // unspecified start value, wraps around at 65535     
    frag_vars.global_tag = openrandom_get16b() & 0x7FF;
}

owerror_t frag_fragment6LoPacket(OpenQueueEntry_t *msg) {
    OpenQueueEntry_t *lowpan_fragment;
    uint16_t remaining_bytes;
    uint8_t fragment_length; uint8_t fragment_offset;
    int8_t bpos;

    // check if fragmentation is necessary
    if (!msg->l3_isFragment && msg->length > (MAX_FRAGMENT_SIZE + FRAGN_HEADER_SIZE)) {

        openserial_printInfo(COMPONENT_FRAG, ERR_FRAG_FRAGMENTING,
                              (errorparameter_t) msg->length,
                              (errorparameter_t) (msg->length / MAX_FRAGMENT_SIZE) + 1);

        // update the global 6LoWPAN datagram tag
        frag_vars.global_tag++;
        remaining_bytes = msg->length;
        fragment_offset = 0;
        bpos = -1;

        // start fragmenting
        while (remaining_bytes > 0) {

            lowpan_fragment = openqueue_getFreePacketBuffer(COMPONENT_FRAG);
            if (lowpan_fragment == NULL) {
                // 6LoWPAN packet couldn't be entirely fragmented, thus clean up previously created fragments and exit.
                openserial_printError(COMPONENT_FRAG, ERR_NO_FREE_PACKET_BUFFER,
                        (errorparameter_t) 0,
                        (errorparameter_t) 0);
                cleanup_fragments(frag_vars.global_tag);
                return E_FAIL;
            }

            lowpan_fragment->l3_isFragment = TRUE;
            lowpan_fragment->owner = COMPONENT_FRAG;
            lowpan_fragment->creator = msg->creator;

            if (remaining_bytes > MAX_FRAGMENT_SIZE)
                fragment_length = MAX_FRAGMENT_SIZE;
            else
                fragment_length = remaining_bytes;

            // find a new spot in the fragmentation buffer  
            for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
                if (frag_vars.fragmentBuf[i].pFragment == NULL) {
                    bpos = i;
                    break;
                }
            }

            // if fragmentation buffer is full, delete previously created fragments and abandon here
            if (bpos == -1) {
                openqueue_freePacketBuffer(lowpan_fragment);

                cleanup_fragments(frag_vars.global_tag);

                openserial_printError(COMPONENT_FRAG, ERR_FRAG_BUFFER_OV, (errorparameter_t) 0, (errorparameter_t) 0);
                return E_FAIL;
            }

            // populate a fragment buffer 
            frag_vars.fragmentBuf[bpos].datagram_tag = frag_vars.global_tag;
            frag_vars.fragmentBuf[bpos].datagram_offset = fragment_offset;
            frag_vars.fragmentBuf[bpos].pFragment = lowpan_fragment;
            frag_vars.fragmentBuf[bpos].pOriginalMsg = msg;

            // copy 'fragment_length' bytes from the original packet to the fragment
            packetfunctions_reserveHeaderSize(lowpan_fragment, fragment_length);
            memcpy(lowpan_fragment->payload, msg->payload + (fragment_offset * OFFSET_MULTIPLE), fragment_length);

            // copy address information
            lowpan_fragment->l3_destinationAdd = msg->l3_destinationAdd;
            lowpan_fragment->l3_sourceAdd = msg->l3_sourceAdd;
            lowpan_fragment->l2_nextORpreviousHop = msg->l2_nextORpreviousHop;

            remaining_bytes -= fragment_length;

            if (fragment_offset == 0) {
                prepend_frag1_header(lowpan_fragment, msg->length, frag_vars.global_tag);
            } else {
                prepend_fragn_header(lowpan_fragment, msg->length, frag_vars.global_tag, fragment_offset);
            }

            // update the fragment offset
            fragment_offset += (fragment_length / OFFSET_MULTIPLE);
        }

        // send all the fragments with the current datagram tag
        for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
            if (frag_vars.fragmentBuf[i].datagram_tag == frag_vars.global_tag) {
                // try to send the fragment. If this fails, abort the transmission of the other fragments.
                if (sixtop_send(frag_vars.fragmentBuf[i].pFragment) == E_FAIL) {
                    openserial_printError(COMPONENT_FRAG, ERR_FRAG_TX_FAIL,
                                          (errorparameter_t) frag_vars.global_tag,
                                          (errorparameter_t) frag_vars.fragmentBuf[i].datagram_offset);
                    cleanup_fragments(frag_vars.global_tag);
                    return E_FAIL;
                } else {
                    // fragment succesfully scheduled, lock it
                    LOCK(frag_vars.fragmentBuf[i]);
                }
            }
        }

        // if we arrive here, all fragments were successfully created and passed to the MAC layer
        return E_SUCCESS;

    } else if (msg->l3_isFragment) {
        // this is a fragment (type frag1) that's being source routed
        // set nexthop in vrb and restore original frag1 header

        for (uint8_t i = 0; i < NUM_OF_VRBS; i++) {
            if (frag_vars.vrbs[i].frag1 == msg) {
                memcpy(&frag_vars.vrbs[i].nexthop, &msg->l2_nextORpreviousHop, sizeof(open_addr_t));
                prepend_frag1_header(msg, frag_vars.vrbs[i].size, frag_vars.vrbs[i].tag);
                fast_forward_frags(frag_vars.vrbs[i].tag, frag_vars.vrbs[i].size, i);
                break;
            }
        }

        // forward source routed fragment
        return sixtop_send(msg);
    } else {
        msg->l3_isFragment = FALSE;
        return sixtop_send(msg);
    }
}


void frag_sendDone(OpenQueueEntry_t *msg, owerror_t sendError) {
    bool frags_queued;
    bool upward_relay;
    uint16_t datagram_tag;
    OpenQueueEntry_t *original_msg;

    if (msg->l3_isFragment && !msg->l3_useSourceRouting) {

        frags_queued = FALSE;
        upward_relay = FALSE;

        // delete sent fragment from fragment buffer
        int i;
        for (i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
            if (frag_vars.fragmentBuf[i].pFragment == msg) {
                datagram_tag = frag_vars.fragmentBuf[i].datagram_tag;
                original_msg = frag_vars.fragmentBuf[i].pOriginalMsg;
                UNLOCK(frag_vars.fragmentBuf[i]);
                RESET_FRAG_BUFFER_ENTRY(i);
                break;
            }
        }

        if (i >= FRAGMENT_BUFFER_SIZE) {
            upward_relay = TRUE;
        }

        if (sendError == E_SUCCESS && upward_relay == FALSE) {
            // check if we have send all other fragments of the original packet
            for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
                if (frag_vars.fragmentBuf[i].datagram_tag == datagram_tag) {
                    frags_queued = TRUE;
                    break;
                }
            }

            if (frags_queued == FALSE) {
                iphc_sendDone(original_msg, sendError);
            }

        } else if (sendError == E_FAIL && upward_relay == FALSE) {
            // transmission failed, remove the other fragments that are not locked in for transmission
            cleanup_fragments(datagram_tag);
            iphc_sendDone(original_msg, sendError);
        } else {
            openqueue_freePacketBuffer(msg);
        }
    } else if (msg->l3_isFragment && msg->l3_useSourceRouting) {

        int k;
        for (k = 0; k < FRAGMENT_BUFFER_SIZE; k++) {
            if (frag_vars.fragmentBuf[k].pFragment == msg) {
                datagram_tag = frag_vars.fragmentBuf[k].datagram_tag;
                original_msg = frag_vars.fragmentBuf[k].pOriginalMsg;
                UNLOCK(frag_vars.fragmentBuf[k]);
                RESET_FRAG_BUFFER_ENTRY(k);
                break;
            }
        }

        if (k >= FRAGMENT_BUFFER_SIZE) {
            // fragment not found in fragment buffer (it was never stored locally, immediately fast-forwarded)
            openqueue_freePacketBuffer(msg);
        }

    } else {
        // if the sent msg is not a fragment, just pass along to higher layer 
        iphc_sendDone(msg, sendError);
    }
}


void frag_receive(OpenQueueEntry_t *msg) {
    uint8_t dispatch;
    uint8_t offset;
    uint8_t page_length;
    uint16_t size;
    uint16_t tag;
    ipv6_header_iht ipv6_outer_header;
    ipv6_header_iht ipv6_inner_header;

    memset(&ipv6_outer_header, 0, sizeof(ipv6_header_iht));
    memset(&ipv6_inner_header, 0, sizeof(ipv6_header_iht));

    msg->owner = COMPONENT_FRAG;
    dispatch = (uint8_t)(packetfunctions_ntohs(msg->payload) >> DISPATCH_SHIFT);

    if (dispatch == DISPATCH_FRAG_FIRST) {
        // first part of a fragment message
        size = (uint16_t)(packetfunctions_ntohs(msg->payload) & SIZE_MASK);
        tag = (uint16_t)(packetfunctions_ntohs(msg->payload + 2));
        offset = 0;

        msg->l3_isFragment = TRUE;

        // protection against oversized packets.
        CHECK_OVERSIZED(size);

        if (idmanager_getIsDAGroot() == TRUE) {
            openbridge_receive(msg);
            return;
        } else {
            // recover ip address from first fragment
            packetfunctions_tossHeader(msg, FRAG1_HEADER_SIZE);
            iphc_retrieveIPv6Header(msg, &ipv6_outer_header, &ipv6_inner_header, &page_length);

            if (idmanager_isMyAddress(&ipv6_inner_header.dest)) {
                // if LoWPAN packet is for me, store it for reassembly
                store_fragment(msg, size, tag, offset);
            } else {
                // fast forwarding / source routing
                msg->creator = COMPONENT_FRAG;
                allocate_vrb(msg, size, tag);
                return iphc_receive(msg);
            }
        }
    } else if (dispatch == DISPATCH_FRAG_SUBSEQ) {
        size = (uint16_t)(packetfunctions_ntohs(msg->payload) & SIZE_MASK);
        tag = (uint16_t)(packetfunctions_ntohs(msg->payload + 2));
        offset = (uint8_t)((((fragn_t *) msg->payload)->datagram_offset));

        msg->l3_isFragment = TRUE;

        // protection against oversized packets.
        CHECK_OVERSIZED(size);

        if (idmanager_getIsDAGroot() == TRUE) {
            openbridge_receive(msg);
        } else {
            packetfunctions_tossHeader(msg, FRAGN_HEADER_SIZE);


            uint8_t i;
            for (i = 0; i < NUM_OF_VRBS; i++) {
                if (frag_vars.vrbs[i].tag == tag &&
                    frag_vars.vrbs[i].size == size &&
                    frag_vars.vrbs[i].nexthop.type != ADDR_NONE)
                    break;
            }

            if (i < NUM_OF_VRBS) {
                // we have found a corresponding VRB for this subsequent fragment, update the fragment's next hop
                msg->l3_useSourceRouting = TRUE;
                msg->creator = COMPONENT_FRAG;

                memcpy(&msg->l2_nextORpreviousHop, &frag_vars.vrbs[i].nexthop, sizeof(open_addr_t));

                // update the VRB (how many bytes do we still need to forward)
                frag_vars.vrbs[i].left -= msg->length;

                if (frag_vars.vrbs[i].left == 0) {
                    // all bytes forwarded, remove VRB entry
                    openserial_printInfo(COMPONENT_FRAG, ERR_FRAG_FAST_FORWARD, (errorparameter_t) tag,
                                         (errorparameter_t) size);
                    opentimers_cancel(frag_vars.vrbs[i].forward_timer);
                    opentimers_destroy(frag_vars.vrbs[i].forward_timer);
                    if (frag_timerq_remove(frag_vars.vrbs[i].forward_timer) == E_FAIL) {
                        openserial_printCritical(COMPONENT_FRAG, ERR_EMPTY_QUEUE_OR_UNKNOWN_TIMER,
                                                 (errorparameter_t) 1,
                                                 (errorparameter_t) 0);
                    }
                    memset(&frag_vars.vrbs[i], 0, sizeof(vrb_t));

                }

                // restore fragn header
                prepend_fragn_header(msg, size, tag, offset);
                sixtop_send(msg);
            } else {
                /*
                 * If VRB buffer does not exist, there are two scenarios:
                 * (1) I am the destination, but I have not yet received the first fragment containing the address
                 *     information. Store the fragment and attempt reassembly later.
                 * (2) A subsequent fragment arrived out-of-order. Store temporarily and wait for first fragment to
                 *     create a VRB and fast-forward to the next hop.
                */
                store_fragment(msg, size, tag, offset);
            }
        }
    } else {
        // not a fragment
        iphc_receive(msg);
    }
}

//=========================== private =======================================


static void cleanup_fragments(uint16_t datagram_tag) {
    for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
        if (frag_vars.fragmentBuf[i].datagram_tag == datagram_tag)
            RESET_FRAG_BUFFER_ENTRY(i);
    }
}

static void store_fragment(OpenQueueEntry_t *msg, uint16_t size, uint16_t tag, uint8_t offset) {
    uint8_t dropped_srh_len;
    uint8_t count;
    uint16_t total_wanted_bytes;
    uint16_t received_bytes;

    bool do_reassemble = FALSE;
    bool has_timer = FALSE;

    // we detect a duplicate fragment (if datagram_tag and offset are the same)
    // check if we have running reassembly timer
    for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
        if (frag_vars.fragmentBuf[i].datagram_tag == tag && frag_vars.fragmentBuf[i].datagram_offset == offset) {
            openqueue_freePacketBuffer(msg);
            return;
        }

        if (frag_vars.fragmentBuf[i].datagram_tag == tag && frag_vars.fragmentBuf[i].reassembly_timer != 0) {
            has_timer = TRUE;
        }
    }

    // we find buffer space for a new fragment (new datagram_tag)
    int i;
    for (i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
        if (frag_vars.fragmentBuf[i].pFragment == NULL) {
            frag_vars.fragmentBuf[i].datagram_tag = tag;
            frag_vars.fragmentBuf[i].datagram_offset = offset;
            frag_vars.fragmentBuf[i].pFragment = msg;
            frag_vars.fragmentBuf[i].pOriginalMsg = NULL;

            if (!has_timer) {
                frag_vars.fragmentBuf[i].reassembly_timer = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_FRAG);

                // get a timer for the fragment reassembly and add it to the timer queue
                if ((frag_vars.fragmentBuf[i].reassembly_timer == ERROR_NO_AVAILABLE_ENTRIES) ||
                    (frag_timerq_enqueue(frag_vars.fragmentBuf[i].reassembly_timer) == E_FAIL)) {

                    openserial_printError(COMPONENT_FRAG, ERR_NO_FREE_TIMER_OR_QUEUE_ENTRY,
                                          (errorparameter_t) 0, (errorparameter_t) 0);
                    RESET_FRAG_BUFFER_ENTRY(i);
                    return;
                }

                opentimers_scheduleAbsolute(
                        frag_vars.fragmentBuf[i].reassembly_timer,
                        FRAG_REASSEMBLY_TIMEOUT,
                        opentimers_getValue(),
                        TIME_MS,
                        frag_timeout_cb
                );
            }
            break;
        }
    }

    // if we don't find any buffer space, delete all the related fragments
    if (i == FRAGMENT_BUFFER_SIZE) {
        openserial_printError(COMPONENT_FRAG, ERR_FRAG_BUFFER_OV, (errorparameter_t) 0, (errorparameter_t) 0);
        cleanup_fragments(tag);
        return;
    }

    // check if we have all the fragments
    total_wanted_bytes = size;
    received_bytes = dropped_srh_len = count = 0;

    for (int j = 0; j < FRAGMENT_BUFFER_SIZE; j++) {
        if (tag == frag_vars.fragmentBuf[j].datagram_tag) {
            if (frag_vars.fragmentBuf[j].datagram_offset == 0) {
                dropped_srh_len = MAX_FRAGMENT_SIZE - frag_vars.fragmentBuf[j].pFragment->length;
                received_bytes += (frag_vars.fragmentBuf[j].pFragment->length + dropped_srh_len);
            } else {
                received_bytes += (frag_vars.fragmentBuf[j].pFragment->length);
            }
        }
        if (frag_vars.fragmentBuf[j].pFragment != NULL) {
            count++;
        }
    }

    if (total_wanted_bytes == received_bytes) {
        do_reassemble = TRUE;
        openserial_printInfo(COMPONENT_FRAG, ERR_FRAG_STORED, (errorparameter_t) offset, (errorparameter_t) count);
    } else if (total_wanted_bytes < received_bytes) {
        board_reset();
    } else {
        do_reassemble = FALSE;
        openserial_printInfo(COMPONENT_FRAG, ERR_FRAG_STORED, (errorparameter_t) offset, (errorparameter_t) count);
    }

    if (do_reassemble) {
        OpenQueueEntry_t *reassembled_msg;
        reassembled_msg = openqueue_getFreeBigPacketBuffer(COMPONENT_FRAG);

        reassemble_fragments(tag, size - dropped_srh_len, reassembled_msg);

        if (reassembled_msg == NULL) {
            return;
        } else {
            iphc_receive(reassembled_msg);
        }
    }
}

static void reassemble_fragments(uint16_t tag, uint16_t size, OpenQueueEntry_t *reassembled_msg) {
    uint8_t *ptr;
    uint8_t offset = 0;

    if (reassembled_msg == NULL) {
        openserial_printError(COMPONENT_FRAG, ERR_NO_FREE_PACKET_BUFFER, (errorparameter_t) 1, (errorparameter_t) 0);
        cleanup_fragments(tag);
        return;
    }

    reassembled_msg->is_big_packet = TRUE;
    reassembled_msg->length = size;

    for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
        if (frag_vars.fragmentBuf[i].pFragment != NULL && frag_vars.fragmentBuf[i].datagram_tag == tag) {
            if (frag_vars.fragmentBuf[i].datagram_offset == 0 &&
                frag_vars.fragmentBuf[i].pFragment->length < MAX_FRAGMENT_SIZE) {
                offset = (MAX_FRAGMENT_SIZE - frag_vars.fragmentBuf[i].pFragment->length);
                ptr = reassembled_msg->packet + offset;
            } else {
                ptr = reassembled_msg->packet + (frag_vars.fragmentBuf[i].datagram_offset * OFFSET_MULTIPLE);
            }

            memcpy(ptr, frag_vars.fragmentBuf[i].pFragment->payload, frag_vars.fragmentBuf[i].pFragment->length);

            if (frag_vars.fragmentBuf[i].reassembly_timer != 0) {
                opentimers_cancel(frag_vars.fragmentBuf[i].reassembly_timer);
                opentimers_destroy(frag_vars.fragmentBuf[i].reassembly_timer);
                if (frag_timerq_remove(frag_vars.fragmentBuf[i].reassembly_timer) == E_FAIL) {
                    openserial_printCritical(COMPONENT_FRAG, ERR_EMPTY_QUEUE_OR_UNKNOWN_TIMER,
                                             (errorparameter_t) 2,
                                             (errorparameter_t) 0);
                }
            }

            RESET_FRAG_BUFFER_ENTRY(i);
        }
    }

    openserial_printInfo(COMPONENT_FRAG, ERR_FRAG_REASSEMBLED, (errorparameter_t) size, (errorparameter_t) tag);
    reassembled_msg->payload = reassembled_msg->packet + offset;
}

static owerror_t allocate_vrb(OpenQueueEntry_t *frag1, uint16_t size, uint16_t tag) {
    // find an a vrb spot
    uint8_t i;
    for (i = 0; i < NUM_OF_VRBS; i++) {
        if (frag_vars.vrbs[i].tag == 0 && frag_vars.vrbs[i].size == 0) {
            frag_vars.vrbs[i].tag = tag;
            frag_vars.vrbs[i].size = size;
            frag_vars.vrbs[i].left = (size - MAX_FRAGMENT_SIZE);
            frag_vars.vrbs[i].frag1 = frag1;

            frag_vars.vrbs[i].forward_timer = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_FRAG);

            // get a timer for the fragment forwarding and add it to the timer queue
            if ((frag_vars.vrbs[i].forward_timer == ERROR_NO_AVAILABLE_ENTRIES) ||
                (frag_timerq_enqueue(frag_vars.vrbs[i].forward_timer) == E_FAIL)) {

                openserial_printError(COMPONENT_FRAG, ERR_NO_FREE_TIMER_OR_QUEUE_ENTRY,
                                      (errorparameter_t) 0, (errorparameter_t) 0);
                memset(&frag_vars.vrbs[i], 0, sizeof(vrb_t));
                return E_FAIL;
            }

            opentimers_scheduleAbsolute(
                    frag_vars.vrbs[i].forward_timer,
                    FRAG_REASSEMBLY_TIMEOUT,
                    opentimers_getValue(),
                    TIME_MS,
                    frag_timeout_cb
            );

            break;
        }
    }

    if (i >= NUM_OF_VRBS)
        return E_FAIL;
    else
        return E_SUCCESS;
}

static void fast_forward_frags(uint16_t tag, uint16_t size, uint8_t vrb_pos) {
    for (int i = 0; i < FRAGMENT_BUFFER_SIZE; i++) {
        // check if we have subsequent fragments stored.
        if (frag_vars.fragmentBuf[i].pFragment != NULL &&
            frag_vars.fragmentBuf[i].datagram_tag == tag &&
            frag_vars.fragmentBuf[i].datagram_offset != 0) {

            frag_vars.fragmentBuf[i].pFragment->creator = COMPONENT_FRAG;
            frag_vars.fragmentBuf[i].pFragment->l3_useSourceRouting = TRUE;

            // provide the stored fragment with the right next hop address
            memcpy(&frag_vars.fragmentBuf[i].pFragment->l2_nextORpreviousHop,
                   &frag_vars.vrbs[vrb_pos].nexthop,
                   sizeof(open_addr_t)
            );

            // update the VRB
            frag_vars.vrbs[vrb_pos].left -= frag_vars.fragmentBuf[i].pFragment->length;

            if (frag_vars.vrbs[vrb_pos].left == 0) {
                // clear VRB entry if all data is forwarded
                openserial_printInfo(COMPONENT_FRAG, ERR_FRAG_FAST_FORWARD, (errorparameter_t) tag,
                                     (errorparameter_t) size);
                opentimers_cancel(frag_vars.vrbs[vrb_pos].forward_timer);
                opentimers_destroy(frag_vars.vrbs[vrb_pos].forward_timer);
                if (frag_timerq_remove(frag_vars.vrbs[vrb_pos].forward_timer) == E_FAIL) {
                    openserial_printCritical(COMPONENT_FRAG, ERR_EMPTY_QUEUE_OR_UNKNOWN_TIMER,
                                             (errorparameter_t) 3,
                                             (errorparameter_t) 0);
                }
                memset(&frag_vars.vrbs[vrb_pos], 0, sizeof(vrb_t));
            }

            if (frag_vars.fragmentBuf[i].reassembly_timer != 0) {
                opentimers_cancel(frag_vars.fragmentBuf[i].reassembly_timer);
                opentimers_destroy(frag_vars.fragmentBuf[i].reassembly_timer);
                if (frag_timerq_remove(frag_vars.fragmentBuf[i].reassembly_timer) == E_FAIL) {
                    openserial_printCritical(COMPONENT_FRAG, ERR_EMPTY_QUEUE_OR_UNKNOWN_TIMER,
                                             (errorparameter_t) 4,
                                             (errorparameter_t) 0);
                }
            }

            prepend_fragn_header(
                    frag_vars.fragmentBuf[i].pFragment,
                    size,
                    frag_vars.fragmentBuf[i].datagram_tag,
                    frag_vars.fragmentBuf[i].datagram_offset);

            LOCK(frag_vars.fragmentBuf[i]);
            if (sixtop_send(frag_vars.fragmentBuf[i].pFragment) == E_FAIL) {
                openserial_printError(COMPONENT_FRAG, ERR_FRAG_TX_FAIL,
                                      (errorparameter_t) frag_vars.fragmentBuf[i].datagram_tag,
                                      (errorparameter_t) frag_vars.fragmentBuf[i].datagram_offset);
            };
        }
    }
}

static void prepend_frag1_header(OpenQueueEntry_t *frag1, uint16_t size, uint16_t tag) {
    uint16_t ds_field; // temporary dispatch | size field for fragmentation header
    packetfunctions_reserveHeaderSize(frag1, FRAG1_HEADER_SIZE);
    ds_field = ((DISPATCH_FRAG_FIRST & DISPATCH_MASK) << DISPATCH_SHIFT);
    ds_field |= (size & SIZE_MASK);
    packetfunctions_htons(ds_field, (uint8_t * ) & (((frag1_t *) frag1->payload)->dispatch_size_field));
    packetfunctions_htons(tag, (uint8_t * ) & (((frag1_t *) frag1->payload)->datagram_tag));
}

static void prepend_fragn_header(OpenQueueEntry_t *fragn, uint16_t size, uint16_t tag, uint8_t offset) {
    uint16_t ds_field; // temporary dispatch | size field for fragmentation header
    packetfunctions_reserveHeaderSize(fragn, FRAGN_HEADER_SIZE);
    ds_field = ((DISPATCH_FRAG_SUBSEQ & DISPATCH_MASK) << DISPATCH_SHIFT);
    ds_field |= (size & SIZE_MASK);
    packetfunctions_htons(ds_field, (uint8_t * ) & (((fragn_t *) fragn->payload)->dispatch_size_field));
    packetfunctions_htons(tag, (uint8_t * ) & (((fragn_t *) fragn->payload)->datagram_tag));
    ((fragn_t *) fragn->payload)->datagram_offset = offset;
}

owerror_t frag_timerq_enqueue(opentimers_id_t id) {
    uint8_t i;
    for (i = 0; i < NUM_OF_CONCURRENT_TIMERS; i++) {
        if (frag_vars.frag_timerq[i] == 0) {
            frag_vars.frag_timerq[i] = id;
            return E_SUCCESS;
        }
    }

    return E_FAIL;
}

opentimers_id_t frag_timerq_dequeue(void) {
    opentimers_id_t expired;
    expired = frag_vars.frag_timerq[0];

    memcpy((uint8_t *) frag_vars.frag_timerq, (uint8_t * ) & (frag_vars.frag_timerq[1]), NUM_OF_CONCURRENT_TIMERS - 1);
    frag_vars.frag_timerq[NUM_OF_CONCURRENT_TIMERS - 1] = 0;

    return expired;
}

owerror_t frag_timerq_remove(opentimers_id_t id) {
    for (int i = 0; i < NUM_OF_CONCURRENT_TIMERS - 1; i++) {
        if (frag_vars.frag_timerq[i] == id) {
            memcpy(&frag_vars.frag_timerq[i], &frag_vars.frag_timerq[i + 1], NUM_OF_CONCURRENT_TIMERS - 1 - i);
            frag_vars.frag_timerq[NUM_OF_CONCURRENT_TIMERS - 1] = 0;
            return E_SUCCESS;
        }
    }

    if (frag_vars.frag_timerq[NUM_OF_CONCURRENT_TIMERS - 1] == id) {
        frag_vars.frag_timerq[NUM_OF_CONCURRENT_TIMERS - 1] = 0;
        return E_SUCCESS;
    }

    return E_FAIL;
}

void frag_timeout_cb(opentimers_id_t id) {
    opentimers_id_t expired_timer;

    if ((expired_timer = frag_timerq_dequeue()) == 0) {
        // timer id can never be 0, if we get zero we have "dequeued" and empty queue!
        openserial_printCritical(COMPONENT_FRAG, ERR_EMPTY_QUEUE_OR_UNKNOWN_TIMER,
                                 (errorparameter_t) 0,
                                 (errorparameter_t) 0);
    }

    // find the tag of the expired fragments
    for (int j = 0; j < FRAGMENT_BUFFER_SIZE; j++) {
        if (frag_vars.fragmentBuf[j].pFragment != NULL && frag_vars.fragmentBuf[j].reassembly_timer == expired_timer) {
            openserial_printError(COMPONENT_FRAG, ERR_FRAG_REASSEMBLY_OR_VRB_TIMEOUT,
                                  (errorparameter_t) frag_vars.fragmentBuf[j].datagram_tag,
                                  (errorparameter_t) 0);
            opentimers_destroy(frag_vars.fragmentBuf[j].reassembly_timer);
            cleanup_fragments(frag_vars.fragmentBuf[j].datagram_tag);
            break;
        }
    }

    for (uint8_t j = 0; j < NUM_OF_VRBS; j++) {
        if (frag_vars.vrbs[j].tag != 0 && frag_vars.vrbs[j].forward_timer == expired_timer) {
            openserial_printError(COMPONENT_FRAG, ERR_FRAG_REASSEMBLY_OR_VRB_TIMEOUT,
                                  (errorparameter_t) frag_vars.vrbs[j].tag,
                                  (errorparameter_t) 0);
            opentimers_destroy(frag_vars.vrbs[j].forward_timer);
            memset(&frag_vars.vrbs[j], 0, sizeof(vrb_t));
        }
    }
}
