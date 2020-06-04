#include "config.h"

#if OPENWSN_UDP_C

#include "sock_internal.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "udp.h"
#include "openqueue.h"
#include "forwarding.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udp_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
    msg->owner = COMPONENT_UDP;
    sock_senddone_internal(msg, error);
    openqueue_freePacketBuffer(msg);
}

void udp_receive(OpenQueueEntry_t *msg) {

    msg->owner = COMPONENT_UDP_TO_SOCK;
    msg->l4_sourcePortORicmpv6Type = packetfunctions_ntohs((uint8_t * ) & (((udp_ht *) (msg->payload))->source_port));
    msg->l4_destination_port = packetfunctions_ntohs((uint8_t * ) & (((udp_ht *) (msg->payload))->dest_port));

    // verify checksum

    scheduler_push_task(sock_receive_internal, TASKPRIO_UDP);

    packetfunctions_tossHeader(&msg, sizeof(udp_ht));
    msg->l4_length = msg->length;
    msg->l4_payload = msg->payload;
}

void udp_transmit(OpenQueueEntry_t *msg) {
    msg->l4_protocol_compressed = FALSE;
    msg->l4_protocol = IANA_UDP;

    packetfunctions_reserveHeader(&msg, sizeof(udp_ht));
    packetfunctions_htons(msg->l4_sourcePortORicmpv6Type, &(msg->payload[0]));
    packetfunctions_htons(msg->l4_destination_port, &(msg->payload[2]));
    packetfunctions_htons(msg->length, &(msg->payload[4]));
    packetfunctions_calculateChecksum(msg, (uint8_t * ) & (((udp_ht *) msg->payload)->checksum));

    if (forwarding_send(msg) == E_FAIL) {
        openqueue_freePacketBuffer(msg);
    }
}

#endif /* OPENWSN_UDP_C */
