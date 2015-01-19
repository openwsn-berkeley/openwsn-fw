#include "opendefs.h"
#include "openudp.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "forwarding.h"
#include "openqueue.h"
// applications
#include "opencoap.h"
#include "uecho.h"
#include "rrt.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void openudp_init() {
}

owerror_t openudp_send(OpenQueueEntry_t* msg) {
   msg->owner       = COMPONENT_OPENUDP;
   msg->l4_protocol = IANA_UDP;
   msg->l4_payload  = msg->payload;
   msg->l4_length   = msg->length;
   packetfunctions_reserveHeaderSize(msg,sizeof(udp_ht));
   packetfunctions_htons(msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
   packetfunctions_htons(msg->l4_destination_port,&(msg->payload[2]));
   packetfunctions_htons(msg->length,&(msg->payload[4]));
   packetfunctions_calculateChecksum(msg,(uint8_t*)&(((udp_ht*)msg->payload)->checksum));
   return forwarding_send(msg);
}

void openudp_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_OPENUDP;
   switch(msg->l4_sourcePortORicmpv6Type) {
      case WKP_UDP_COAP:
         opencoap_sendDone(msg,error);
         break;
      case WKP_UDP_ECHO:
         uecho_sendDone(msg,error);
         break;
      case WKP_UDP_RINGMASTER:
	 //udpprint_sendDone(msg, error);
         rrt_sendDone(msg, error);
         break;
      default:
         openserial_printError(COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
                               (errorparameter_t)msg->l4_sourcePortORicmpv6Type,
                               (errorparameter_t)5);
         openqueue_freePacketBuffer(msg);         
   }
}

void openudp_receive(OpenQueueEntry_t* msg) {
   uint8_t temp_8b;
      
   msg->owner                      = COMPONENT_OPENUDP;
   if (msg->l4_protocol_compressed==TRUE) {
      // get the UDP header encoding byte
      temp_8b = *((uint8_t*)(msg->payload));
      packetfunctions_tossHeader(msg,sizeof(temp_8b));
      switch (temp_8b & NHC_UDP_PORTS_MASK) {
         case NHC_UDP_PORTS_INLINE:
            // source port:         16 bits in-line
            // dest port:           16 bits in-line
            msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
            msg->l4_destination_port        = msg->payload[2]*256+msg->payload[3];
            packetfunctions_tossHeader(msg,2+2);
            break;
         case NHC_UDP_PORTS_16S_8D:
            // source port:         16 bits in-line
            // dest port:   0xf0  +  8 bits in-line
            msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
            msg->l4_destination_port        = 0xf000 +            msg->payload[2];
            packetfunctions_tossHeader(msg,2+1);
            break;
         case NHC_UDP_PORTS_8S_8D:
            // source port: 0xf0  +  8 bits in-line
            // dest port:   0xf0  +  8 bits in-line
            msg->l4_sourcePortORicmpv6Type  = 0xf000 +            msg->payload[0];
            msg->l4_destination_port        = 0xf000 +            msg->payload[1];
            packetfunctions_tossHeader(msg,1+1);
            break;
         case NHC_UDP_PORTS_4S_4D:
            // source port: 0xf0b +  4 bits in-line
            // dest port:   0xf0b +  4 bits in-line
            msg->l4_sourcePortORicmpv6Type  = 0xf0b0 + ((msg->payload[0] >> 4) & 0x0f);
            msg->l4_destination_port        = 0xf0b0 + ((msg->payload[0] >> 0) & 0x0f);
            packetfunctions_tossHeader(msg,1);
            break;
      }
   } else {
      msg->l4_sourcePortORicmpv6Type  = msg->payload[0]*256+msg->payload[1];
      msg->l4_destination_port        = msg->payload[2]*256+msg->payload[3];
      packetfunctions_tossHeader(msg,sizeof(udp_ht));
   }
   
   switch(msg->l4_destination_port) {
      case WKP_UDP_COAP:
         opencoap_receive(msg);
         break;
      case WKP_UDP_RINGMASTER:
         if (msg->l4_payload[0] > 90) {
            rrt_sendCoAPMsg('B', NULL);
         }

         openqueue_freePacketBuffer(msg);
         break;
      case WKP_UDP_ECHO:
         uecho_receive(msg);
         break;
      default:
         openserial_printError(COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
                               (errorparameter_t)msg->l4_destination_port,
                               (errorparameter_t)6);
         openqueue_freePacketBuffer(msg);         
   }
}

bool openudp_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
