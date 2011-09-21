#include "openwsn.h"
#include "appudpheli.h"
//openwsn stack
#include "udp.h"
#include "openqueue.h"
#include "openserial.h"
#include "packetfunctions.h"

#define MOTORPERIOD 100
#define MOTORMAX    MOTORPERIOD
#define MOTORMIN    0

//=========================== variables =======================================

//=========================== prototypes ======================================

void appudpheli_setmotor(uint8_t which, uint16_t value);
uint16_t appudpheli_threshold(uint16_t value);

//=========================== public ==========================================

void appudpheli_init() {
   P1DIR   |= 0x0C;                              // P1.2,3 output
   P1SEL   |= 0x0C;                              // P1.2,3 in PWM mode
   TACTL    = TBSSEL_1 + ID_3 + MC_1;            // ACLK, count up to TACCR0
   TACCR0   = MOTORPERIOD;                       // ~320 Hz frequency
   TACCTL1  = OUTMOD_7;
   TACCR1   = MOTORMIN;
   TACCTL2  = OUTMOD_7;
   TACCR2   = MOTORMIN;
}

//this is called when the corresponding button is pressed on the OpenVisualizer interface
void appudpheli_trigger() {
}

uint16_t appudpheli_threshold(uint16_t value) {
   /*if (value < MOTORMIN) { //causes warning because set to zero
      return MOTORMIN;
   }*/
   if (value > MOTORMAX) {
      return MOTORMAX;
   }
   return value;
}

//I just received a request
void appudpheli_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_APPUDPHELI;
   if (msg->length==4) {
      appudpheli_setmotor(1,appudpheli_threshold(packetfunctions_ntohs(&(msg->payload[0]))));
      appudpheli_setmotor(2,appudpheli_threshold(packetfunctions_ntohs(&(msg->payload[2]))));
   }
   openqueue_freePacketBuffer(msg);
}

//I just sent a IMU packet, check I need to resend one
void appudpheli_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_APPUDPHELI;
   if (msg->creator!=COMPONENT_APPUDPHELI) {
      openserial_printError(COMPONENT_APPUDPHELI,ERR_SENDDONE_FOR_MSG_I_DID_NOT_SEND,0,0);
   }
   openqueue_freePacketBuffer(msg);
}

bool appudpheli_debugPrint() {
   return FALSE;
}

//=========================== private =========================================

void appudpheli_setmotor(uint8_t which, uint16_t value) {
   /*if (value < MOTORMIN) {
      value = MOTORMIN;
   }*/
   if (value > MOTORMAX) {
      value = MOTORMAX;
   }
   switch (which) {
      case 1:
         TACCR1 = value;
         break;
      case 2:
         TACCR2 = value;
         break;
   }
} 
