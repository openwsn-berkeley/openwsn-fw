/**
\brief GINA-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430x26x.h"
#include "board.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "radiotimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init() {
   // disable watchdog timer
   WDTCTL  = WDTPW + WDTHOLD;
   
   // setup clock speed
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at ~16MHz
   DCOCTL  = CALDCO_16MHZ;                       // MCLK at ~16MHz
   
   // initialize bsp modules
   leds_init();
   uart_init();
   spi_init();
   radio_init();
   radiotimer_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

#pragma vector = RESERVED0_VECTOR
__interrupt void RESERVED0_ISR (void) {
   while (1);
}

#pragma vector = RESERVED1_VECTOR
__interrupt void RESERVED1_ISR (void) {
   while (1);
}

#pragma vector = RESERVED2_VECTOR
__interrupt void RESERVED2_ISR (void) {
   while (1);
}

#pragma vector = RESERVED3_VECTOR
__interrupt void RESERVED3_ISR (void) {
   while (1);
}

#pragma vector = RESERVED4_VECTOR
__interrupt void RESERVED4_ISR (void) {
   while (1);
}

#pragma vector = RESERVED5_VECTOR
__interrupt void RESERVED5_ISR (void) {
   while (1);
}

#pragma vector = RESERVED6_VECTOR
__interrupt void RESERVED6_ISR (void) {
   while (1);
}

#pragma vector = RESERVED7_VECTOR
__interrupt void RESERVED7_ISR (void) {
   while (1);
}

#pragma vector = RESERVED8_VECTOR
__interrupt void RESERVED8_ISR (void) {
   while (1);
}

#pragma vector = RESERVED9_VECTOR
__interrupt void RESERVED9_ISR (void) {
   while (1);
}

#pragma vector = RESERVED10_VECTOR
__interrupt void RESERVED10_ISR (void) {
   while (1);
}

#pragma vector = RESERVED11_VECTOR
__interrupt void RESERVED11_ISR (void) {
   while (1);
}

#pragma vector = RESERVED12_VECTOR
__interrupt void RESERVED12_ISR (void) {
   while (1);
}

#pragma vector = RESERVED13_VECTOR
__interrupt void RESERVED13_ISR (void) {
   while (1);
}

#pragma vector = DAC12_VECTOR
__interrupt void DAC12_ISR (void) {
   while (1);
}

#pragma vector = DMA_VECTOR
__interrupt void DMA_ISR (void) {
   while (1);
}

/* // defined in bsp module "uart"
#pragma vector = USCIAB1TX_VECTOR
__interrupt void USCIAB1TX_ISR (void) {
   while (1);
}
*/

/* // defined in bsp module "uart"
#pragma vector = USCIAB1RX_VECTOR
__interrupt void USCIAB1RX_ISR (void) {
   while (1);
}
*/

/* // defined in bsp module "radio"
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR (void) {
   while (1);
}
*/

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR (void) {
   while (1);
}

#pragma vector = RESERVED20_VECTOR
__interrupt void RESERVED20_ISR (void) {
   while (1);
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR (void) {
   while (1);
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR (void) {
   while (1);
}

/* // defined in bsp module "spi"
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR (void) {
   while (1);
}
*/

/* // defined in bsp module "radiotimer"
#pragma vector = TIMERA1_VECTOR
__interrupt void TIMERA1_ISR (void) {
   while (1);
}
*/

#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
   while (1);
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR (void) {
   while (1);
}

#pragma vector = COMPARATORA_VECTOR
__interrupt void COMPARATORA_ISR (void) {
   while (1);
}

#pragma vector = TIMERB1_VECTOR
__interrupt void TIMERB1_ISR (void) {
   while (1);
}

#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void) {
   while (1);
}

#pragma vector = NMI_VECTOR
__interrupt void NMI_ISR (void) {
   while (1);
}

/* // can not be set?
#pragma vector = RESET_VECTOR
__interrupt void RESET_ISR (void) {
   while (1);
}
*/