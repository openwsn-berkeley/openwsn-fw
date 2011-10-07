#include "openwsn.h"
#include "udpchannel.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "openudp.h"

//=========================== variables =======================================

typedef struct {
   uint8_t new_channel;
} appudpchannel_vars_t;

appudpchannel_vars_t appudpchannel_vars;

//=========================== prototypes ======================================

void appudpchannel_change_channel(uint8_t new_channel);

//=========================== public ==========================================

void appudpchannel_init() {
   appudpchannel_vars.new_channel = 0;
}

void appudpchannel_trigger() {
   OpenQueueEntry_t* pkt;
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[1];
   //get command from OpenSerial (16B IPv6 destination address, 1B new_channel)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_APPUDPCHANNEL,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   appudpchannel_vars.new_channel = input_buffer[16];
   if (appudpchannel_vars.new_channel<11 && appudpchannel_vars.new_channel>26) {
      appudpchannel_vars.new_channel = 0;
      return;
   }
   //prepare packet
   pkt = openqueue_getFreePacketBuffer();
   if (pkt==NULL) {
      openserial_printError(COMPONENT_APPUDPCHANNEL,ERR_NO_FREE_PACKET_BUFFER,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
      return;
   }
   pkt->creator                     = COMPONENT_APPUDPCHANNEL;
   pkt->owner                       = COMPONENT_APPUDPCHANNEL;
   pkt->l4_protocol                 = IANA_UDP;
   pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_CHANNEL;
   pkt->l4_destination_port         = WKP_UDP_CHANNEL;
   pkt->l3_destinationORsource.type = ADDR_128B;
   memcpy(&(pkt->l3_destinationORsource.addr_128b[0]),&(input_buffer[0]),16);   
   packetfunctions_reserveHeaderSize(pkt,1);
   ((uint8_t*)pkt->payload)[0] = appudpchannel_vars.new_channel;
   //send packet
   if ((openudp_send(pkt))==E_FAIL) {
      openqueue_freePacketBuffer(pkt);
   }
}

void appudpchannel_sendDone(OpenQueueEntry_t* msg, error_t error) {
   if (error==E_SUCCESS && appudpchannel_vars.new_channel!=0) {
      appudpchannel_change_channel(appudpchannel_vars.new_channel);
      appudpchannel_vars.new_channel = 0;
   }
   openqueue_freePacketBuffer(msg);
}

void appudpchannel_receive(OpenQueueEntry_t* msg) {
   appudpchannel_change_channel(((uint8_t*)msg->payload)[0]);
   openqueue_freePacketBuffer(msg);
}

bool appudpchannel_debugPrint() {
   return FALSE;
}

//=========================== private =========================================

void appudpchannel_change_channel(uint8_t new_channel) {
   if (new_channel>=11 && new_channel<=26) {
      //openwsn_frequency_channel = new_channel;
   }
}