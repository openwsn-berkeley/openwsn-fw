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
//   openserial_printData((uint8_t*)(msg->payload),msg->length);
    owerror_t error;
   error = openqueue_freePacketBuffer(msg);
   if (error == E_FAIL) {
       openserial_printError(
          COMPONENT_UDPPRINT,
          ERR_UNEXPECTED_SENDDONE,
          (errorparameter_t)0,
          (errorparameter_t)0
       );
   }
}

void udpprint_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
    owerror_t error2;
   error2 = openqueue_freePacketBuffer(msg);

   if (error2 == E_FAIL) {
   openserial_printError(
      COMPONENT_UDPPRINT,
      ERR_UNEXPECTED_SENDDONE,
      (errorparameter_t)0,
      (errorparameter_t)0
   );

   }

}

bool udpprint_debugPrint() {
   return FALSE;
}

//=========================== private =========================================
