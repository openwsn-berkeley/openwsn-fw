/**
\brief This is a standalone test program for the radio on the GINA2.2b/c and GINA 
       basestation boards.

Download the program to a platform, run it:
  - when you press the AUX button, a packet gets sent which contains 5 payload
    bytes: 0x0102030405. The LEDs should also circular-shift;
  - when the board receives a packet, the LEDs circular-shift.

The SPI interface to the radio is:
   - P3.0:  A0_RF_SCLK
   - P3.4:  A0_RF_SIMO
   - P3.5:  A0_RF_SOMI
   - P4.0: /EN_RF, chip select (active low)

Digital pins to the radio are:
   - P4.7: SLP_TR_CNTL, when high, forces radio to sleep mode (not used)

Interrupt pins from radio:
   - P1.6: IRQ_RF, programmable interrupt

Other "debug" pins are:
   - P1.1: toggles at every sent packet
   - P1.2: high during configuration
   - P2.7: button
   - P4.6: mimicks the /EN_RF (P4.0) pin for scope-based debugging
   - P2.0: red LED
   - P2.1: green LED
   - P2.2: blue LED
   - P2.3: red LED

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
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
   //openwsn_init();
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
