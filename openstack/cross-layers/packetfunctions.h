#ifndef __PACKETFUNCTIONS_H
#define __PACKETFUNCTIONS_H

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
void     packetfunctions_ip128bToMac64b(open_addr_t* ip128b,open_addr_t* prefix64btoWrite,open_addr_t* mac64btoWrite);
void     packetfunctions_mac64bToIp128b(open_addr_t* prefix64b,open_addr_t* mac64b,open_addr_t* ip128bToWrite);
void     packetfunctions_mac64bToMac16b(open_addr_t* mac64b, open_addr_t* mac16btoWrite);
void     packetfunctions_mac16bToMac64b(open_addr_t* mac16b, open_addr_t* mac64btoWrite);

// address recognition
bool     packetfunctions_isBroadcastMulticast(open_addr_t* address);
bool     packetfunctions_isAllRoutersMulticast(open_addr_t* address);
bool     packetfunctions_isAllHostsMulticast(open_addr_t* address);
bool     packetfunctions_sameAddress(open_addr_t* address_1, open_addr_t* address_2);

// read/write addresses to/from packets
void     packetfunctions_readAddress(uint8_t* payload, uint8_t type, open_addr_t* writeToAddress, bool littleEndian);
void     packetfunctions_writeAddress(OpenQueueEntry_t* msg, open_addr_t* address, bool littleEndian);

// reserving/tossing headers and footers
void     packetfunctions_reserveHeaderSize(OpenQueueEntry_t* pkt, uint8_t header_length);
void     packetfunctions_tossHeader(OpenQueueEntry_t* pkt, uint8_t header_length);
void     packetfunctions_reserveFooterSize(OpenQueueEntry_t* pkt, uint8_t header_length);
void     packetfunctions_tossFooter(OpenQueueEntry_t* pkt, uint8_t header_length);

// calculate CRC
void     packetfunctions_calculateCRC(OpenQueueEntry_t* msg);
bool     packetfunctions_checkCRC(OpenQueueEntry_t* msg);

// calculate checksum
void     packetfunctions_calculateChecksum(OpenQueueEntry_t* msg, uint8_t* checksum_ptr);

// endianness
void     packetfunctions_htons( uint16_t val, uint8_t* dest );
uint16_t packetfunctions_ntohs( uint8_t* src );
void     packetfunctions_htonl( uint32_t val, uint8_t* dest );
uint32_t packetfunctions_ntohl( uint8_t* src );

/**
\}
\}
*/

#endif
