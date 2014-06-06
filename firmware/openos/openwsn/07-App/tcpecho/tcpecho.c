#include "openwsn.h"
#include "tcpecho.h"
#include "openqueue.h"
#include "openserial.h"
#include "opentcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void tcpecho_init() {
}

BOOL tcpecho_shouldIlisten() {
   return TRUE;
}

void tcpecho_receive(OpenQueueEntry_t* msg) {
   uint16_t temp_l4_destination_port;
   msg->owner   = COMPONENT_TCPECHO;
   //reply with the same OpenQueueEntry_t
   msg->creator                   = COMPONENT_TCPECHO;
   msg->l4_protocol               = IANA_TCP;
   temp_l4_destination_port       = msg->l4_destination_port;
   msg->l4_destination_port       = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type = temp_l4_destination_port;
   if (opentcp_send(msg)==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

void tcpecho_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_TCPECHO;
   if (msg->creator!=COMPONENT_TCPECHO) {
      openserial_printError(COMPONENT_TCPECHO,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   //close TCP session, but keep listening
   opentcp_close();
   openqueue_freePacketBuffer(msg);
}

void tcpecho_connectDone(owerror_t error) {
}

BOOL tcpecho_debugPrint() {
   return FALSE;
}

//=========================== private =========================================