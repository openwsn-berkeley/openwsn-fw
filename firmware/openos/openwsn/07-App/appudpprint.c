#include "openwsn.h"
#include "appudpprint.h"
#include "openqueue.h"
#include "openserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void appudpprint_init() {
}

void appudpprint_sendDone(OpenQueueEntry_t* msg, error_t error) {
   openserial_printError(COMPONENT_APPUDPPRINT,ERR_UNEXPECTED_SENDDONE,
                         (errorparameter_t)0,
                         (errorparameter_t)0);
   openqueue_freePacketBuffer(msg);
}

void appudpprint_receive(OpenQueueEntry_t* msg) {
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   openqueue_freePacketBuffer(msg);
}

bool appudpprint_debugPrint() {
   return FALSE;
}

//=========================== private =========================================