/**
\brief CC2538-specific definition of the "uart" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, September 2013.
*/


#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "uarthal.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "sys_ctrl.h"
#include "gpio.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "board_info.h"
#include "ioc.h"
#include "hw_ioc.h"
#include "debugpins.h"

//=========================== defines =========================================
#define PIN_UART_RXD            GPIO_PIN_0 //PA0 is UART rx
#define PIN_UART_TXD            GPIO_PIN_1 //PA1 is UART tx
//=========================== variables =======================================

typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;
uint8_t i=0;
//=========================== prototypes ======================================
void uart_isr_private(void);
//=========================== public ==========================================

void uart_init() {
   // reset local variables
   memset(&uart_vars,0,sizeof(uart_vars_t));
   
   // Disable UART function
   UARTDisable(UART0_BASE);
   // Disable all UART module interrupts
   UARTIntDisable(UART0_BASE, 0x1FFF);
   // Set IO clock as UART clock source
   UARTClockSourceSet(UART0_BASE, UART_CLOCK_SYSTEM);

   // Map UART signals to the correct GPIO pins and configure them as
   // hardware controlled. GPIO-A pin 0 and 1
   IOCPinConfigPeriphOutput(GPIO_A_BASE, PIN_UART_TXD, IOC_MUX_OUT_SEL_UART0_TXD);
   GPIOPinTypeUARTOutput(GPIO_A_BASE, PIN_UART_TXD);
   IOCPinConfigPeriphInput(GPIO_A_BASE, PIN_UART_RXD, IOC_UARTRXD_UART0);
   GPIOPinTypeUARTInput(GPIO_A_BASE, PIN_UART_RXD);

   // Configure the UART for 115,200, 8-N-1 operation.
   // This function uses SysCtrlClockGet() to get the system clock
   // frequency.  This could be also be a variable or hard coded value
   // instead of a function call.
   UARTConfigSetExpClk(UART0_BASE, SysCtrlClockGet(), 115200,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));


   // Set the UART to interrupt whenever the TX FIFO is almost empty or
   // when any character is received.
   //UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

   //register isr in the nvic
   UARTIntRegister(UART0_BASE, uart_isr_private);
   //enable isr at the nvic
   IntEnable(INT_UART0);
   //raise interrupt at end of tx (not by fifo)
   UARTTxIntModeSet(UART0_BASE,UART_TXINT_MODE_EOT);
   //enable UART hardware
   UARTEnable(UART0_BASE);
   //disable FIFO as we only one 1byte buffer
   UARTFIFODisable(UART0_BASE);
}

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT|UART_INT_TX);
}

void    uart_disableInterrupts(){
  UARTIntDisable(UART0_BASE, UART_INT_RX | UART_INT_RT|UART_INT_TX);
}

void    uart_clearRxInterrupts(){
  UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

void    uart_clearTxInterrupts(){
  UARTIntClear(UART0_BASE, UART_INT_TX);
}

void  uart_writeByte(uint8_t byteToWrite){
   UARTCharPutNonBlocking(UART0_BASE, byteToWrite);
   //UARTIntEnable(UART0_BASE, UART_INT_TX);
}

uint8_t uart_readByte(){
	 int32_t i32Char;
     i32Char = UARTCharGetNonBlocking(UART0_BASE);
	 return (uint8_t)(i32Char & 0xFF);
}

//=========================== interrupt handlers ==============================


void uart_isr_private(void){
	uint32_t reg;
	debugpins_isr_set();

	//read source
	reg = UARTIntStatus(UART0_BASE, true);
	//clear uart nvic interrupt
	IntPendClear(INT_UART0);
	//tx isr
	if(reg & UART_INT_TX){
	     uart_tx_isr();
	 	 //UARTIntDisable(UART0_BASE, UART_INT_TX);
	}
	//rx isr
	if(reg & (UART_INT_RX | UART_INT_RT)) {
		uart_rx_isr();
	}
	debugpins_isr_clr();
}

kick_scheduler_t uart_tx_isr() {
   uart_clearTxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.txCb();
   return DO_NOT_KICK_SCHEDULER;
}

kick_scheduler_t uart_rx_isr() {
   uart_clearRxInterrupts(); // TODO: do not clear, but disable when done
   uart_vars.rxCb();
   return DO_NOT_KICK_SCHEDULER;
}
