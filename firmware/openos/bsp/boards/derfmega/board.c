/**
\brief board setup file for derfmega

\author Kevin Weekly <kweekly@eecs.berkeley.edu>, June 2012.
*/

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================


extern uint8_t radio_rx_start_isr();
extern uint8_t radio_trx_end_isr();
extern uint8_t radiotimer_compare_isr();
extern uint8_t radiotimer_overflow_isr();

//=========================== main ============================================
volatile uint8_t reset_source;

extern int mote_main();

int main() {
   reset_source = MCUSR;
   MCUSR = 0;   
   return mote_main();
}

//=========================== public ==========================================

void board_init() {
   // disable watchdog timer
   wdt_reset();
   wdt_disable();
   /*
   MCUSR &= ~(1<<WDRF);
   WDTCSR |= (1<<WDCE) | (1<<WDE);
   WDTCSR = 0x00;
     */ 
   // setup clock speed (Use configuration bits, EXTCLK, NO CLKDIV8, no BOD)
   
   // turn off power to all periphrals ( will be enabled specifically later)
   PRR0 = 0x00;
   PRR1 = 0x00;
   // enable data retention
   DRTRAM0 |= 0x10; 
   DRTRAM1 |= 0x10;
   DRTRAM2 |= 0x10;
   DRTRAM3 |= 0x10;
   // initialize bsp modules
   debugpins_init();
   leds_init();
   uart_init();
   bsp_timer_init();
   radio_init();
   radiotimer_init();
   
   // enable interrupts
   sei();   
}

void board_sleep() {
   SMCR = (0x03<<1) | 1; // power-save, enable sleep
 //  sleep_cpu();
   SMCR &= 0xFE; // disable sleep
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

// UART0 interrupt
// pass to uart_isr_rx/tx
ISR(USART0_RX_vect) {
	uart_isr_rx(); // doing nothing w/ return value
}

ISR(USART0_TX_vect) {
	uart_isr_tx();
}


// radio interrupt(s)
// pass to radio_isr
ISR(TRX24_RX_START_vect) {
	radio_rx_start_isr(); // doing nothing w/ return value
}

ISR(TRX24_RX_END_vect) {
	radio_trx_end_isr();
}
ISR(TRX24_TX_END_vect) {
	radio_trx_end_isr();
}

// MAC symbol counter interrupt compare 1
// pass to bsp_timer_isr
ISR(SCNT_CMP1_vect) {
	bsp_timer_isr();
}

//MAC symbol counter interrupt compare 2/3
// pass to radiotimer_isr
ISR(SCNT_CMP2_vect) {
	radiotimer_compare_isr();
}

ISR(SCNT_CMP3_vect) {
	radiotimer_overflow_isr();
}


// buttons (none)


// error
ISR(BADISR_vect) {
	static const char msg[] = "BADISR\n";
	char c = 0;
	while(1) {
		for (c = 0; c < sizeof(msg); c++) {
			uart_writeByte(msg[c]);
		}
	}
}