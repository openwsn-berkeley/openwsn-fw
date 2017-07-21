/**
\brief TelosB-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f1611.h"
#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "sctimer.h"
#include "radio.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

// these are part of the TelosB port only, so not present in debugpins.h, only
// in debugpins.c. We therefore need to extern them.
extern void debugpins_isruarttx_toggle(void);
extern void debugpins_isruarttx_clr(void);
extern void debugpins_isruarttx_set(void);
extern void debugpins_isruartrx_toggle(void);
extern void debugpins_isruartrx_clr(void);
extern void debugpins_isruartrx_set(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {
   // disable watchdog timer
   WDTCTL     =  WDTPW + WDTHOLD;
   
   // setup clock speed
   DCOCTL    |=  DCO0 | DCO1 | DCO2;             // MCLK at ~8MHz
   BCSCTL1   |=  RSEL0 | RSEL1 | RSEL2;          // MCLK at ~8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running
   
   // initialize pins
   P4DIR     |=  0x20;                           // [P4.5] radio VREG:  output
   P4DIR     |=  0x40;                           // [P4.6] radio reset: output
   
   // initialize bsp modules
   debugpins_init();
   leds_init();
   uart_init();
   spi_init();
   radio_init();
   sctimer_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM0_bits);             // sleep, but leave ACLK on
}

void board_reset() {
   WDTCTL = (WDTPW+0x1200) + WDTHOLD; // writing a wrong watchdog password to causes handler to reset
}

// during startup process before executing main function, 
// all variables need to be initialized, which may take long time
// and watchdog may be triggered during this period. 
// Using __low_level_init to disable the watchdog to avoid this situation.
int __low_level_init(void)
{
  // stop WDT
  WDTCTL = WDTPW + WDTHOLD;
 
  // Perform data segment initialization
  return 1;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

// DACDMA_VECTOR

// PORT2_VECTOR

ISR(USART1TX){
   debugpins_isruarttx_set();
   if (uart_tx_isr()==KICK_SCHEDULER) {          // UART; TX
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isruarttx_clr();
}

ISR(USART1RX) {
   debugpins_isruartrx_set();
   if (uart_rx_isr()==KICK_SCHEDULER) {          // UART: RX
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isruartrx_clr();
}

// PORT1_VECTOR

// TIMERA1_VECTOR

// ADC12_VECTOR

// USART0TX_VECTOR

ISR(USART0RX) {
   debugpins_isr_set();
   if (spi_isr()==KICK_SCHEDULER) {              // SPI
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

// WDT_VECTOR

ISR(COMPARATORA) {
   debugpins_isr_set();
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   debugpins_isr_clr();
}

ISR(TIMERB1) {
   debugpins_isr_set();
   if (sctimer_isr()==KICK_SCHEDULER) {          // sctimer
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

// TIMERB0_VECTOR

// NMI_VECTOR

