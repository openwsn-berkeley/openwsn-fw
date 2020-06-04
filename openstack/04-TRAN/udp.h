#ifndef OPENWSN_UDP_H
#define OPENWSN_UDP_H

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef struct udp_header {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} udp_ht;

//=========================== prototypes ======================================

void udp_sendDone(OpenQueueEntry_t *msg, owerror_t);

void udp_receive(OpenQueueEntry_t *msg);

void udp_transmit(OpenQueueEntry_t *msg);

#endif /* OPENWSN_UDP_H */
