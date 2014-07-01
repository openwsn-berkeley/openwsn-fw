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
#include "bsp_timer.h"
#include "clkpwr.h"
#include "debugpins.h"
#include "radiotimer.h"


//=========================== variables =======================================

//=========================== prototypes ======================================

extern void EINT3_IRQHandler(void);

//=========================== public ==========================================

extern int mote_main();

int main() {
   return mote_main();
}

void board_init() {

	//===== radio pins
	// OpenMote SLP_TR [P1.22]
#ifdef OPENMOTE
	LPC_PINCON->PINSEL3      &= ~(0x3<<12);    // GPIO mode
	LPC_GPIO1->FIODIR        |=  1<<22;       // set as output
	LPC_GPIO1->FIOCLR        |=  1<<22;       // pull low
	// [P0.22] ISR
	LPC_PINCON->PINSEL1      &= ~(0x3<<12);    // GPIO mode
	LPC_GPIO0->FIODIR        &= ~(RADIO_ISR_MASK);       // set as input
	LPC_GPIOINT->IO0IntClr   |=  RADIO_ISR_MASK;       // clear possible pending interrupt
	LPC_GPIOINT->IO0IntEnR   |=  RADIO_ISR_MASK;       // enable interrupt, rising edge
#endif
	//LPCXpresso is [P2.8]
#ifdef LPCXPRESSO1769
	LPC_PINCON->PINSEL4      &= ~(0x3<<16);    // GPIO mode
	LPC_GPIO2->FIODIR        |=  1<<8;       // set as output
	LPC_GPIO2->FIOCLR        |=  1<<8;       // pull low
	// [P0.21] ISR as 0.22 is the led
	LPC_PINCON->PINSEL1      &= ~(0x3<<10);    // GPIO mode
	LPC_GPIO0->FIODIR        &= ~(RADIO_ISR_MASK);       // set as input
	LPC_GPIOINT->IO0IntClr   |=  RADIO_ISR_MASK;       // clear possible pending interrupt
	LPC_GPIOINT->IO0IntEnR   |=  RADIO_ISR_MASK;       // enable interrupt, rising edge

	// [P0.15] is a gpio binded to [P0.23] that is used to toggle capture signal. normally the radio isr line
	//should be bounded to P0.23 to toggle capture pin. To solve that problem in lpcxpresso hack, just put a wire between
	// P0.23 and P0.15 and us P0.15 as gpio to start capture. We can use RADIO_ISR pin as alternative but then some code needs to be changed.

	LPC_PINCON->PINSEL0     &= ~0x3<<30;          // P0.15 as gpio
	LPC_GPIO0->FIODIR        |= CAPTURE_PIN_MASK;             // as output
	LPC_GPIO0->FIOCLR        |=CAPTURE_PIN_MASK; //set as low
#endif

	// [P0.17] RSTn
	LPC_PINCON->PINSEL1      &= ~(0x3<<2);     // GPIO mode
	LPC_GPIO0->FIODIR        |=  1<<17;       // set as output
	LPC_GPIO0->FIOSET        |=  1<<17;       // set as high


	// enable interrupts
	NVIC_EnableIRQ(EINT3_IRQn);              // GPIOs -- check that..

	debugpins_init();
	leds_init();
	uart_init();

	spi_init();
	//   i2c_init();
	bsp_timer_init();
	radio_init();
	radiotimer_init();
//	LPC_PINCON->PINSEL3      |= (0x1<<22);     // CLKOUT mode
//	LPC_SC->CLKOUTCFG|=(1<<2)|(1<<8); //to see the 32khz clock output

}

void board_sleep() {
	CLKPWR_Sleep();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

// GPIOs
// note: all GPIO interrupts, both port 0 and 2, trigger this same vector
void EINT3_IRQHandler(void) {
	if ((LPC_GPIOINT->IO0IntStatR) & (RADIO_ISR_MASK)) {
		LPC_GPIOINT->IO0IntClr = (RADIO_ISR_MASK);
		//capture timer
		radio_isr();
	}
}
