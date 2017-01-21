#include "opendefs.h"
#include "openudp.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "forwarding.h"
#include "openqueue.h"
// applications
#include "opencoap.h"
#include "uecho.h"
#include "uinject.h"
#include "userialbridge.h"
#include "rrt.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void openudp_init() {
}

owerror_t openudp_send(OpenQueueEntry_t* msg) {
   uint8_t* checksum_position;
   msg->owner       = COMPONENT_OPENUDP;
   msg->l4_protocol = IANA_UDP;
   msg->l4_payload  = msg->payload;
   msg->l4_length   = msg->length;

   msg->l4_protocol_compressed = FALSE; // by default
   uint8_t compType = NHC_UDP_PORTS_INLINE;

   //check if the header can be compressed.
   if (msg->l4_destination_port>=0xf000 && msg->l4_destination_port<0xf100){
        // destination can be compressed 8 bit
        msg->l4_protocol_compressed = TRUE;
        compType = NHC_UDP_PORTS_16S_8D;
        //check source now
        if (msg->l4_sourcePortORicmpv6Type>=0xf000 && msg->l4_sourcePortORicmpv6Type<0xf100){
          //source can be compressed -> 8bit
           compType = NHC_UDP_PORTS_8S_8D;
        }
        //check now if both can still be more compressed 4b each
        if (msg->l4_destination_port>=0xf0b0 &&
            msg->l4_destination_port<=0xf0bf &&
            msg->l4_sourcePortORicmpv6Type>=0xf0b0 &&
            msg->l4_sourcePortORicmpv6Type<=0xf0bf){
            //can be fully compressed
            compType = NHC_UDP_PORTS_4S_4D;
        }
   }

   // fill in the header in the packet
   if (msg->l4_protocol_compressed){

       //add checksum space in the packet.
       packetfunctions_reserveHeaderSize(msg,2);
       //keep position and calculatre checksum at the end.
       checksum_position = &msg->payload[0];

       //length is always omitted
       /*
        RFC6282 -> The UDP Length field
                   MUST always be elided and is inferred from lower layers using the
                   6LoWPAN Fragmentation header or the IEEE 802.15.4 header.
       */

       switch (compType) {
          case NHC_UDP_PORTS_INLINE:
              // error this is not possible.
              break;
          case NHC_UDP_PORTS_16S_8D:
             // dest port:   0xf0  +  8 bits in-line
             // source port:         16 bits in-line
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = (uint8_t) (msg->l4_destination_port & 0x00ff);
             packetfunctions_reserveHeaderSize(msg,2);
             packetfunctions_htons(msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
             //write proper LOWPAN_NHC
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = NHC_UDP_ID|NHC_UDP_PORTS_16S_8D;
             break;
          case NHC_UDP_PORTS_8S_8D:
             // dest port:   0xf0  +  8 bits in-line
             // source port: 0xf0  +  8 bits in-line
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = (uint8_t) (msg->l4_destination_port & 0x00ff);
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = (uint8_t) (msg->l4_sourcePortORicmpv6Type & 0x00ff);
             //write proper LOWPAN_NHC
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = NHC_UDP_ID|NHC_UDP_PORTS_8S_8D;
             break;
          case NHC_UDP_PORTS_4S_4D:
             // source port: 0xf0b +  4 bits in-line (high 4)
             // dest port:   0xf0b +  4 bits in-line  (low 4)
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = (msg->l4_sourcePortORicmpv6Type & 0x000f)<<4;
             msg->payload[0] |= (msg->l4_destination_port & 0x000f);
             //write proper LOWPAN_NHC
             packetfunctions_reserveHeaderSize(msg,1);
             msg->payload[0] = NHC_UDP_ID|NHC_UDP_PORTS_4S_4D;
             break;
       }

       //after filling the packet we calculate the checksum
       packetfunctions_calculateChecksum(msg,checksum_position);
       printf("sending udp packet\n");

   } else{
       packetfunctions_reserveHeaderSize(msg,sizeof(udp_ht));
       packetfunctions_htons(msg->l4_sourcePortORicmpv6Type,&(msg->payload[0]));
       packetfunctions_htons(msg->l4_destination_port,&(msg->payload[2]));
       //TODO check this as the lenght MUST be ommited.
       packetfunctions_htons(msg->length,&(msg->payload[4]));
       packetfunctions_calculateChecksum(msg,(uint8_t*)&(((udp_ht*)msg->payload)->checksum));
   }
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
      case WKP_UDP_INJECT:
         uinject_sendDone(msg,error);
         break;
      case WKP_UDP_SERIALBRIDGE:
         userialbridge_sendDone(msg,error);
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
      case WKP_UDP_INJECT:
         uinject_receive(msg);
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
