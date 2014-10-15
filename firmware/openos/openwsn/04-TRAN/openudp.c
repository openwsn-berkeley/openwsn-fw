#include "openwsn.h"
#include "openudp.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "forwarding.h"
#include "openqueue.h"
//UDP applications
#include "opencoap.h"
#include "udpecho.h"
#include "udpinject.h"
#include "udpprint.h"
#include "udprand.h"
#include "udpstorm.h"
#include "udplatency.h"
//#include "heli.h"
//#include "imu.h"

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
      /*    
      case WKP_UDP_HELI:
         appudpheli_sendDone(msg,error);
         break;
      case WKP_UDP_IMU:
         appudpgina_sendDone(msg,error);
         break;
      */
      case WKP_UDP_ECHO:
         udpecho_sendDone(msg,error);
         break;
      case WKP_UDP_INJECT:
         udpinject_sendDone(msg,error);
         break;
      case WKP_UDP_DISCARD:
         udpprint_sendDone(msg,error);
         break;
      case WKP_UDP_RAND:
         udprand_sendDone(msg,error);
         break;
      case WKP_UDP_LATENCY:
         udplatency_sendDone(msg,error);
         break;
      case WKP_UDP_RINGMASTER:
         //udpprint_sendDone(msg, error);
         printf("send message in openudp\n");
         openqueue_freePacketBuffer(msg);
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
      /* 
      case WKP_UDP_HELI:
         appudpheli_receive(msg);
         break;
      case WKP_UDP_IMU:
         imu_receive(msg);
         break;
      */
      case WKP_UDP_ECHO:
         udpecho_receive(msg);
         break;
      case WKP_UDP_INJECT:
         udpinject_receive(msg);
         break;
      case WKP_UDP_DISCARD:
         udpprint_receive(msg);
         break;
      case WKP_UDP_RAND:
         udprand_receive(msg);
         break;
      case WKP_UDP_RINGMASTER:
         printf("received message in openudp %i\n", msg->l4_payload[0]);
         if (msg->l4_payload[0] > 90) {
            printf("blinked! \n");
            sendMsgToRingmasterFrom4('B');
         }

         openqueue_freePacketBuffer(msg);
         break;
      default:
         openserial_printError(COMPONENT_OPENUDP,ERR_UNSUPPORTED_PORT_NUMBER,
                               (errorparameter_t)msg->l4_destination_port,
                               (errorparameter_t)6);
         openqueue_freePacketBuffer(msg);         
   }
}

void sendMsgToRingmasterFrom4(char actionMsg) {
      OpenQueueEntry_t* pkt;
      owerror_t outcome;
      uint8_t numOptions;

      pkt = openqueue_getFreePacketBuffer(COMPONENT_RRT);
      if (pkt == NULL) {
          openserial_printError(COMPONENT_RRT,ERR_BUSY_SENDING,
                                (errorparameter_t)0,
                                (errorparameter_t)0);
          openqueue_freePacketBuffer(pkt);
          return;
      }

      pkt->creator   = COMPONENT_RRT;
      pkt->owner      = COMPONENT_RRT;
      pkt->l4_protocol  = IANA_UDP;

      packetfunctions_reserveHeaderSize(pkt, 1);
      pkt->payload[0] = actionMsg;

      numOptions = 0;
      // location-path option
      packetfunctions_reserveHeaderSize(pkt,sizeof("rt")-1);
      memcpy(&pkt->payload[0], "rt",sizeof("rt")-1);
      packetfunctions_reserveHeaderSize(pkt,1);
      pkt->payload[0]                  = (COAP_OPTION_NUM_URIPATH) << 4 |
         sizeof("rt")-1;
       numOptions++;
      // content-type option
      packetfunctions_reserveHeaderSize(pkt,2);
      pkt->payload[0]                  = COAP_OPTION_NUM_CONTENTFORMAT << 4 |
         1;
      pkt->payload[1]                  = COAP_MEDTYPE_APPOCTETSTREAM;
      numOptions++;

      //metada
      pkt->l4_destination_port   = WKP_UDP_RINGMASTER; 
      pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_RINGMASTER;
      pkt->l3_destinationAdd.type = ADDR_128B;
      memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_ringmaster, 16);
      printf("sending back to ringmaster from openudp\n");
      //send
      outcome = opencoap_send(pkt,
                              COAP_TYPE_CON,
                              COAP_CODE_REQ_PUT,
                              numOptions,
                              &rrt_vars.desc);
      

      if (outcome == E_FAIL) {
        openqueue_freePacketBuffer(pkt);
      }
}

bool openudp_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
