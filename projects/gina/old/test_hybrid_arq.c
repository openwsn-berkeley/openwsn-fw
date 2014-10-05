/**
\brief This is a standalone test program for testing the hybrid ARQ concept

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2010
*/

//board
#include "gina.h"
//drivers
#include "leds.h"
#include "button.h"
//openwsn
#include "opendefs.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "radio.h"

OpenQueueEntry_t* testRadioPacketToSend;

int main(void) {
   //configuring
   P1OUT |=  0x04;                               // set P1.2 for debug
   
   gina_init();
   scheduler_init();
   button_init();
   openwsn_init();
   openqueue_init();
   P1OUT &= ~0x04;                               // clear P1.2 for debug
   
   radio_init();

   radio_rxOn(DEFAULTCHANNEL);

   scheduler_start();
}

void isr_button() {
   //prepare packet
   testRadioPacketToSend = openqueue_getFreePacketBuffer();
   //l1
   testRadioPacketToSend->l1_channel  = DEFAULTCHANNEL;
   //payload
   packetfunctions_reserveHeaderSize(testRadioPacketToSend,5);
   testRadioPacketToSend->payload[0]  =  0x01;
   testRadioPacketToSend->payload[1]  =  0x02;
   testRadioPacketToSend->payload[2]  =  0x03;
   testRadioPacketToSend->payload[3]  =  0x04;
   testRadioPacketToSend->payload[4]  =  0x05;
   packetfunctions_reserveFooterSize(testRadioPacketToSend,2); //space for radio to fill in CRC
   //send packet
   radio_send(testRadioPacketToSend);
   //debug
   P1OUT ^= 0x02;                                // toggle P1.1 (for debug)
   leds_circular_shift();                        // circular-shift LEDs (for debug)
}

void stupidmac_sendDone(OpenQueueEntry_t* pkt, error_t error) {
   openqueue_freePacketBuffer(pkt);
}

void radio_packet_received(OpenQueueEntry_t* packetReceived) {
   openqueue_freePacketBuffer(packetReceived);
   P1OUT ^= 0x02;                                // toggle P1.1 for debug
   leds_increment();                             // increment LEDs for debug
}
