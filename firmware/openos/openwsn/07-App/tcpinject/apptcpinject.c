#include "openwsn.h"
#include "apptcpinject.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "opentcp.h"
#include "openqueue.h"

//=========================== variables =======================================

typedef struct {
   OpenQueueEntry_t*    pkt;
   bool                 sending;
   open_addr_t          hisAddress;
   uint16_t             hisPort;
} apptcpinject_vars_t;

apptcpinject_vars_t apptcpinject_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void apptcpinject_init() {
}

bool apptcpinject_shouldIlisten() {
   return FALSE;
}

void apptcpinject_trigger() {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[18];
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_APPTCPINJECT,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   apptcpinject_vars.hisAddress.type = ADDR_128B;
   memcpy(&(apptcpinject_vars.hisAddress.addr_128b[0]),&(input_buffer[0]),16);
   apptcpinject_vars.hisPort = packetfunctions_ntohs(&(input_buffer[16]));
   //connect
   opentcp_connect(&apptcpinject_vars.hisAddress,apptcpinject_vars.hisPort,WKP_TCP_INJECT);
}

void apptcpinject_connectDone(error_t error) {
   if (error==E_SUCCESS) {
      apptcpinject_vars.pkt = openqueue_getFreePacketBuffer();
      if (apptcpinject_vars.pkt==NULL) {
         openserial_printError(COMPONENT_APPTCPINJECT,ERR_NO_FREE_PACKET_BUFFER,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
         return;
      }
      apptcpinject_vars.pkt->creator                      = COMPONENT_APPTCPINJECT;
      apptcpinject_vars.pkt->owner                        = COMPONENT_APPTCPINJECT;
      apptcpinject_vars.pkt->l4_protocol                  = IANA_UDP;
      apptcpinject_vars.pkt->l4_sourcePortORicmpv6Type    = WKP_TCP_INJECT;
      apptcpinject_vars.pkt->l4_destination_port          = apptcpinject_vars.hisPort;
      memcpy(&(apptcpinject_vars.pkt->l3_destinationORsource),&apptcpinject_vars.hisAddress,sizeof(open_addr_t));
      packetfunctions_reserveHeaderSize(apptcpinject_vars.pkt,6);
      ((uint8_t*)apptcpinject_vars.pkt->payload)[0] = 'p';
      ((uint8_t*)apptcpinject_vars.pkt->payload)[1] = 'o';
      ((uint8_t*)apptcpinject_vars.pkt->payload)[2] = 'i';
      ((uint8_t*)apptcpinject_vars.pkt->payload)[3] = 'p';
      ((uint8_t*)apptcpinject_vars.pkt->payload)[4] = 'o';
      ((uint8_t*)apptcpinject_vars.pkt->payload)[5] = 'i';
      if (opentcp_send(apptcpinject_vars.pkt)==E_FAIL) {
         openqueue_freePacketBuffer(apptcpinject_vars.pkt);
      }
      return;
   }
}

void apptcpinject_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPTCPINJECT;
   if (msg->creator!=COMPONENT_APPTCPINJECT) {
      openserial_printError(COMPONENT_APPTCPINJECT,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   opentcp_close();
   openqueue_freePacketBuffer(msg);
}

void apptcpinject_receive(OpenQueueEntry_t* msg) {
}

bool apptcpinject_debugPrint() {
   return FALSE;
}

//=========================== private =========================================