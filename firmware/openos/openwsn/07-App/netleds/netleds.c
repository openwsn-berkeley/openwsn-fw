#include "openwsn.h"
#include "netleds.h"
//openwsn stack
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "leds.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void netleds_init() {
}

//this is called when the corresponding button is pressed on the OpenVisualizer interface
void netleds_trigger() {
  leds_blink();
}

//I just received a request
void netleds_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_NETLEDS;
   if (msg->length==1) {
      leds_setCombination(msg->payload[0]);      // turn on a specified combination
   }
   openqueue_freePacketBuffer(msg);
}

void netleds_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_NETLEDS;
   if (msg->creator!=COMPONENT_NETLEDS) {
      openserial_printError(COMPONENT_NETLEDS,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

bool netleds_debugPrint() {
   return FALSE;
}

//=========================== private =========================================