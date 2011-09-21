#include "openwsn.h"
#include "apptcpecho.h"
#include "openqueue.h"
#include "openserial.h"
#include "tcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void apptcpecho_init() {
}

bool apptcpecho_shouldIlisten() {
   return TRUE;
}

void apptcpecho_receive(OpenQueueEntry_t* msg) {
   uint16_t temp_l4_destination_port;
   msg->owner   = COMPONENT_APPTCPECHO;
   //reply with the same OpenQueueEntry_t
   msg->creator                   = COMPONENT_APPTCPECHO;
   msg->l4_protocol               = IANA_TCP;
   temp_l4_destination_port       = msg->l4_destination_port;
   msg->l4_destination_port       = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type = temp_l4_destination_port;
   if (tcp_send(msg)==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

void apptcpecho_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPTCPECHO;
   if (msg->creator!=COMPONENT_APPTCPECHO) {
      openserial_printError(COMPONENT_APPTCPECHO,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   //close TCP session, but keep listening
   tcp_close();
   openqueue_freePacketBuffer(msg);
}

void apptcpecho_connectDone() {
}

bool apptcpecho_debugPrint() {
   return FALSE;
}

//=========================== private =========================================