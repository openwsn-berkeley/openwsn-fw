#include "openwsn.h"
#include "appudpsensor.h"
#include "udp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void appudpsensor_init() {
}

void appudpsensor_trigger() {
   OpenQueueEntry_t* pkt;
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[18];
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_APPUDPSENSOR,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   //prepare packet
   pkt = openqueue_getFreePacketBuffer();
   if (pkt==NULL) {
      openserial_printError(COMPONENT_APPUDPSENSOR,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_APPUDPSENSOR;
   pkt->owner                       = COMPONENT_APPUDPSENSOR;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_SENSOR;
   pkt->l4_destination_port         = packetfunctions_ntohs(&(input_buffer[16]));
   pkt->l3_destinationORsource.type = ADDR_128B;
   memcpy(&(pkt->l3_destinationORsource.addr_128b[0]),&(input_buffer[0]),16);
   packetfunctions_reserveHeaderSize(pkt,6);
   ((uint8_t*)pkt->payload)[0]      = 'p';
   ((uint8_t*)pkt->payload)[1]      = 'o';
   ((uint8_t*)pkt->payload)[2]      = 'i';
   ((uint8_t*)pkt->payload)[3]      = 'p';
   ((uint8_t*)pkt->payload)[4]      = 'o';
   ((uint8_t*)pkt->payload)[5]      = 'i';
   //send packet
   if ((udp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void appudpsensor_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPUDPSENSOR;
   if (msg->creator!=COMPONENT_APPUDPSENSOR) {
      openserial_printError(COMPONENT_APPUDPSENSOR,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

void appudpsensor_receive(OpenQueueEntry_t* msg) {
   openqueue_freePacketBuffer(msg);
}

bool appudpsensor_debugPrint() {
   return FALSE;
}

//=========================== private =========================================