/**
\brief Zigduino definition of the "leds" bsp module.

\author Sven Akkermans <sven.akkermans@cs.kuleuven.be>, September 2015.
 */

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "board.h"

// bsp modules
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"

//=========================== variables =======================================

volatile int return_to_sleep = 1;

//=========================== prototypes ======================================
extern uint8_t radio_rx_start_isr();
extern uint8_t radio_trx_end_isr();
extern uint8_t radiotimer_compare_isr();
extern uint8_t radiotimer_overflow_isr();

//=========================== main ============================================
uint8_t mcusr_backup;

extern int mote_main(void);

int main(void) {
	mcusr_backup = MCUSR; // Read reset-source
	MCUSR = 0;

	wdt_reset(); 	//disable watchdog timer as quickly as possible
	wdt_disable();
	return mote_main();
}

//=========================== public ==========================================

void board_init() {


	// turn off power to all periphrals ( will be enabled specifically later)
	PRR0 = 0x00;
	PRR1 = 0x00;

	//disable interrupts
	cli();


	// initialize bsp modules
	debugpins_init();
	leds_init();
	uart_init();
	spi_init();
	bsp_timer_init();
	radio_init();
	radiotimer_init();

	// enable interrupts
	sei();
}

// Uses high-level functions from avr/sleep.h
void board_sleep() {
	//  Symbol Counter can wakeup if the Transceiver
	// Oscillator is enabled (Transceiver not in SLEEP).
	//Todo work with a more aggressive power mode?
	cli();
	set_sleep_mode(SLEEP_MODE_IDLE); // select power down mode
	sei();

	sleep_enable();
	sleep_cpu();
	sleep_disable();

	while(1){
		if(return_to_sleep){
			sleep_enable();
			sleep_cpu();
			sleep_disable();
		} else {
			return_to_sleep = 1;
			return;
		}
	}
}

void board_reset() {
	  wdt_reset();
	  WDTCSR = (1 << WDCE) | (1 << WDE);
	  WDTCSR = (1 << WDE);
	  while (1);
}
//=========================== private =========================================

//=========================== interrupt handlers ==============================

// Based from the derfmega and from avr/interrupt.h because this macro
// is defined elsewhere also, this overrides all
#define ISR(vector, ...)            \
		void vector (void) __attribute__ ((signal,__INTR_ATTRS)) __VA_ARGS__; \
		void vector (void)

// UART0 interrupt
// pass to uart_isr_rx/tx
ISR(USART0_RX_vect) {
	debugpins_isr_set();
	if(uart_rx_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}
	debugpins_isr_clr();
}

ISR(USART0_TX_vect) {
	debugpins_isr_set();
	if(uart_tx_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}
// radio interrupt(s)
// pass to radio_isr
ISR(TRX24_RX_START_vect) {
	debugpins_isr_set();
	if(radio_rx_start_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}

ISR(TRX24_RX_END_vect) {
	debugpins_isr_set();
	if(radio_trx_end_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}

ISR(TRX24_TX_END_vect) {
	debugpins_isr_set();
	if(radio_trx_end_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}

ISR(SCNT_CMP1_vect) {
	debugpins_isr_set();
	if(bsp_timer_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}

ISR(SCNT_CMP2_vect) {
	debugpins_isr_set();
	if(radiotimer_compare_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}

ISR(SCNT_CMP3_vect) {
	debugpins_isr_set();
	if(radiotimer_overflow_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}

//The 32bit counter will overflow after quite some time.
//The radiotimer is all relative and will continue, the bsptimer needs to execute a special procedure to continue working.
ISR(SCNT_OVFL_vect) {
	debugpins_isr_set();
	if(bsp_timer_overflow_isr()==KICK_SCHEDULER){
		return_to_sleep=0;
	}

	debugpins_isr_clr();
}
/* Hang on any unsupported interrupt */
/* Useful for diagnosing unknown interrupts that reset the mcu.
 * Currently set up for 12mega128rfa1.
 * For other mcus, enable all and then disable the conflicts.
 */
ISR( _VECTOR(0)) {print_msg("0 ISR raised. \n");}
ISR( _VECTOR(1)) {print_msg("1 ISR raised. \n");}
ISR( _VECTOR(2)) {print_msg("2 ISR raised. \n");}
ISR( _VECTOR(3)) {print_msg("3 ISR raised. \n");}
ISR( _VECTOR(4)) {print_msg("4 ISR raised. \n");}
ISR( _VECTOR(5)) {print_msg("5 ISR raised. \n");}
ISR( _VECTOR(6)) {print_msg("6 ISR raised. \n");}
ISR( _VECTOR(7)) {print_msg("7 ISR raised. \n");}
ISR( _VECTOR(8)) {print_msg("8 ISR raised. \n");}
ISR( _VECTOR(9)) {print_msg("9 ISR raised. \n");}
ISR( _VECTOR(10)) {print_msg("10 ISR raised. \n");}
ISR( _VECTOR(11)) {print_msg("11 ISR raised. \n");}
ISR( _VECTOR(12)) {print_msg("12 ISR raised. \n");}
ISR( _VECTOR(13)) {print_msg("13 ISR raised. \n");}
ISR( _VECTOR(14)) {print_msg("14 ISR raised. \n");}
ISR( _VECTOR(15)) {print_msg("15 ISR raised. \n");}
ISR( _VECTOR(16)) {print_msg("16 ISR raised. \n");}
ISR( _VECTOR(17)) {print_msg("17 ISR raised. \n");}
ISR( _VECTOR(18)) {print_msg("18 ISR raised. \n");}
ISR( _VECTOR(19)) {print_msg("19 ISR raised. \n");}
ISR( _VECTOR(20)) {print_msg("20 ISR raised. \n");}
ISR( _VECTOR(21)) {print_msg("21 ISR raised. \n");}
ISR( _VECTOR(22)) {print_msg("22 ISR raised. \n");}
ISR( _VECTOR(23)) {print_msg("23 ISR raised. \n");}
ISR( _VECTOR(24)) {print_msg("24 ISR raised. \n");}
//ISR( _VECTOR(25)) {print_msg("25 ISR raised. \n");}
ISR( _VECTOR(26)) {print_msg("26 ISR raised. \n");}
//ISR( _VECTOR(27)) {print_msg("27 ISR raised. \n");}
ISR( _VECTOR(28)) {print_msg("28 ISR raised. \n");}
ISR( _VECTOR(29)) {print_msg("29 ISR raised. \n");}
ISR( _VECTOR(30)) {print_msg("30 ISR raised. \n");}
ISR( _VECTOR(31)) {print_msg("31 ISR raised. \n");}
ISR( _VECTOR(32)) {print_msg("32 ISR raised. \n");}
ISR( _VECTOR(33)) {print_msg("33 ISR raised. \n");}
ISR( _VECTOR(34)) {print_msg("34 ISR raised. \n");}
ISR( _VECTOR(35)) {print_msg("35 ISR raised. \n");}
ISR( _VECTOR(36)) {print_msg("36 ISR raised. \n");}
ISR( _VECTOR(37)) {print_msg("37 ISR raised. \n");}
ISR( _VECTOR(38)) {print_msg("38 ISR raised. \n");}
ISR( _VECTOR(39)) {print_msg("39 ISR raised. \n");}
ISR( _VECTOR(40)) {print_msg("40 ISR raised. \n");}
ISR( _VECTOR(41)) {print_msg("41 ISR raised. \n");}
ISR( _VECTOR(42)) {print_msg("42 ISR raised. \n");}
ISR( _VECTOR(43)) {print_msg("43 ISR raised. \n");}
ISR( _VECTOR(44)) {print_msg("44 ISR raised. \n");}
ISR( _VECTOR(45)) {print_msg("45 ISR raised. \n");}
ISR( _VECTOR(46)) {print_msg("46 ISR raised. \n");}
ISR( _VECTOR(47)) {print_msg("47 ISR raised. \n");}
ISR( _VECTOR(48)) {print_msg("48 ISR raised. \n");}
ISR( _VECTOR(49)) {print_msg("49 ISR raised. \n");}
ISR( _VECTOR(50)) {print_msg("50 ISR raised. \n");}
ISR( _VECTOR(51)) {print_msg("51 ISR raised. \n");}
ISR( _VECTOR(52)) {print_msg("52 ISR raised. \n");}
ISR( _VECTOR(53)) {print_msg("53 ISR raised. \n");}
ISR( _VECTOR(54)) {print_msg("54 ISR raised. \n");}
ISR( _VECTOR(55)) {print_msg("55 ISR raised. \n");}
ISR( _VECTOR(56)) {print_msg("56 ISR raised. \n");}
ISR( _VECTOR(57)) {print_msg("57 ISR raised. \n");}
ISR( _VECTOR(58)) {print_msg("58 ISR raised. \n");}
//ISR( _VECTOR(59)) {print_msg("59 ISR raised. \n");}
//ISR( _VECTOR(60)) {print_msg("60 ISR raised. \n");}
ISR( _VECTOR(61)) {print_msg("61 ISR raised. \n");}
ISR( _VECTOR(62)) {print_msg("62 ISR raised. \n");}
//ISR( _VECTOR(63)) {print_msg("63 ISR raised. \n");}
ISR( _VECTOR(64)) {print_msg("64 ISR raised. \n");}
//ISR( _VECTOR(65)) {print_msg("65 ISR raised. \n");}
//ISR( _VECTOR(66)) {print_msg("66 ISR raised. \n");}
//ISR( _VECTOR(67)) {print_msg("67 ISR raised. \n");}
//ISR( _VECTOR(68)) {print_msg("68 ISR raised. \n");}
ISR( _VECTOR(69)) {print_msg("69 ISR raised. \n");}
ISR( _VECTOR(70)) {print_msg("70 ISR raised. \n");}
ISR( _VECTOR(71)) {print_msg("71 ISR raised. \n");}
ISR( _VECTOR(72)) {print_msg("72 ISR raised. \n");}
ISR( _VECTOR(73)) {print_msg("73 ISR raised. \n");}
ISR( _VECTOR(74)) {print_msg("74 ISR raised. \n");}
ISR( _VECTOR(75)) {print_msg("75 ISR raised. \n");}
ISR( _VECTOR(76)) {print_msg("76 ISR raised. \n");}
ISR( _VECTOR(77)) {print_msg("77 ISR raised. \n");}
ISR( _VECTOR(78)) {print_msg("78 ISR raised. \n");}
ISR( _VECTOR(79)) {print_msg("79 ISR raised. \n");}

// error
ISR(BADISR_vect) {
	print_msg("BADISR\n");
}

void print_msg(char msg[]){
	char c = 0;
	while(1) {
		for (c = 0; c < sizeof(msg); c++) {
			uart_writeByte(msg[c]);
		}
	}
}
