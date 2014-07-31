/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:10:52.097013.
*/
#include "openwsn_obj.h"
#include "openudp_obj.h"
#include "openserial_obj.h"
#include "packetfunctions_obj.h"
#include "forwarding_obj.h"
#include "openqueue_obj.h"
//UDP applications
#include "opencoap_obj.h"
#include "udpecho_obj.h"
#include "udpinject_obj.h"
#include "udpprint_obj.h"
#include "udprand_obj.h"
#include "udpstorm_obj.h"
#include "udplatency_obj.h"
//#include "heli.h"
//#include "imu.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void openudp_init(OpenMote* self) {
}

owerror_t openudp_send(OpenMote* self, OpenQueueEntry_t* msg) {
   msg->owner       = COMPONENT_OPENUDP;
   msg->l4_protocol = IANA_UDP;
   msg->l4_payload  = msg->payload;
   msg->l4_length   = msg->length;
 packetfunctions_reserveHeaderSize(self, msg,sizeof(udp_ht));
 packetfunctions_htons(self, msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
 packetfunctions_htons(self, msg->l4_destination_port,&(msg->payload[2]));
 packetfunctions_htons(self, msg->length,&(msg->payload[4]));
 packetfunctions_calculateChecksum(self, msg,(uint8_t*)&(((udp_ht*)msg->payload)->checksum));
   return forwarding_send(self, msg);
}

void openudp_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_OPENUDP;
   switch(msg->l4_sourcePortORicmpv6Type) {
      case WKP_UDP_COAP:
 opencoap_sendDone(self, msg,error);
         break;
      /*    
      case WKP_UDP_HELI:
         appudpheli_sendDone(msg,error);
         break;
      case WKP_UDP_IMU:
         appudpgina_sendDone(msg,error);
         break;
      */
      case WKP_UDP_ECHO:
 udpecho_sendDone(self, msg,error);
         break;
      case WKP_UDP_INJECT:
 udpinject_sendDone(self, msg,error);
         break;
      case WKP_UDP_DISCARD:
 udpprint_sendDone(self, msg,error);
         break;
      case WKP_UDP_RAND:
 udprand_sendDone(self, msg,error);
         break;
      case WKP_UDP_LATENCY:
 udplatency_sendDone(self, msg,error);
         break;
         
      default:
// openserial_printError(self, COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
//                               (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
//                               (errorparameter_t)5);
 openqueue_freePacketBuffer(self, msg);         
   }
}

void openudp_receive(OpenMote* self, OpenQueueEntry_t* msg) {
   uint8_t temp_8b;
      
   msg->owner                      = COMPONENT_OPENUDP;
   if (msg->l4_protocol_compressed==TRUE) {
      // get the UDP header encoding byte
      temp_8b = *((uint8_t*)(msg->payload));
 packetfunctions_tossHeader(self, msg,sizeof(temp_8b));
      switch (temp_8b & NHC_UDP_PORTS_MASK) {
         case NHC_UDP_PORTS_INLINE:
            // source port:         16 bits in-line
            // dest port:           16 bits in-line
            msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
            msg->l4_destination_port        = msg->payload[2]*256+msg->payload[3];
 packetfunctions_tossHeader(self, msg,2+2);
            break;
         case NHC_UDP_PORTS_16S_8D:
            // source port:         16 bits in-line
            // dest port:   0xf0  +  8 bits in-line
            msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
            msg->l4_destination_port        = 0xf000 +            msg->payload[2];
 packetfunctions_tossHeader(self, msg,2+1);
            break;
         case NHC_UDP_PORTS_8S_8D:
            // source port: 0xf0  +  8 bits in-line
            // dest port:   0xf0  +  8 bits in-line
            msg->l4_sourcePortORicmpv6Type  = 0xf000 +            msg->payload[0];
            msg->l4_destination_port        = 0xf000 +            msg->payload[1];
 packetfunctions_tossHeader(self, msg,1+1);
            break;
         case NHC_UDP_PORTS_4S_4D:
            // source port: 0xf0b +  4 bits in-line
            // dest port:   0xf0b +  4 bits in-line
            msg->l4_sourcePortORicmpv6Type  = 0xf0b0 + ((msg->payload[0] >> 4) & 0x0f);
            msg->l4_destination_port        = 0xf0b0 + ((msg->payload[0] >> 0) & 0x0f);
 packetfunctions_tossHeader(self, msg,1);
            break;
      }
   } else {
      msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
      msg->l4_destination_port        = msg->payload[2]*256+msg->payload[3];
 packetfunctions_tossHeader(self, msg,sizeof(udp_ht));
   }
   
   switch(msg->l4_destination_port) {
      case WKP_UDP_COAP:
 opencoap_receive(self, msg);
         break;
      /* 
      case WKP_UDP_HELI:
         appudpheli_receive(msg);
         break;
      case WKP_UDP_IMU:
         imu_receive(msg);
         break;
      */
      case WKP_UDP_ECHO:
 udpecho_receive(self, msg);
         break;
      case WKP_UDP_INJECT:
 udpinject_receive(self, msg);
         break;
      case WKP_UDP_DISCARD:
 udpprint_receive(self, msg);
         break;
      case WKP_UDP_RAND:
 udprand_receive(self, msg);
         break;
      default:
// openserial_printError(self, COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
//                               (errorparameter_t)msg->l4_destination_port,
//                               (errorparameter_t)6);
 openqueue_freePacketBuffer(self, msg);         
   }
}

bool openudp_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================
