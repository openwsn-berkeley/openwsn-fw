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
#include "timers.h"
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
   
   // initialize pins
   //-- radio RF_SLP_TR_CNTL (P4.7)
   P4OUT  &= ~0x80;                              // set low
   P4DIR  |=  0x80;                              // configure as output
   //-- radio IRQ_RF (P1.6)
   P1OUT  &= ~0x40;                              // set low
   P1DIR  &= ~0x40;                              // configure as low
   P1IES  &= ~0x40;                              // interrup when transition is low-to-high
   P1IE   |=  0x40;                              // enable interrupt
   
   // initialize bsp modules
   leds_init();
   uart_init();
   spi_init();
   timers_init();
   radio_init();
   radiotimer_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM0_bits);             // sleep, but leave ACLK on
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR (void) {
   if (P1IFG & 0x40) {
      P1IFG &= ~0x40;
      if (radio_isr()==1) {
         __bic_SR_register_on_exit(CPUOFF);
      }
   } else {
      while (1); // should never happen
   }   
}

#pragma vector = TIMERA1_VECTOR
__interrupt void TIMERA1_ISR (void) {
   if (radiotimer_isr()==1) {
      __bic_SR_register_on_exit(CPUOFF);
   }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR (void) {
   if (spi_isr()==1) {
      __bic_SR_register_on_exit(CPUOFF);
   }
}

#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void) {
   if (timer_isr_0()==1) {
      __bic_SR_register_on_exit(CPUOFF);
   }
}

#pragma vector = TIMERB1_VECTOR
__interrupt void TIMERB1_ISR (void) {
   if (timer_isr_1()==1) {
      __bic_SR_register_on_exit(CPUOFF);
   }
}

#pragma vector = USCIAB1TX_VECTOR
__interrupt void USCIAB1TX_ISR (void) {
   if (uart_isr_tx()==1) {
      __bic_SR_register_on_exit(CPUOFF);
   }
}

#pragma vector = USCIAB1RX_VECTOR
__interrupt void USCIAB1RX_ISR (void) {
   if (uart_isr_rx()==1) {
      __bic_SR_register_on_exit(CPUOFF);
   }
}