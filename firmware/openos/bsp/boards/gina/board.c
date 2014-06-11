/**
\brief GINA-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
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

// sensors
//#include "gyro.h"
//#include "large_range_accel.h"
//#include "magnetometer.h"
//#include "sensitive_accel_temperature.h"
//#include "ADC_Channel.h"

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
   uint8_t eui[8];
   
   // disable watchdog timer
   WDTCTL  = WDTPW + WDTHOLD;
   
   // setup clock speed
   BCSCTL1 = CALBC1_16MHZ;                       // MCLK at ~16MHz
   DCOCTL  = CALDCO_16MHZ;                       // MCLK at ~16MHz
   
   // enable flash access violation NMIs
   IE1 |= ACCVIE;
   
   // initialize pins
   //-- radio RF_SLP_TR_CNTL (P4.7)
   P4OUT  &= ~0x80;                              // set low
   P4DIR  |=  0x80;                              // configure as output
   //-- radio IRQ_RF (P1.6)
   P1OUT  &= ~0x40;                              // set low
   P1DIR  &= ~0x40;                              // configure as low
   P1IES  &= ~0x40;                              // interrup when transition is low-to-high
   P1IE   |=  0x40;                              // enable interrupt
   
#ifdef ISR_BUTTON
   //p2.7 button
   P2DIR &= ~0x80; // Set P2.7 to output direction
   P2IE |= 0x80; // P2.7 interrupt enabled
   P2IES |= 0x80; // P2.7 Hi/lo edge 
   P2IFG &= ~0x80; // P2.7 IFG cleared
#endif
   
   // initialize bsp modules
   debugpins_init();
   leds_init();
   uart_init();
   spi_init();
   i2c_init();
   bsp_timer_init();
   radio_init();
   radiotimer_init();
   //ADC_init();
   
   // enable interrupts
   __bis_SR_register(GIE);
   
   //turn sensors off, if this is a gina (not a basestation)
   eui64_get(eui);
   if (eui[3]==0x09) {
      // first initialize them
//      gyro_init();
//      large_range_accel_init();
//      magnetometer_init();
//      sensitive_accel_temperature_init();
//     
//      // then turn them off
//      gyro_disable();
//      large_range_accel_disable();
//      magnetometer_disable();
//      sensitive_accel_temperature_disable();
   }
}

void board_sleep() {
   __bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

void board_reset() {
   WDTCTL = (WDTPW+0x1200) + WDTHOLD; // writing a wrong watchdog password to causes handler to reset
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

ISR(DAC12) {
   CAPTURE_TIME();
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(DMA) {
   CAPTURE_TIME();
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(USCIAB1TX) {
   CAPTURE_TIME();
   debugpins_isr_set();
   if ( ((UC1IFG & UCB1TXIFG) && (UC1IE & UCB1TXIE)) ||
        ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ) {
      isr_i2c_tx(1);                             // I2C: TX
   }
   if ( (UC1IFG & UCA1TXIFG) && (UC1IE & UCA1TXIE) ){
      if (uart_tx_isr()==KICK_SCHEDULER) {       // UART: TX
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   debugpins_isr_clr();
}

ISR(USCIAB1RX) {
   CAPTURE_TIME();
   debugpins_isr_set();
   if ( ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ||
         (UCB1STAT & UCNACKIFG) ) {
      isr_i2c_rx(1);                             // I2C: RX, bus 1
   }
   if ( (UC1IFG & UCA1RXIFG) && (UC1IE & UCA1RXIE) ){
      if (uart_rx_isr()==KICK_SCHEDULER) {       // UART: RX
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   debugpins_isr_clr();
}

ISR(PORT1) {
   CAPTURE_TIME();
   debugpins_isr_set();
   if (P1IFG & 0x40) {
      P1IFG &= ~0x40;
      if (radio_isr()==KICK_SCHEDULER) {         // radio:  SFD pin [P1.6]
         __bic_SR_register_on_exit(CPUOFF);
      }
   } else {
      while (1); // should never happen
   }
   debugpins_isr_clr();
}

ISR(PORT2) {
   CAPTURE_TIME();
   debugpins_isr_set();
#ifdef ISR_BUTTON
   if ((P2IFG & 0x80)!=0) {                      // button: [P2.7]
      P2IFG &= ~0x80;
      scheduler_push_task(ID_ISR_BUTTON);
      __bic_SR_register_on_exit(CPUOFF);
   } else {
      while (1); // should never happen
   }
   debugpins_isr_clr();
#else
   while(1); // should never happen
#endif
}

ISR(ADC12) {
   CAPTURE_TIME();
   debugpins_isr_set();
   ADC12IFG &= ~0x1F;
   __bic_SR_register_on_exit(CPUOFF);
   debugpins_isr_clr();
}

ISR(USCIAB0TX) {
   CAPTURE_TIME();
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(USCIAB0RX) {
   CAPTURE_TIME();
   debugpins_isr_set();
   if ( (IFG2 & UCA0RXIFG) && (IE2 & UCA0RXIE) ) {
      if (spi_isr()==KICK_SCHEDULER) {           // SPI
         __bic_SR_register_on_exit(CPUOFF);
      }
   }
   if ( ((IFG2 & UCB0RXIFG) && (IE2 & UCB0RXIE)) ||
        (UCB0STAT & UCNACKIFG) ) {
      isr_i2c_rx(0);                             // I2C: RX, bus 0
   }
   debugpins_isr_clr();
}

ISR(TIMERA1) {
   CAPTURE_TIME();
   debugpins_isr_set();
   
   if (radiotimer_isr()==KICK_SCHEDULER) {       // radiotimer
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(TIMERA0) {
   CAPTURE_TIME();
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(WDT) {
   CAPTURE_TIME();
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(COMPARATORA) {
   CAPTURE_TIME();
   debugpins_isr_set();
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   debugpins_isr_clr();
}

ISR(TIMERB1) {
   CAPTURE_TIME();
   debugpins_isr_set();
   while(1); // should never happen
}

ISR(TIMERB0) {
   CAPTURE_TIME();
   debugpins_isr_set();
   if (bsp_timer_isr()==KICK_SCHEDULER) {        // timer: 0
      __bic_SR_register_on_exit(CPUOFF);
   }
   debugpins_isr_clr();
}

ISR(NMI) {
   CAPTURE_TIME();
   debugpins_isr_set();
   debugpins_frame_set();
   while(1); // should never happen
}
