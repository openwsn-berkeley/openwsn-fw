/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:13.894224.
*/
#include "openwsn_obj.h"
#include "udpprint_obj.h"
#include "openqueue_obj.h"
#include "openserial_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void udpprint_init(OpenMote* self) {
}

void udpprint_receive(OpenMote* self, OpenQueueEntry_t* msg) {
 openserial_printData(self, (uint8_t*)(msg->payload),msg->length);
 openqueue_freePacketBuffer(self, msg);
}

void udpprint_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
 openserial_printError(self, 
      COMPONENT_UDPPRINT,
      ERR_UNEXPECTED_SENDDONE,
      (errorparameter_t)0,
      (errorparameter_t)0
   );
 openqueue_freePacketBuffer(self, msg);
}

bool udpprint_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================
