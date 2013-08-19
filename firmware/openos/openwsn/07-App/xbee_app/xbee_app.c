#include "openwsn.h"
#include "udpinject.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void xbeeapp_init() {
}

void xbeeapp_send(uint8_t frame_id, uint8_t * dest_address, uint8_t * data, uint16_t data_len) {
   OpenQueueEntry_t* pkt;
   open_addr_t * tempaddr;
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[18];
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_XBEEAPP,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   //prepare packet
   pkt = openqueue_getFreePacketBuffer(COMPONENT_XBEEAPP);
   if (pkt==NULL) {
      openserial_printError(COMPONENT_XBEEAPP,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_XBEEAPP;
   pkt->owner                       = COMPONENT_XBEEAPP;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_XBEE;
   pkt->l4_destination_port         = WKP_UDP_XBEE;
   pkt->l3_destinationAdd.type      = ADDR_128B;
   tempaddr = idmanager_getMyID(ADDR_PREFIX);
   memcpy(&(pkt->l3_destinationAdd.prefix),&(tempaddr->prefix),8);
   memcpy(&(pkt->l3_destinationAdd.addr_64b),dest_address,8);
   packetfunctions_reserveHeaderSize(pkt,data_len);
   memcpy(&(pkt->payload),data,data_len);
   //send packet
   if ((openudp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void xbeeapp_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_XBEEAPP;
   if (msg->creator!=COMPONENT_XBEEAPP) {
      openserial_printError(COMPONENT_XBEEAPP,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
   
   // send TX success to xbee
}

void xbeeapp_receive(OpenQueueEntry_t* msg) {
   openqueue_freePacketBuffer(msg);
}

bool xbeeapp_debugPrint() {
   return FALSE;
}

void at_command_set(uint8_t atcmd[2], uint8_t * data, uint16_t len) {
  
}

void at_command_get(uint8_t atcmd[2]){
  
}

//=========================== private =========================================