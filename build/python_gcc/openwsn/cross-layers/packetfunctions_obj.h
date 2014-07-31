/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:16.377420.
*/
#ifndef __PACKETFUNCTIONS_H
#define __PACKETFUNCTIONS_H

/**
\addtogroup cross-layers
\{
\addtogroup PacketFunctions
\{
*/

#include "openwsn_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// address translation
void packetfunctions_ip128bToMac64b(OpenMote* self, open_addr_t* ip128b,open_addr_t* prefix64btoWrite,open_addr_t* mac64btoWrite);
void packetfunctions_mac64bToIp128b(OpenMote* self, open_addr_t* prefix64b,open_addr_t* mac64b,open_addr_t* ip128bToWrite);
void packetfunctions_mac64bToMac16b(OpenMote* self, open_addr_t* mac64b, open_addr_t* mac16btoWrite);
void packetfunctions_mac16bToMac64b(OpenMote* self, open_addr_t* mac16b, open_addr_t* mac64btoWrite);

// address recognition
bool packetfunctions_isBroadcastMulticast(OpenMote* self, open_addr_t* address);
bool packetfunctions_isAllRoutersMulticast(OpenMote* self, open_addr_t* address);
bool packetfunctions_isAllHostsMulticast(OpenMote* self, open_addr_t* address);
bool packetfunctions_sameAddress(OpenMote* self, open_addr_t* address_1, open_addr_t* address_2);

// read/write addresses to/from packets
void packetfunctions_readAddress(OpenMote* self, uint8_t* payload, uint8_t type, open_addr_t* writeToAddress, bool littleEndian);
void packetfunctions_writeAddress(OpenMote* self, OpenQueueEntry_t* msg, open_addr_t* address, bool littleEndian);

// reserving/tossing headers and footers
void packetfunctions_reserveHeaderSize(OpenMote* self, OpenQueueEntry_t* pkt, uint8_t header_length);
void packetfunctions_tossHeader(OpenMote* self, OpenQueueEntry_t* pkt, uint8_t header_length);
void packetfunctions_reserveFooterSize(OpenMote* self, OpenQueueEntry_t* pkt, uint8_t header_length);
void packetfunctions_tossFooter(OpenMote* self, OpenQueueEntry_t* pkt, uint8_t header_length);

// calculate CRC
void packetfunctions_calculateCRC(OpenMote* self, OpenQueueEntry_t* msg);
bool packetfunctions_checkCRC(OpenMote* self, OpenQueueEntry_t* msg);

// calculate checksum
void packetfunctions_calculateChecksum(OpenMote* self, OpenQueueEntry_t* msg, uint8_t* checksum_ptr);

// endianness
void packetfunctions_htons(OpenMote* self,  uint16_t val, uint8_t* dest );
uint16_t packetfunctions_ntohs(OpenMote* self,  uint8_t* src );
void packetfunctions_htonl(OpenMote* self,  uint32_t val, uint8_t* dest );
uint32_t packetfunctions_ntohl( uint8_t* src );

/**
\}
\}
*/

#endif
