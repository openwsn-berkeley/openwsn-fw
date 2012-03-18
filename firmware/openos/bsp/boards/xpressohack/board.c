/**
\brief LPC17XX-specific definition of the "board" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#include "LPC17xx.h"
#include "board.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "bsp_timers.h"
#include "clkpwr.h"
#include "debugpins.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

extern void EINT3_IRQHandler(void);

//=========================== public ==========================================

void board_init() {

   //===== radio pins
   // [P2.8] SLP_TR
   LPC_PINCON->PINSEL4      &= ~0x3<<16;    // GPIO mode
   LPC_GPIO2->FIODIR        |=  1<<8;       // set as output
   LPC_GPIO2->FIOCLR        |=  1<<8;       // pull low
   // [P2.4] RSTn
   LPC_PINCON->PINSEL4      &= ~0x3<<8;     // GPIO mode
   LPC_GPIO2->FIODIR        |=  1<<4;       // set as output
   // [P2.5] ISR
   LPC_PINCON->PINSEL4      &= ~0x3<<10;    // GPIO mode
   LPC_GPIO2->FIODIR        &= ~1<<5;       // set as input
   LPC_GPIOINT->IO2IntClr   |=  1<<5;       // clear possible pending interrupt
   LPC_GPIOINT->IO2IntEnR   |=  1<<5;       // enable interrupt, rising edge

   // enable interrupts
   NVIC_EnableIRQ(EINT3_IRQn);              // GPIOs

   // initialize bsp modules
   leds_init();
   debugpins_init();
   timers_init();
   spi_init();
   //uart_init();
   radio_init();
   //radiotimer_init();
}

void board_sleep() {
   CLKPWR_Sleep();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

// GPIOs
// note: all GPIO interrupts, both port 0 and 2, trigger this same vector
void EINT3_IRQHandler(void) {
   if ((LPC_GPIOINT->IO2IntStatR) & (1<<5)) {
      LPC_GPIOINT->IO2IntClr = (1<<5);
      radio_isr();
   }
}
