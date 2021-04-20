#include "config.h"
#include "packetfunctions.h"
#include "IEEE802154_security.h"
#include "openserial.h"
#include "idmanager.h"
#include "radio.h"

#if OPENWSN_6LO_FRAGMENTATION_C
#include "openqueue.h"
#endif

//=========================== variables =======================================

//=========================== prototypes ======================================

void onesComplementSum(uint8_t *global_sum, const uint8_t *ptr, int length);

//=========================== public ==========================================

//======= address translation

//assuming an ip128b is a concatenation of prefix64b followed by a mac64b
void packetfunctions_ip128bToMac64b(const open_addr_t *ip128b, open_addr_t *prefix64btoWrite, open_addr_t *mac64btoWrite) {
    if (ip128b->type != ADDR_128B) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                     (errorparameter_t) ip128b->type,
                     (errorparameter_t) 0);
        mac64btoWrite->type = ADDR_NONE;
        return;
    }

    prefix64btoWrite->type = ADDR_PREFIX;
    memcpy(prefix64btoWrite->addr_type.prefix, &(ip128b->addr_type.addr_128b[0]), 8);
    mac64btoWrite->type = ADDR_64B;
    memcpy(mac64btoWrite->addr_type.addr_64b, &(ip128b->addr_type.addr_128b[8]), 8);
}

void packetfunctions_mac64bToIp128b(const open_addr_t *prefix64b, const open_addr_t *mac64b, open_addr_t *ip128bToWrite) {
    if (prefix64b->type != ADDR_PREFIX || mac64b->type != ADDR_64B) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                     (errorparameter_t) prefix64b->type,
                     (errorparameter_t) 1);
        ip128bToWrite->type = ADDR_NONE;
        return;
    }

    ip128bToWrite->type = ADDR_128B;
    memcpy(&(ip128bToWrite->addr_type.addr_128b[0]), &(prefix64b->addr_type.prefix[0]), 8);
    memcpy(&(ip128bToWrite->addr_type.addr_128b[8]), &(mac64b->addr_type.addr_64b[0]), 8);
}

//assuming an mac16b is lower 2B of mac64b
void packetfunctions_mac64bToMac16b(const open_addr_t *mac64b, open_addr_t *mac16btoWrite) {
    if (mac64b->type != ADDR_64B) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                     (errorparameter_t) mac64b->type,
                     (errorparameter_t) 2);
        mac16btoWrite->type = ADDR_NONE;
        return;
    }

    mac16btoWrite->type = ADDR_16B;
    mac16btoWrite->addr_type.addr_16b[0] = mac64b->addr_type.addr_64b[6];
    mac16btoWrite->addr_type.addr_16b[1] = mac64b->addr_type.addr_64b[7];
}

void packetfunctions_mac16bToMac64b(const open_addr_t *mac16b, open_addr_t *mac64btoWrite) {
    if (mac16b->type != ADDR_16B) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                     (errorparameter_t) mac16b->type,
                     (errorparameter_t) 3);
        mac64btoWrite->type = ADDR_NONE;
        return;
    }
    mac64btoWrite->type = ADDR_64B;
    mac64btoWrite->addr_type.addr_64b[0] = 0;
    mac64btoWrite->addr_type.addr_64b[1] = 0;
    mac64btoWrite->addr_type.addr_64b[2] = 0;
    mac64btoWrite->addr_type.addr_64b[3] = 0;
    mac64btoWrite->addr_type.addr_64b[4] = 0;
    mac64btoWrite->addr_type.addr_64b[5] = 0;
    mac64btoWrite->addr_type.addr_64b[6] = mac16b->addr_type.addr_16b[0];
    mac64btoWrite->addr_type.addr_64b[7] = mac16b->addr_type.addr_16b[1];
}

//======= address recognition

bool packetfunctions_isBroadcastMulticast(const open_addr_t *address) {
    uint8_t i;
    uint8_t address_length;

    // anycast type
    if (address->type == ADDR_ANYCAST) {
        return TRUE;
    }

    //IPv6 multicast
    if (address->type == ADDR_128B) {
        if (address->addr_type.addr_128b[0] == 0xff) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    //15.4 broadcast
    switch (address->type) {
        case ADDR_16B:
            address_length = 2;
            break;
        case ADDR_64B:
            address_length = 8;
            break;
        default:
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                         (errorparameter_t) address->type,
                         (errorparameter_t) 4);
            return FALSE;
    }

    for (i = 0; i < address_length; i++) {
        if (address->addr_type.addr_128b[i] != 0xFF) {
            return FALSE;
        }
    }
    return TRUE;
}

bool packetfunctions_isAllRoutersMulticast(const open_addr_t *address) {
    if (
            address->type == ADDR_128B &&
            address->addr_type.addr_128b[0] == 0xff &&
            address->addr_type.addr_128b[1] == 0x02 &&
            address->addr_type.addr_128b[2] == 0x00 &&
            address->addr_type.addr_128b[3] == 0x00 &&
            address->addr_type.addr_128b[4] == 0x00 &&
            address->addr_type.addr_128b[5] == 0x00 &&
            address->addr_type.addr_128b[6] == 0x00 &&
            address->addr_type.addr_128b[7] == 0x00 &&
            address->addr_type.addr_128b[8] == 0x00 &&
            address->addr_type.addr_128b[9] == 0x00 &&
            address->addr_type.addr_128b[10] == 0x00 &&
            address->addr_type.addr_128b[11] == 0x00 &&
            address->addr_type.addr_128b[12] == 0x00 &&
            address->addr_type.addr_128b[13] == 0x00 &&
            address->addr_type.addr_128b[14] == 0x00 &&
            address->addr_type.addr_128b[15] == 0x1a
            ) {
        return TRUE;
    }
    return FALSE;
}

bool packetfunctions_isAllHostsMulticast(const open_addr_t *address) {
    if (
            address->type == ADDR_128B &&
            address->addr_type.addr_128b[0] == 0xff &&
            address->addr_type.addr_128b[1] == 0x02 &&
            address->addr_type.addr_128b[2] == 0x00 &&
            address->addr_type.addr_128b[3] == 0x00 &&
            address->addr_type.addr_128b[4] == 0x00 &&
            address->addr_type.addr_128b[5] == 0x00 &&
            address->addr_type.addr_128b[6] == 0x00 &&
            address->addr_type.addr_128b[7] == 0x00 &&
            address->addr_type.addr_128b[8] == 0x00 &&
            address->addr_type.addr_128b[9] == 0x00 &&
            address->addr_type.addr_128b[10] == 0x00 &&
            address->addr_type.addr_128b[11] == 0x00 &&
            address->addr_type.addr_128b[12] == 0x00 &&
            address->addr_type.addr_128b[13] == 0x00 &&
            address->addr_type.addr_128b[14] == 0x00 &&
            address->addr_type.addr_128b[15] == 0x01
            ) {
        return TRUE;
    }
    return FALSE;
}

bool packetfunctions_isLinkLocal(const open_addr_t *address) {
    if (
            address->type == ADDR_128B &&
            address->addr_type.addr_128b[0] == 0xfe &&
            address->addr_type.addr_128b[1] == 0x80 &&
            address->addr_type.addr_128b[2] == 0x00 &&
            address->addr_type.addr_128b[3] == 0x00 &&
            address->addr_type.addr_128b[4] == 0x00 &&
            address->addr_type.addr_128b[5] == 0x00 &&
            address->addr_type.addr_128b[6] == 0x00 &&
            address->addr_type.addr_128b[7] == 0x00
            ) {
        return TRUE;
    }
    return FALSE;
}

bool packetfunctions_sameAddress(const open_addr_t *address_1, const open_addr_t *address_2) {

    uint8_t address_length;

    if (address_1->type != address_2->type) {
        return FALSE;
    }

    switch (address_1->type) {
        case ADDR_16B:
        case ADDR_PANID:
            address_length = 2;
            break;
        case ADDR_64B:
        case ADDR_PREFIX:
            address_length = 8;
            break;
        case ADDR_128B:
        case ADDR_ANYCAST:
            address_length = 16;
            break;
        default:
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                         (errorparameter_t) address_1->type,
                         (errorparameter_t) 5);
            return FALSE;
    }

    if (memcmp(address_1->addr_type.addr_128b, address_2->addr_type.addr_128b, address_length) == 0) {
        return TRUE;
    }
    return FALSE;
}

//======= address read/write

void packetfunctions_readAddress(const uint8_t *payload, uint8_t type, open_addr_t *writeToAddress, bool littleEndian) {
    uint8_t i;
    uint8_t address_length;

    writeToAddress->type = type;
    switch (type) {
        case ADDR_16B:
        case ADDR_PANID:
            address_length = 2;
            break;
        case ADDR_64B:
        case ADDR_PREFIX:
            address_length = 8;
            break;
        case ADDR_128B:
            address_length = 16;
            break;
        default:
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                         (errorparameter_t) type,
                         (errorparameter_t) 6);
            return;
    }

    for (i = 0; i < address_length; i++) {
        if (littleEndian) {
            writeToAddress->addr_type.addr_128b[address_length - 1 - i] = *(payload + i);
        } else {
            writeToAddress->addr_type.addr_128b[i] = *(payload + i);
        }
    }
}

owerror_t packetfunctions_writeAddress(OpenQueueEntry_t **msg, const open_addr_t *address, bool littleEndian) {
    uint8_t i;
    uint8_t address_length;

    switch (address->type) {
        case ADDR_16B:
        case ADDR_PANID:
            address_length = 2;
            break;
        case ADDR_64B:
        case ADDR_PREFIX:
            address_length = 8;
            break;
        case ADDR_128B:
            address_length = 16;
            break;
        default:
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_WRONG_ADDR_TYPE,
                         (errorparameter_t) address->type,
                         (errorparameter_t) 7);
            return E_FAIL;
    }

    for (i = 0; i < address_length; i++) {
        if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL){
            return E_FAIL;
        }
        if (littleEndian) {
            *((uint8_t * )((*msg)->payload)) = address->addr_type.addr_128b[i];
        } else {
            *((uint8_t * )((*msg)->payload)) = address->addr_type.addr_128b[address_length - 1 - i];
        }
    }

    return E_SUCCESS;
}

//======= reserving/tossing headers

owerror_t packetfunctions_reserveHeader(OpenQueueEntry_t **pkt, uint16_t header_length) {
    int16_t available_bytes;
    available_bytes = IEEE802154_FRAME_SIZE - LENGTH_CRC - IEEE802154_SECURITY_TAG_LEN;

#if OPENWSN_6LO_FRAGMENTATION_C
    if ((*pkt)->is_big_packet == FALSE && ((*pkt)->length + header_length) > available_bytes) {
        // CASE 1: small packet exceeding it boundaries, try copy to big packet.

        // below the fragmentation layer, we cannot use big packets
        if ((*pkt)->owner < COMPONENT_FRAG) {
            return E_FAIL;
        }

        OpenQueueEntry_t *bpkt;
        if ((bpkt = openqueue_getFreeBigPacketBuffer((*pkt)->creator)) == NULL) {
            return E_FAIL;
        }

        memcpy(bpkt, (*pkt), sizeof(OpenQueueEntry_t));

        // reset some packet metadata
        bpkt->length = 0;
        bpkt->is_big_packet = TRUE;
        bpkt->payload = &(((OpenQueueBigEntry_t *) bpkt)->packet_remainder[IPV6_PACKET_SIZE - IEEE802154_FRAME_SIZE]);

        // copy contents from small packet to new packet
        if (packetfunctions_reserveHeader(&bpkt, (*pkt)->length) == E_FAIL) {
            openqueue_freePacketBuffer(bpkt);
            return E_FAIL;
        }
        memcpy(bpkt->payload, (*pkt)->payload, (*pkt)->length);
        bpkt->l4_payload = bpkt->payload + ((*pkt)->payload - (*pkt)->l4_payload);

        // now reserve the original requested allocation
        if (packetfunctions_reserveHeader(&bpkt, header_length) == E_FAIL) {
            openqueue_freePacketBuffer(bpkt);
            return E_FAIL;
        }

        LOG_VERBOSE(COMPONENT_PACKETFUNCTIONS, ERR_COPY_TO_BPKT, (*pkt)->length + header_length, available_bytes);

        // release normal OpenQueueEntry
        openqueue_freePacketBuffer((*pkt));

        // set pointer
        (*pkt) = bpkt;
    } else if ((*pkt)->is_big_packet == FALSE && (*pkt)->length + header_length <= available_bytes){
        // CASE 2: within boundaries small packet, do normal allocation
        (*pkt)->payload -= header_length;
        (*pkt)->length += header_length;

        // check for buffer overflow on the left and on the right
        if ((uint8_t * )((*pkt)->payload) < (uint8_t * )((*pkt)->packet) ||
            (*pkt)->payload + (*pkt)->length > &(*pkt)->packet[IEEE802154_FRAME_SIZE]) {
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_LONG,
                         (errorparameter_t) (*pkt)->length,
                         (errorparameter_t) header_length);
            return E_FAIL;
        }

    } else if ((*pkt)->is_big_packet == TRUE && (*pkt)->length + header_length > IPV6_PACKET_SIZE) {
        // CASE 3: is big packet already and exceeding, must fail
        LOG_ERROR(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_LONG,
                (errorparameter_t) (*pkt)->length,
                (errorparameter_t) header_length);
        return E_FAIL;
    } else if ((*pkt)->is_big_packet == TRUE && (*pkt)->length + header_length <= IPV6_PACKET_SIZE) {
        // CASE 4: is big packet and allocating normally
        (*pkt)->payload -= header_length;
        (*pkt)->length += header_length;

        // check for buffer overflow on the left and on the right
        if ((uint8_t * )((*pkt)->payload) < (uint8_t * )((*pkt)->packet) ||
            (*pkt)->payload + (*pkt)->length >
            &(((OpenQueueBigEntry_t *) (*pkt))->packet_remainder[IPV6_PACKET_SIZE - IEEE802154_FRAME_SIZE])) {
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_LONG,
                         (errorparameter_t) (*pkt)->length,
                         (errorparameter_t) header_length);
            return E_FAIL;
        }
    } else {
        return E_FAIL;
    }

    return E_SUCCESS;
#else
    // CRC is allocated with the reserveFooter call, so here the packet should never be greater than 125 bytes
    if ((*pkt)->length + header_length > available_bytes) {
        LOG_ERROR(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_LONG,
                  (errorparameter_t) (*pkt)->length,
                  (errorparameter_t) header_length);
        return E_FAIL;
    }

    (*pkt)->payload -= header_length;
    (*pkt)->length += header_length;

    // check for buffer overflow on the left and on the right
    if ((uint8_t * )((*pkt)->payload) < (uint8_t * )((*pkt)->packet) ||
        (*pkt)->payload + (*pkt)->length > &(*pkt)->packet[IEEE802154_FRAME_SIZE]) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_LONG,
                     (errorparameter_t) (*pkt)->length,
                     (errorparameter_t) header_length);
        return E_FAIL;
    }

    return E_SUCCESS;
#endif
}

void packetfunctions_tossHeader(OpenQueueEntry_t **pkt, uint16_t header_length) {
#if OPENWSN_6LO_FRAGMENTATION_C
    int16_t available_bytes;
    available_bytes = IEEE802154_FRAME_SIZE - LENGTH_CRC - IEEE802154_SECURITY_TAG_LEN;

    if ((*pkt)->is_big_packet == FALSE) {
        // CASE 1: is a small packet, just toss bytes
        if ((uint8_t * )((*pkt)->payload + header_length) > &((*pkt)->packet[IEEE802154_FRAME_SIZE]) ||
            (*pkt)->length - header_length < 0) {
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_SHORT,
                         (errorparameter_t) (*pkt)->length,
                         (errorparameter_t) header_length);
            return;
        }

        (*pkt)->payload += header_length;
        (*pkt)->length -= header_length;
    } else {
        // CASE 2: is a big packet
        if ((uint8_t * )((*pkt)->payload + header_length) >
            &(((OpenQueueBigEntry_t *) (*pkt))->packet_remainder[IPV6_PACKET_SIZE - IEEE802154_FRAME_SIZE]) ||
            (*pkt)->length - header_length < 0) {
            LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_SHORT,
                         (errorparameter_t) (*pkt)->length,
                         (errorparameter_t) header_length);
            return;

        }
        if ((*pkt)->length - header_length <= available_bytes){
            // update length and payload pointer
            (*pkt)->payload += header_length;
            (*pkt)->length -= header_length;

            // try moving to a smaller packet
            OpenQueueEntry_t *spkt;
            if ((spkt = openqueue_getFreePacketBuffer((*pkt)->creator)) == NULL) {
                return;
            }

            memcpy(spkt, (*pkt), sizeof(OpenQueueEntry_t));

            spkt->length = 0;
            spkt->is_big_packet = FALSE;
            spkt->payload = &spkt->packet[available_bytes];

            if (packetfunctions_reserveHeader(&spkt, (*pkt)->length) == E_FAIL) {
                openqueue_freePacketBuffer(spkt);
                return;
            }

            memcpy(spkt->payload, (*pkt)->payload, (*pkt)->length);
            spkt->l4_payload = spkt->payload + ((*pkt)->payload - (*pkt)->l4_payload);

            LOG_VERBOSE(COMPONENT_PACKETFUNCTIONS, ERR_COPY_TO_SPKT, (*pkt)->length, available_bytes);

            // release OpenQueueBigEntry
            openqueue_freePacketBuffer((*pkt));

            // set pointer
            (*pkt) = spkt;
        } else {
            (*pkt)->payload += header_length;
            (*pkt)->length -= header_length;
        }
    }
#else
    if ((uint8_t * )((*pkt)->payload + header_length) > &(*pkt)->packet[IEEE802154_FRAME_SIZE] ||
        (*pkt)->length - header_length < 0) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_SHORT,
                     (errorparameter_t) (*pkt)->length,
                     (errorparameter_t) header_length);
    }

    (*pkt)->payload += header_length;
    (*pkt)->length -= header_length;
#endif
}

owerror_t packetfunctions_reserveFooter(OpenQueueEntry_t **pkt, uint16_t footer_length) {
    (*pkt)->length += footer_length;

    // function is only called from the MAC layer, there the packets should never be bigger than IEEE802154_FRAME_SIZE
    if ((*pkt)->length > IEEE802154_FRAME_SIZE) {
        LOG_ERROR(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_LONG,
                  (errorparameter_t) (*pkt)->length,
                  (errorparameter_t) footer_length);
        return E_FAIL;
    }

    return E_SUCCESS;
}

void packetfunctions_tossFooter(OpenQueueEntry_t **pkt, uint16_t footer_length) {
    (*pkt)->length -= footer_length;

    // function is only called from the MAC layer
    if ((*pkt)->length < 0) {
        LOG_CRITICAL(COMPONENT_PACKETFUNCTIONS, ERR_PACKET_TOO_SHORT,
                     (errorparameter_t) (*pkt)->length,
                     (errorparameter_t) footer_length);
    }
}


//======= packet duplication
// function duplicates a frame from one OpenQueueEntry structure to the other,
// updating pointers to the new memory location. Used to make a local copy of
// the frame before transmission (where it can possibly be encrypted). 
void packetfunctions_duplicatePacket(OpenQueueEntry_t *dst, const OpenQueueEntry_t *src) {
    // make a copy of the frame

    memcpy(dst, src, sizeof(OpenQueueEntry_t));

    // Calculate where payload starts in the buffer
    dst->payload = &dst->packet[src->payload - src->packet]; // update pointers

    // update l2_FrameCounter pointer
    dst->l2_FrameCounter = dst->payload + (src->l2_FrameCounter - src->payload);

    // update l2_ASNpayload pointer
    dst->l2_ASNpayload = dst->payload + (src->l2_ASNpayload - src->payload);

    // update l2_payload pointer
    dst->l2_payload = dst->payload + (src->l2_payload - src->payload);

    // update l4_payload pointer
    dst->l4_payload = dst->payload + (src->l4_payload - src->payload);
}

//======= CRC calculation

void packetfunctions_calculateCRC(OpenQueueEntry_t *msg) {
    uint16_t crc;
    uint8_t i;
    uint16_t count;
    crc = 0;
    for (count = 1; count < msg->length - 2; count++) {
        crc = crc ^ (uint8_t) * (msg->payload + count);
        //crc = crc ^ (uint16_t)*ptr++ << 8;
        for (i = 0; i < 8; i++) {
            if (crc & 0x1) {
                crc = crc >> 1 ^ 0x8408;
            } else {
                crc = crc >> 1;
            }
        }
    }
    *(msg->payload + (msg->length - 2)) = crc % 256;
    *(msg->payload + (msg->length - 1)) = crc / 256;
}

bool packetfunctions_checkCRC(const OpenQueueEntry_t *msg) {
    uint16_t crc;
    uint8_t i;
    uint16_t count;
    crc = 0;
    for (count = 0; count < msg->length - 2; count++) {
        crc = crc ^ (uint8_t) * (msg->payload + count);
        //crc = crc ^ (uint16_t)*ptr++ << 8;
        for (i = 0; i < 8; i++) {
            if (crc & 0x1) {
                crc = crc >> 1 ^ 0x8408;
            } else {
                crc = crc >> 1;
            }
        }
    }
    if (*(msg->payload + (msg->length - 2)) == crc % 256 &&
        *(msg->payload + (msg->length - 1)) == crc / 256) {
        return TRUE;
    } else {
        return FALSE;
    }
}

//======= checksum calculation

//see http://www-net.cs.umass.edu/kurose/transport/UDP.html, or http://tools.ietf.org/html/rfc1071
//see http://en.wikipedia.org/wiki/User_Datagram_Protocol#IPv6_PSEUDO-HEADER
void packetfunctions_calculateChecksum(OpenQueueEntry_t *msg, uint8_t *checksum_ptr) {
    uint8_t temp_checksum[2];
    uint8_t little_helper[2];
    open_addr_t localscopeAddress;

    // initialize running checksum
    temp_checksum[0] = 0;
    temp_checksum[1] = 0;

    //===== IPv6 pseudo header

    // determine the source and destination address format
    if (packetfunctions_isBroadcastMulticast(&msg->l3_destinationAdd) == TRUE) {
        // use link local address for source address (prefix and EUI64)

        // source address
        onesComplementSum(temp_checksum, (uint8_t *) linklocalprefix, 8);
        memcpy(&localscopeAddress, idmanager_getMyID(ADDR_64B), sizeof(open_addr_t));
        // invert 'u' bit (section 2.5.1 at https://www.ietf.org/rfc/rfc2373.txt)
        localscopeAddress.addr_type.addr_64b[0] ^= 0x02;
        onesComplementSum(temp_checksum, localscopeAddress.addr_type.addr_64b, 8);

        // boardcast destination address
        onesComplementSum(temp_checksum, msg->l3_destinationAdd.addr_type.addr_128b, 16);
    } else {
        // use 128-bit ipv6 address for source address and destination address

        // source address
        onesComplementSum(temp_checksum, (idmanager_getMyID(ADDR_PREFIX))->addr_type.prefix, 8);
        onesComplementSum(temp_checksum, (idmanager_getMyID(ADDR_64B))->addr_type.addr_64b, 8);
        // destination address
        onesComplementSum(temp_checksum, msg->l3_destinationAdd.addr_type.addr_128b, 16);
    }

    // length
    little_helper[0] = (msg->length & 0xFF00) >> 8;
    little_helper[1] = (msg->length & 0x00FF);
    onesComplementSum(temp_checksum, little_helper, 2);

    // next header
    little_helper[0] = 0;
    little_helper[1] = msg->l4_protocol;
    onesComplementSum(temp_checksum, little_helper, 2);

    //===== payload

    // reset the checksum currently in the payload
    *checksum_ptr = 0;
    *(checksum_ptr + 1) = 0;

    onesComplementSum(temp_checksum, msg->payload, msg->length);
    temp_checksum[0] ^= 0xFF;
    temp_checksum[1] ^= 0xFF;

    //write in packet
    *checksum_ptr = temp_checksum[0];
    *(checksum_ptr + 1) = temp_checksum[1];
}


void onesComplementSum(uint8_t *global_sum, const uint8_t *ptr, int length) {
    uint32_t sum = 0xFFFF & (global_sum[0] << 8 | global_sum[1]);
    while (length > 1) {
        sum += 0xFFFF & (*ptr << 8 | *(ptr + 1));
        ptr += 2;
        length -= 2;
    }
    if (length) {
        sum += (0xFF & *ptr) << 8;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    global_sum[0] = (sum >> 8) & 0xFF;
    global_sum[1] = sum & 0xFF;
}

//======= endianness

void packetfunctions_htons(uint16_t val, uint8_t *dest) {
    dest[0] = (val & 0xff00) >> 8;
    dest[1] = (val & 0x00ff);
}

uint16_t packetfunctions_ntohs(uint8_t *src) {
    return (((uint16_t) src[0]) << 8) |
           (((uint16_t) src[1])
           );
}

void packetfunctions_htonl(uint32_t val, uint8_t *dest) {
    dest[0] = (val & 0xff000000) >> 24;
    dest[1] = (val & 0x00ff0000) >> 16;
    dest[2] = (val & 0x0000ff00) >> 8;
    dest[3] = (val & 0x000000ff);
}

uint32_t packetfunctions_ntohl(uint8_t *src) {
    return (((uint32_t) src[0]) << 24) |
           (((uint32_t) src[1]) << 16) |
           (((uint32_t) src[2]) << 8) |
           (((uint32_t) src[3])
           );
}

// reverse byte order in the array
void packetfunctions_reverseArrayByteOrder(uint8_t *start, uint8_t len) {
    uint8_t *lo = start;
    uint8_t *hi = start + len - 1;
    uint8_t swap;
    while (lo < hi) {
        swap = *lo;
        *lo++ = *hi;
        *hi-- = swap;
    }
}

//=========================== private =========================================
