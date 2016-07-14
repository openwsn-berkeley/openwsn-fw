/**
\brief WSN430v13b-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f5438a.h"
#include "board.h"
// bsp modules
#include "debugpins.h"
#include "leds.h"
//#include "uart.h"
#include "spi.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {
   
   
   //DCOCTL    |=  DCO0 | DCO1 | DCO2;             // MCLK at ~8MHz
   //BCSCTL1   |=  RSEL0 | RSEL1 | RSEL2;          // MCLK at ~8MHz
                                                 // by default, ACLK from 32kHz XTAL which is running
   // disable watchdog timer 
   WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT
   __bis_SR_register(SCG0);                  // allows to unset the DCO flag error    
 // __bis_SR_register(SCG1);                // this switches off the smclk
 
  // Initialize LFXT1
  P7SEL |= 0x03;                            // Select XT1
  UCSCTL6 &= ~(XT1OFF);                     // XT1 On
  UCSCTL6 |= XCAP_3;                        // Internal load cap
    // Loop until XT1 fault flag is cleared
  do
  {
    UCSCTL7 &= ~XT1LFOFFG;                  // Clear XT1 fault flags
  }while (UCSCTL7&XT1LFOFFG);               // Test XT1 fault flag
  
  //P6DIR |= BIT1;                            // P6.1 output
  P11DIR |= 0x07;                           // ACLK, MCLK, SMCLK set out to pins
  P11SEL |= 0x07;                           // P11.0,1,2 for debugging purposes.
  //P4DIR |= BIT0 | BIT1 | BIT2 | BIT3 ;
  // setup clock speed
  UCSCTL0 = /*DCO3 |*/ DCO1;              //  ~26.5 MHz , DCO3+DCO1 and DCORSEL_7
  UCSCTL1 = /*DCORSEL_7 |*/ DISMOD | DCORSEL_1 ;     
  UCSCTL2 = 0;
  UCSCTL3 = 0;
  UCSCTL4 =  SELM_3 | SELS_3 ;
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG );
                                       // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  }while (SFRIFG1&OFIFG); 
   
   // initialize pins
   //P4DIR     |=  0x20;                           // [P4.5] radio VREG:  output
   //P4DIR     |=  0x40;                           // [P4.6] radio reset: output
   
   // initialize bsp modules
   debugpins_init();
   leds_init();
  // uart_init();
  // spi_init();
   bsp_timer_init();
  // radio_init();
  // radiotimer_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
}

void board_sleep() {
   __bis_SR_register(GIE+LPM0_bits);             // sleep, but leave ACLK on
}

void board_reset() {
    WDTCTL = (WDTPW+0x1200) + WDTHOLD; // writing a wrong watchdog password to causes handler to reset
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

// DACDMA_VECTOR

// PORT2_VECTOR

//ISR(USART1TX){
//   debugpins_isr_set();
//   if (uart_tx_isr()==KICK_SCHEDULER) {          // UART; TX
//      __bic_SR_register_on_exit(CPUOFF);
//   }
//   debugpins_isr_clr();
//}

//ISR(USART1RX) {
//   debugpins_isr_set();
//   if (uart_rx_isr()==KICK_SCHEDULER) {          // UART: RX
//      __bic_SR_register_on_exit(CPUOFF);
//   }
//   debugpins_isr_clr();
//}

// PORT1_VECTOR

// TIMERA1_VECTOR

ISR(TIMER0_A0) {
   debugpins_isr_set();
   if (bsp_timer_isr()==KICK_SCHEDULER) {        // timer: 0
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

// ADC12_VECTOR

// USART0TX_VECTOR

//ISR(USART0RX) {
//   debugpins_isr_set();
//   if (spi_isr()==KICK_SCHEDULER) {              // SPI
//      __bic_SR_register_on_exit(CPUOFF);
//   }
//   debugpins_isr_clr();
//}

// WDT_VECTOR

//ISR(COMPARATORA) {
//   debugpins_isr_set();
//   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
//   debugpins_isr_clr();
//}

//ISR(TIMERB1) {
//   debugpins_isr_set();
//   if (radiotimer_isr()==KICK_SCHEDULER) {       // radiotimer
//      __bic_SR_register_on_exit(CPUOFF);
//   }
//   debugpins_isr_clr();
//}

// TIMERB0_VECTOR

// NMI_VECTOR

