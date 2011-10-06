#include "openwsn.h"
#include "appudpleds.h"
//openwsn stack
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"
#include "leds.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void appudpleds_init() {
}

//this is called when the corresponding button is pressed on the OpenVisualizer interface
void appudpleds_trigger() {
  leds_blink();
}

//I just received a request
void appudpleds_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_APPUDPLEDS;
   if (msg->length==1) {
      leds_setCombination(msg->payload[0]);      // turn on a specified combination
   }
   openqueue_freePacketBuffer(msg);
}

void appudpleds_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPUDPLEDS;
   if (msg->creator!=COMPONENT_APPUDPLEDS) {
      openserial_printError(COMPONENT_APPUDPLEDS,ERR_UNEXPECTED_SENDDONE,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
   }
   openqueue_freePacketBuffer(msg);
}

bool appudpleds_debugPrint() {
   return FALSE;
}

//=========================== private =========================================