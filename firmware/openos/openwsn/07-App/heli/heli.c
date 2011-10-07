#include "openwsn.h"
#include "heli.h"
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

void heli_setmotor(uint8_t which, uint16_t value);
uint16_t heli_threshold(uint16_t value);

//=========================== public ==========================================

void heli_init() {
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
void heli_trigger() {
}

uint16_t heli_threshold(uint16_t value) {
   /*if (value < MOTORMIN) { //causes warning because set to zero
      return MOTORMIN;
   }*/
   if (value > MOTORMAX) {
      return MOTORMAX;
   }
   return value;
}

//I just received a request
void heli_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_HELI;
   if (msg->length==4) {
      heli_setmotor(1,heli_threshold(packetfunctions_ntohs(&(msg->payload[0]))));
      heli_setmotor(2,heli_threshold(packetfunctions_ntohs(&(msg->payload[2]))));
   }
   openqueue_freePacketBuffer(msg);
}

//I just sent a IMU packet, check I need to resend one
void heli_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_HELI;
   if (msg->creator!=COMPONENT_HELI) {
      openserial_printError(COMPONENT_HELI,ERR_SENDDONE_FOR_MSG_I_DID_NOT_SEND,0,0);
   }
   openqueue_freePacketBuffer(msg);
}

bool heli_debugPrint() {
   return FALSE;
}

//=========================== private =========================================

void heli_setmotor(uint8_t which, uint16_t value) {
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
