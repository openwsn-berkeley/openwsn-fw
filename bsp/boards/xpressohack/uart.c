



/**
\brief LPC17XX-specific definition of the "UART" bsp module. Note that this module only enables UART0.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "uart_config.h"
#include "LPC17xx.h"
#include "clkpwr.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

//=========================== prototypes ======================================
//extern void UART0_IRQHandler (void); //weak function defined in cr_startup_lpc17.c


//static void uart_set_divisors(uint32_t baudrate);
//=========================== public ==========================================

void uart_init() {

	//UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfg;

	uint32_t tmp;


	/*
	 * Initialize UART0 pin connect
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);


	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init((LPC_UART_TypeDef *)LPC_UART0, &UARTConfigStruct);


	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART0, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd((LPC_UART_TypeDef *)LPC_UART0, ENABLE);

	/* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_RLS, ENABLE);
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	NVIC_EnableIRQ(UART0_IRQn);

}





void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
	uart_vars.txCb = txCb;
	uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, ENABLE);
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_RBR, ENABLE);
}

void    uart_disableInterrupts(){
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, DISABLE);
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_RBR, DISABLE);
}

void    uart_clearRxInterrupts(){
	//do nothing, are done in the isr to read the line status.
}

void    uart_clearTxInterrupts(){
	//do nothing, are done in the isr to read the line status.
}

void    uart_writeByte(uint8_t byteToWrite){
	//UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, DISABLE);
	//UART_Send((LPC_UART_TypeDef *)LPC_UART0,&byteToWrite,1,NONE_BLOCKING);
	UART_SendByte((LPC_UART_TypeDef *)LPC_UART0,byteToWrite);
	//LPC_UART0->/*DLIER.*/THR |=byteToWrite;
	while (UART_CheckBusy((LPC_UART_TypeDef *)LPC_UART0));
	//UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, ENABLE);
}

uint8_t uart_readByte(){
	uint8_t byteToRCV;
	byteToRCV=UART_Receive((LPC_UART_TypeDef *)LPC_UART0,&byteToRCV,1,NONE_BLOCKING);
	return byteToRCV;
}

//=========================== interrupt handlers ==============================

uint8_t uart_isr_tx() {
	uart_clearTxInterrupts(); // TODO: do not clear, but disable when done
	uart_vars.txCb();
	return 0;
}

uint8_t uart_isr_rx() {
	uart_clearRxInterrupts(); // TODO: do not clear, but disable when done
	uart_vars.rxCb();
	return 0;
}


void UART0_IRQHandler (void)
{
	uint32_t intsrc, tmp, tmp1;

	/* Determine the interrupt source */
	intsrc = UART_GetIntId((LPC_UART_TypeDef *)LPC_UART0);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS){
		// Check line status
		tmp1 = UART_GetLineStatus((LPC_UART_TypeDef *)LPC_UART0);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
				| UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		if (tmp1) {
			while (1);
		}
	}

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)){
		uart_isr_rx();
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE){
		uart_isr_tx();
	}
}

