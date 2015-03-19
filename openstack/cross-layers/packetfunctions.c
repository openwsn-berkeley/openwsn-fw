#include "packetfunctions.h"
#include "openserial.h"
#include "idmanager.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void onesComplementSum(uint8_t* global_sum, uint8_t* ptr, int length);

//=========================== public ==========================================

//======= address translation

//assuming an ip128b is a concatenation of prefix64b followed by a mac64b
void packetfunctions_ip128bToMac64b(
      open_addr_t* ip128b,
      open_addr_t* prefix64btoWrite,
      open_addr_t* mac64btoWrite) {
   if (ip128b->type!=ADDR_128B) {
      openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                            (errorparameter_t)ip128b->type,
                            (errorparameter_t)0);
      mac64btoWrite->type=ADDR_NONE;
      return;
   }
   prefix64btoWrite->type=ADDR_PREFIX;
   memcpy(prefix64btoWrite->prefix, &(ip128b->addr_128b[0]), 8);
   mac64btoWrite->type=ADDR_64B;
   memcpy(mac64btoWrite->addr_64b , &(ip128b->addr_128b[8]), 8);
}
void packetfunctions_mac64bToIp128b(
      open_addr_t* prefix64b,
      open_addr_t* mac64b,
      open_addr_t* ip128bToWrite) {
   if (prefix64b->type!=ADDR_PREFIX || mac64b->type!=ADDR_64B) {
      openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                            (errorparameter_t)prefix64b->type,
                            (errorparameter_t)1);
      ip128bToWrite->type=ADDR_NONE;
      return;
   }
   ip128bToWrite->type=ADDR_128B;
   memcpy(&(ip128bToWrite->addr_128b[0]), &(prefix64b->prefix[0]), 8);
   memcpy(&(ip128bToWrite->addr_128b[8]), &(mac64b->addr_64b[0]),  8);
}

//assuming an mac16b is lower 2B of mac64b
void packetfunctions_mac64bToMac16b(open_addr_t* mac64b, open_addr_t* mac16btoWrite) {
   if (mac64b->type!=ADDR_64B) {
      openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                            (errorparameter_t)mac64b->type,
                            (errorparameter_t)2);
      mac16btoWrite->type=ADDR_NONE;
      return;
   }
   mac16btoWrite->type = ADDR_16B;
   mac16btoWrite->addr_16b[0] = mac64b->addr_64b[6];
   mac16btoWrite->addr_16b[1] = mac64b->addr_64b[7];
}
void packetfunctions_mac16bToMac64b(open_addr_t* mac16b, open_addr_t* mac64btoWrite) {
   if (mac16b->type!=ADDR_16B) {
      openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                            (errorparameter_t)mac16b->type,
                            (errorparameter_t)3);
      mac64btoWrite->type=ADDR_NONE;
      return;
   }
   mac64btoWrite->type = ADDR_64B;
   mac64btoWrite->addr_64b[0] = 0;
   mac64btoWrite->addr_64b[1] = 0;
   mac64btoWrite->addr_64b[2] = 0;
   mac64btoWrite->addr_64b[3] = 0;
   mac64btoWrite->addr_64b[4] = 0;
   mac64btoWrite->addr_64b[5] = 0;
   mac64btoWrite->addr_64b[6] = mac16b->addr_16b[0];
   mac64btoWrite->addr_64b[7] = mac16b->addr_16b[1];
}

//======= address recognition

bool packetfunctions_isBroadcastMulticast(open_addr_t* address) {
   uint8_t i;
   uint8_t address_length;
   //IPv6 multicast
   if (address->type==ADDR_128B) {
      if (address->addr_128b[0]==0xff) {
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
         openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)4);
         return FALSE;
   }
   for (i=0;i<address_length;i++) {
      if (address->addr_128b[i]!=0xFF) {
         return FALSE;
      }
   }
   return TRUE;
}

bool packetfunctions_isAllRoutersMulticast(open_addr_t* address) {
   if (
      address->type          == ADDR_128B &&
      address->addr_128b[0]  == 0xff &&
      address->addr_128b[1]  == 0x02 &&
      address->addr_128b[2]  == 0x00 &&
      address->addr_128b[3]  == 0x00 &&
      address->addr_128b[4]  == 0x00 &&
      address->addr_128b[5]  == 0x00 &&
      address->addr_128b[6]  == 0x00 &&
      address->addr_128b[7]  == 0x00 &&
      address->addr_128b[8]  == 0x00 &&
      address->addr_128b[9]  == 0x00 &&
      address->addr_128b[10] == 0x00 &&
      address->addr_128b[11] == 0x00 &&
      address->addr_128b[12] == 0x00 &&
      address->addr_128b[13] == 0x00 &&
      address->addr_128b[14] == 0x00 &&
      address->addr_128b[15] == 0x02
   ) {
      return TRUE;
   }
   return FALSE;
}

bool packetfunctions_isAllHostsMulticast(open_addr_t* address) {
   if (
      address->type          == ADDR_128B &&
      address->addr_128b[0]  == 0xff &&
      address->addr_128b[1]  == 0x02 &&
      address->addr_128b[2]  == 0x00 &&
      address->addr_128b[3]  == 0x00 &&
      address->addr_128b[4]  == 0x00 &&
      address->addr_128b[5]  == 0x00 &&
      address->addr_128b[6]  == 0x00 &&
      address->addr_128b[7]  == 0x00 &&
      address->addr_128b[8]  == 0x00 &&
      address->addr_128b[9]  == 0x00 &&
      address->addr_128b[10] == 0x00 &&
      address->addr_128b[11] == 0x00 &&
      address->addr_128b[12] == 0x00 &&
      address->addr_128b[13] == 0x00 &&
      address->addr_128b[14] == 0x00 &&
      address->addr_128b[15] == 0x01
   ) {
      return TRUE;
   }
   return FALSE;
}

bool packetfunctions_sameAddress(open_addr_t* address_1, open_addr_t* address_2) {
   uint8_t address_length;
   
   if (address_1->type!=address_2->type) {
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
         openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address_1->type,
                               (errorparameter_t)5);
         return FALSE;
   }
   if (memcmp((void*)address_1->addr_128b,(void*)address_2->addr_128b,address_length)==0) {
      return TRUE;
   }
   return FALSE;
}

//======= address read/write

void packetfunctions_readAddress(uint8_t* payload, uint8_t type, open_addr_t* writeToAddress, bool littleEndian) {
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
         openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)type,
                               (errorparameter_t)6);
         return;
   }
   
   for (i=0;i<address_length;i++) {
      if (littleEndian) {
         writeToAddress->addr_128b[address_length-1-i] = *(payload+i);
      } else {
         writeToAddress->addr_128b[i]   = *(payload+i);
      }
   }
}

void packetfunctions_writeAddress(OpenQueueEntry_t* msg, open_addr_t* address, bool littleEndian) {
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
         openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)address->type,
                               (errorparameter_t)7);
         return;
   }
   
   for (i=0;i<address_length;i++) {
      msg->payload      -= sizeof(uint8_t);
      msg->length       += sizeof(uint8_t);
      if (littleEndian) {
         *((uint8_t*)(msg->payload)) = address->addr_128b[i];
      } else {
         *((uint8_t*)(msg->payload)) = address->addr_128b[address_length-1-i];
      }
   }
}

//======= reserving/tossing headers

void packetfunctions_reserveHeaderSize(OpenQueueEntry_t* pkt, uint8_t header_length) {
   pkt->payload -= header_length;
   pkt->length  += header_length;
   if ( (uint8_t*)(pkt->payload) < (uint8_t*)(pkt->packet) ) {
      openserial_printCritical(COMPONENT_PACKETFUNCTIONS,ERR_HEADER_TOO_LONG,
                            (errorparameter_t)0,
                            (errorparameter_t)pkt->length);
   }
}

void packetfunctions_tossHeader(OpenQueueEntry_t* pkt, uint8_t header_length) {
   pkt->payload += header_length;
   pkt->length  -= header_length;
   if ( (uint8_t*)(pkt->payload) > (uint8_t*)(pkt->packet+126) ) {
      openserial_printError(COMPONENT_PACKETFUNCTIONS,ERR_HEADER_TOO_LONG,
                            (errorparameter_t)1,
                            (errorparameter_t)pkt->length);
   }
}

void packetfunctions_reserveFooterSize(OpenQueueEntry_t* pkt, uint8_t header_length) {
   pkt->length  += header_length;
   if (pkt->length>127) {
      openserial_printError(COMPONENT_PACKETFUNCTIONS,ERR_HEADER_TOO_LONG,
                            (errorparameter_t)2,
                            (errorparameter_t)pkt->length);
   }
}

void packetfunctions_tossFooter(OpenQueueEntry_t* pkt, uint8_t header_length) {
   pkt->length  -= header_length;
   if (pkt->length>128) {//wraps around, so a negative value will be >128
      openserial_printError(COMPONENT_PACKETFUNCTIONS,ERR_HEADER_TOO_LONG,
                            (errorparameter_t)3,
                            (errorparameter_t)pkt->length);
   }
}

//======= CRC calculation

void packetfunctions_calculateCRC(OpenQueueEntry_t* msg) {
   uint16_t crc;
   uint8_t  i;
   uint8_t  count;
   crc = 0;
   for (count=1;count<msg->length-2;count++) {
      crc = crc ^ (uint8_t)*(msg->payload+count);
      //crc = crc ^ (uint16_t)*ptr++ << 8;
      for (i=0;i<8;i++) {
         if (crc & 0x1) {
            crc = crc >> 1 ^ 0x8408;
         } else {
            crc = crc >> 1;
         }
      }
   }
   *(msg->payload+(msg->length-2)) = crc%256;
   *(msg->payload+(msg->length-1)) = crc/256;
}

bool packetfunctions_checkCRC(OpenQueueEntry_t* msg) {
   uint16_t crc;
   uint8_t  i;
   uint8_t  count;
   crc = 0;
   for (count=0;count<msg->length-2;count++) {
      crc = crc ^ (uint8_t)*(msg->payload+count);
      //crc = crc ^ (uint16_t)*ptr++ << 8;
      for (i=0;i<8;i++) {
         if (crc & 0x1) {
            crc = crc >> 1 ^ 0x8408;
         } else {
            crc = crc >> 1;
         }
      }
   }
   if (*(msg->payload+(msg->length-2))==crc%256 &&
       *(msg->payload+(msg->length-1))==crc/256) {
          return TRUE;
       } else {
          return FALSE;
       }
}

//======= checksum calculation

//see http://www-net.cs.umass.edu/kurose/transport/UDP.html, or http://tools.ietf.org/html/rfc1071
//see http://en.wikipedia.org/wiki/User_Datagram_Protocol#IPv6_PSEUDO-HEADER
void packetfunctions_calculateChecksum(OpenQueueEntry_t* msg, uint8_t* checksum_ptr) {
   uint8_t temp_checksum[2];
   uint8_t little_helper[2];
   
   // initialize running checksum
   temp_checksum[0]  = 0;
   temp_checksum[1]  = 0;
   
   //===== IPv6 pseudo header
   
   // source address (prefix and EUI64)
   onesComplementSum(temp_checksum,(idmanager_getMyID(ADDR_PREFIX))->prefix,8);
   onesComplementSum(temp_checksum,(idmanager_getMyID(ADDR_64B))->addr_64b,8);
   
   // destination address
   onesComplementSum(temp_checksum,msg->l3_destinationAdd.addr_128b,16);
   
   // length
   little_helper[0] = 0;
   little_helper[1] = msg->length;
   onesComplementSum(temp_checksum,little_helper,2);
   
   // next header
   little_helper[0] = 0;
   little_helper[1] = msg->l4_protocol;
   onesComplementSum(temp_checksum,little_helper,2);
   
   //===== payload
   
   // reset the checksum currently in the payload
   *checksum_ptr     = 0;
   *(checksum_ptr+1) = 0;
   
   onesComplementSum(temp_checksum,msg->payload,msg->length);
   temp_checksum[0] ^= 0xFF;
   temp_checksum[1] ^= 0xFF;
   
   //write in packet
   *checksum_ptr     = temp_checksum[0];
   *(checksum_ptr+1) = temp_checksum[1];
}


void onesComplementSum(uint8_t* global_sum, uint8_t* ptr, int length) {
   uint32_t sum = 0xFFFF & (global_sum[0]<<8 | global_sum[1]);
   while (length>1) {
      sum     += 0xFFFF & (*ptr<<8 | *(ptr+1));
      ptr     += 2;
      length  -= 2;
   }
   if (length) {
      sum     += (0xFF & *ptr)<<8;
   }
   while (sum>>16) {
      sum      = (sum & 0xFFFF)+(sum >> 16);
   }
   global_sum[0] = (sum>>8) & 0xFF;
   global_sum[1] = sum & 0xFF;
}

//======= endianness

void packetfunctions_htons( uint16_t val, uint8_t* dest ) {
   dest[0] = (val & 0xff00) >> 8;
   dest[1] = (val & 0x00ff);
}

uint16_t packetfunctions_ntohs( uint8_t* src ) {
   return (((uint16_t) src[0]) << 8) |
      (((uint16_t) src[1])
      );
}

void packetfunctions_htonl( uint32_t val, uint8_t* dest ) {
   dest[0] = (val & 0xff000000) >> 24;
   dest[1] = (val & 0x00ff0000) >> 16;
   dest[2] = (val & 0x0000ff00) >> 8;
   dest[3] = (val & 0x000000ff);
}

uint32_t packetfunctions_ntohl( uint8_t* src ) {
   return (((uint32_t) src[0]) << 24) |
      (((uint32_t) src[1]) << 16)     |
      (((uint32_t) src[2]) << 8)      |
      (((uint32_t) src[3])
      );
}

//=========================== private =========================================