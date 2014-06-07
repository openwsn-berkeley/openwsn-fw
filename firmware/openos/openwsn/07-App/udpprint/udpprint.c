#include "openwsn.h"
#include "udpprint.h"
#include "openqueue.h"
#include "openserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpprint_init() {
}

void udpprint_receive(OpenQueueEntry_t* msg) {
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   openqueue_freePacketBuffer(msg);
}

void udpprint_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
   openserial_printError(
      COMPONENT_UDPPRINT,
      ERR_UNEXPECTED_SENDDONE,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
   openqueue_freePacketBuffer(msg);
}

bool udpprint_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
