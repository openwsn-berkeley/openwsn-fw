/**
\brief Z1-specific definition of the "board" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2013.
*/

#include "msp430x26x.h"
#include "board.h"
#include "debugpins.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"
#include "eui64.h"

//#define ISR_BUTTON 1

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {
   
   // disable watchdog timer
   WDTCTL  = WDTPW + WDTHOLD;
   
   // setup clock speed --seems that does not work
   //BCSCTL1 = CALBC1_16MHZ;                       // MCLK at ~16MHz
   //DCOCTL  = CALDCO_16MHZ;                       // MCLK at ~16MHz
   
   if(CALBC1_8MHZ != 0xFF) {
     DCOCTL   = 0x00;
     BCSCTL1  = CALBC1_8MHZ;                     //Set DCO to 8MHz
     DCOCTL   = CALDCO_8MHZ;    
   } else { //start using reasonable values at 8 Mhz
     DCOCTL   = 0x00;
     BCSCTL1  = 0x8D;
     DCOCTL   = 0x88;
   }
   
   // enable flash access violation NMIs
   IE1 |= ACCVIE;
   
    // initialize pins
   P4DIR     |=  0x20;                           // [P4.5] radio VREG:  output
   P4DIR     |=  0x40;                           // [P4.6] radio reset: output
   
   // initialize bsp modules
   debugpins_init();
   leds_init();
   uart_init();
   spi_init();
   bsp_timer_init();
   radio_init();
   radiotimer_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

void board_reset() {
   WDTCTL = (WDTPW+0x1200) + WDTHOLD; // writing a wrong watchdog password to causes handler to reset
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

ISR(DAC12){
   
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(DMA){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(USCIAB1TX){
   debugpins_isr_set();
   if ( ((UC1IFG & UCB1TXIFG) && (UC1IE & UCB1TXIE)) ||
        ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ) {
         isr_i2c_tx(1);                             // I2C: TX
   }
   debugpins_isr_clr();
}

ISR(USCIAB1RX){
   debugpins_isr_set();
   if ( ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ||
         (UCB1STAT & UCNACKIFG) ) {
          isr_i2c_rx(1);                             // I2C: RX, bus 1
   }
   debugpins_isr_clr();
}

ISR(ADC12){
   debugpins_isr_set();
   ADC12IFG &= ~0x1F;
   __bic_SR_register_on_exit(CPUOFF);
   debugpins_isr_clr();
}

ISR(USCIAB0TX){
   debugpins_isr_set();
   if ( (UC0IFG & UCA0TXIFG) && (UC0IE & UCA0TXIE) ){
      if (uart_tx_isr()==KICK_SCHEDULER) {       // UART: TX
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   debugpins_isr_clr();
}

ISR(USCIAB0RX){
   debugpins_isr_set();
   if ( (IFG2 & UCB0RXIFG) && (IE2 & UCB0RXIE) ) {
      if (spi_isr()==KICK_SCHEDULER) {           // SPI
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   if ( (UC0IFG & UCA0RXIFG) && (UC0IE & UCA0RXIE) ){
      if (uart_rx_isr()==KICK_SCHEDULER) {       // UART: RX
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   
   debugpins_isr_clr();
}

ISR(TIMERA1){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(WDT){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(COMPARATORA){
   debugpins_isr_set();
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   debugpins_isr_clr();
}

ISR(TIMERB1){
   debugpins_isr_set();
   if (radiotimer_isr()==KICK_SCHEDULER) {       // radiotimer
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(TIMERA0){
   debugpins_isr_set();
   if (bsp_timer_isr()==KICK_SCHEDULER) {        // timer: 0
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(TIMERB0){
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(NMI){
   debugpins_isr_set();
   debugpins_frame_set();
   while(1); // should never happen
}
