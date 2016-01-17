/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: EZR32WG-specific definition of the "uart" bsp module.
 */



#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "board.h"
#include "debugpins.h"

//=========================== defines =========================================

#define PIN_UART_RXD            GPIO_PIN_0 // PA0 is UART RX
#define PIN_UART_TXD            GPIO_PIN_1 // PA1 is UART TX

//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================

static void uart_isr_private(void);

//=========================== public ==========================================

void uart_init() { 
   // reset local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   
   // Disable UART function

   // Disable all UART module interrupts

   // Set IO clock as UART clock source

   // Map UART signals to the correct GPIO pins and configure them as
   // hardware controlled. GPIO-A pin 0 and 1


   // Configure the UART for 115,200, 8-N-1 operation.
   // This function uses SysCtrlClockGet() to get the system clock
   // frequency.  This could be also be a variable or hard coded value
   // instead of a function call.


   // Enable UART hardware


   // Disable FIFO as we only one 1byte buffer


   // Raise interrupt at end of tx (not by fifo)


   // Register isr in the nvic and enable isr at the nvic


   // Enable the UART0 interrupt
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
    uart_vars.txCb = txCb;
    uart_vars.rxCb = rxCb;
}

void uart_enableInterrupts(){

}

void uart_disableInterrupts(){

}

void uart_clearRxInterrupts(){

}

void uart_clearTxInterrupts(){

}

void  uart_writeByte(uint8_t byteToWrite){

}

uint8_t uart_readByte(){
	 int32_t i32Char = 0;

	 return (uint8_t)(i32Char & 0xFF);
}

//=========================== interrupt handlers ==============================

static void uart_isr_private(void){
	uint32_t reg;
	debugpins_isr_set();


	debugpins_isr_clr();
}

kick_scheduler_t uart_tx_isr() {
   uart_clearTxInterrupts();
   if (uart_vars.txCb != NULL) {
       uart_vars.txCb();
   }
   return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() {
   uart_clearRxInterrupts();
   if (uart_vars.rxCb != NULL) {
       uart_vars.rxCb();
   }
   return DO_NOT_KICK_SCHEDULER;
}
