#ifndef OPENWSN_PACKETFUNCTIONS_H
#define OPENWSN_PACKETFUNCTIONS_H

/**
\addtogroup cross-layers
\{
\addtogroup PacketFunctions
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

// address translation
void packetfunctions_ip128bToMac64b(const open_addr_t *ip128b, open_addr_t *prefix64btoWrite, open_addr_t *mac64btoWrite);

void packetfunctions_mac64bToIp128b(const open_addr_t *prefix64b, const open_addr_t *mac64b, open_addr_t *ip128bToWrite);

void packetfunctions_mac64bToMac16b(const open_addr_t *mac64b, open_addr_t *mac16btoWrite);

void packetfunctions_mac16bToMac64b(const open_addr_t *mac16b, open_addr_t *mac64btoWrite);

// address recognition
bool packetfunctions_isBroadcastMulticast(const open_addr_t *address);

bool packetfunctions_isAllRoutersMulticast(const open_addr_t *address);

bool packetfunctions_isAllHostsMulticast(const open_addr_t *address);

bool packetfunctions_sameAddress(const open_addr_t *address_1, const open_addr_t *address_2);

bool packetfunctions_isLinkLocal(const open_addr_t *address);

// read/write addresses to/from packets
void packetfunctions_readAddress(const uint8_t *payload, uint8_t type, open_addr_t *writeToAddress, bool littleEndian);

owerror_t packetfunctions_writeAddress(OpenQueueEntry_t **msg, const open_addr_t *address, bool littleEndian);

// reserving/tossing headers and footers
owerror_t packetfunctions_reserveHeader(OpenQueueEntry_t **pkt, uint16_t header_length);

void packetfunctions_tossHeader(OpenQueueEntry_t **pkt, uint16_t header_length);

owerror_t packetfunctions_reserveFooter(OpenQueueEntry_t **pkt, uint16_t footer_length);

void packetfunctions_tossFooter(OpenQueueEntry_t **pkt, uint16_t footer_length);

// packet duplication
void packetfunctions_duplicatePacket(OpenQueueEntry_t *dst, const OpenQueueEntry_t *src);

// calculate CRC
void packetfunctions_calculateCRC(OpenQueueEntry_t *msg);

bool packetfunctions_checkCRC(const OpenQueueEntry_t *msg);

// calculate checksum
void packetfunctions_calculateChecksum(OpenQueueEntry_t *msg, uint8_t *checksum_ptr);

// endianness
void packetfunctions_htons(uint16_t val, uint8_t *dest);

uint16_t packetfunctions_ntohs(uint8_t *src);

void packetfunctions_htonl(uint32_t val, uint8_t *dest);

uint32_t packetfunctions_ntohl(uint8_t *src);

void packetfunctions_reverseArrayByteOrder(uint8_t *start, uint8_t len);

/**
\}
\}
*/

#endif  /* OPENWSN_PACKETFUNCTIONS_H */
