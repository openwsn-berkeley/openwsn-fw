/**
\brief WSN430v13b-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "msp430f5438a.h"
#include "board.h"
// bsp modules
#include "bsp.h"
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
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
   
  uint16_t ui16IntState;
   // Stop watchdog timer (prevent timeout reset)
   WDTCTL = WDTPW + WDTHOLD;
   // Disable global interrupts
   ui16IntState = __get_interrupt_state();
   __disable_interrupt();
   //  Set capacitor values for XT1, 32768 Hz
   bspMcuStartXT1();
   bspSysClockSpeedSet(BSP_SYS_CLK_25MHZ);
   __set_interrupt_state(ui16IntState);
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


ISR(USCI_A1){
    debugpins_isr_set();
    switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
    if (uart_rx_isr()==KICK_SCHEDULER) {
      __bic_SR_register_on_exit(CPUOFF);
      }
    debugpins_isr_clr();
    break;
  case 4:                                  // Vector 4 - TXIFG
    if (uart_tx_isr()==KICK_SCHEDULER) {
      __bic_SR_register_on_exit(CPUOFF);
      }
    debugpins_isr_clr();
    break;                             
  default: break;
  }
        
}

// SPI

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

// PORT1_VECTOR
ISR(PORT1){
  debugpins_isr_set();
    switch(__even_in_range(P1IV,16))
   {
   case 0:break;
   case 2:break;
   case 4:break;
   case 6:break;
   case 8:
     P1IFG &= ~(BIT3);
     P4OUT     &= ~BIT1;
     break;
   case 10:break;
   case 12:break;
   case 14:break;
   case 16:
     P1IFG &= ~(BIT7);
     P4OUT     &= ~BIT2;
     break;
   }
   debugpins_isr_clr();
}

// WDT_VECTOR

//ISR(COMPARATORA) {
//   debugpins_isr_set();
//   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
//   debugpins_isr_clr();
//}

// TIMERB0_VECTOR

ISR(TIMERB1) {
   debugpins_isr_set();
   if (radiotimer_isr()==KICK_SCHEDULER) {       // radiotimer
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

// NMI_VECTOR

