#include "openwsn.h"
#include "tcpinject.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opentcp.h"
#include "openqueue.h"

//=========================== variables =======================================

tcpinject_vars_t tcpinject_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void tcpinject_init() {
}

bool tcpinject_shouldIlisten() {
   return FALSE;
}

void tcpinject_trigger() {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[18];
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_TCPINJECT,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   tcpinject_vars.hisAddress.type = ADDR_128B;
   memcpy(&(tcpinject_vars.hisAddress.addr_128b[0]),&(input_buffer[0]),16);
   tcpinject_vars.hisPort = packetfunctions_ntohs(&(input_buffer[16]));
   //connect
   opentcp_connect(&tcpinject_vars.hisAddress,tcpinject_vars.hisPort,WKP_TCP_INJECT);
}

void tcpinject_connectDone(owerror_t error) {
   if (error==E_SUCCESS) {
      tcpinject_vars.pkt = openqueue_getFreePacketBuffer(COMPONENT_TCPINJECT);
      if (tcpinject_vars.pkt==NULL) {
         openserial_printError(COMPONENT_TCPINJECT,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      tcpinject_vars.pkt->creator                      = COMPONENT_TCPINJECT;
      tcpinject_vars.pkt->owner                        = COMPONENT_TCPINJECT;
      tcpinject_vars.pkt->l4_protocol                  = IANA_UDP;
      tcpinject_vars.pkt->l4_sourcePortORicmpv6Type    = WKP_TCP_INJECT;
      tcpinject_vars.pkt->l4_destination_port          = tcpinject_vars.hisPort;
      memcpy(&(tcpinject_vars.pkt->l3_destinationAdd),&tcpinject_vars.hisAddress,sizeof(open_addr_t));
      packetfunctions_reserveHeaderSize(tcpinject_vars.pkt,6);
      ((uint8_t*)tcpinject_vars.pkt->payload)[0] = 'p';
      ((uint8_t*)tcpinject_vars.pkt->payload)[1] = 'o';
      ((uint8_t*)tcpinject_vars.pkt->payload)[2] = 'i';
      ((uint8_t*)tcpinject_vars.pkt->payload)[3] = 'p';
      ((uint8_t*)tcpinject_vars.pkt->payload)[4] = 'o';
      ((uint8_t*)tcpinject_vars.pkt->payload)[5] = 'i';
      if (opentcp_send(tcpinject_vars.pkt)==E_FAIL) {
         openqueue_freePacketBuffer(tcpinject_vars.pkt);
      }
      return;
   }
}

void tcpinject_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_TCPINJECT;
   if (msg->creator!=COMPONENT_TCPINJECT) {
      openserial_printError(COMPONENT_TCPINJECT,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   opentcp_close();
   openqueue_freePacketBuffer(msg);
}

void tcpinject_receive(OpenQueueEntry_t* msg) {
}

bool tcpinject_debugPrint() {
   return FALSE;
}

//=========================== private =========================================