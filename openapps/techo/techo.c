#include "opendefs.h"
#include "techo.h"
#include "openqueue.h"
#include "openserial.h"
#include "opentcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void techo_init() {
}

bool techo_shouldIlisten() {
   return TRUE;
}

void techo_receive(OpenQueueEntry_t* msg) {
   uint16_t temp_l4_destination_port;
   msg->owner                     = COMPONENT_TECHO;
   //reply with the same OpenQueueEntry_t
   msg->creator                   = COMPONENT_TECHO;
   msg->l4_protocol               = IANA_TCP;
   temp_l4_destination_port       = msg->l4_destination_port;
   msg->l4_destination_port       = msg->l4_sourcePortORicmpv6Type;
   msg->l4_sourcePortORicmpv6Type = temp_l4_destination_port;
   if (opentcp_send(msg)==E_FAIL) {
      openqueue_freePacketBuffer(msg);
   }
}

void techo_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   msg->owner = COMPONENT_TECHO;
   if (msg->creator!=COMPONENT_TECHO) {
      openserial_printError(COMPONENT_TECHO,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   //close TCP session, but keep listening
   opentcp_close();
   openqueue_freePacketBuffer(msg);
}

void techo_connectDone(owerror_t error) {
}

bool techo_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
