/**
\brief K20-specific definition of the "uart" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */

#include "common.h"
#include "uart.h"
#include "uart_config.h"
#include "board.h"

typedef struct {
	uart_tx_cbt txCb;
	uart_rx_cbt rxCb;
} uart_vars_t;

uart_vars_t uart_vars;

void private_uart_init (UART_MemMapPtr uartch, int sysclk, int baud);
extern void uart_isr(void);

extern core_clk_khz;
extern mcg_clk_hz;
/********************************************************************/
/*
 * Initialize the UART for 8N1 operation, interrupts disabled, and
 * no hardware flow-control
 *
 * NOTE: Since the UARTs are pinned out in multiple locations on most
 *       Kinetis devices, this driver does not enable UART pin functions.
 *       The desired pins should be enabled before calling this init function.
 *
 * Parameters:
 *  uartch      UART channel to initialize
 *  sysclk      UART module Clock in kHz(used to calculate baud)
 *  baud        UART baud rate
 */
void private_uart_init (UART_MemMapPtr uartch, int sysclk, int baud)
{
	uint16_t sbr;
	uint16_t brfa;
	uint8_t temp,txsize,shiftval;
	uint32_t  pFIFO; 
	/* Enable the clock to the selected UART */    
	if(uartch == UART0_BASE_PTR)
		SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;
	else
		if (uartch == UART1_BASE_PTR)
			SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;
		else
			if (uartch == UART2_BASE_PTR)
				SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
			else
				if(uartch == UART3_BASE_PTR)
					SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;
				else
					if(uartch == UART4_BASE_PTR)
						SIM_SCGC1 |= SIM_SCGC1_UART4_MASK;					

	/* Make sure that the transmitter and receiver are disabled while we 
	 * change settings.
	 */
	UART_C2_REG(uartch) &= ~(UART_C2_TE_MASK| UART_C2_RE_MASK );

	/* Configure the UART for 8-bit mode, no parity */
	UART_C1_REG(uartch) = 0;	/* We need all default settings, so entire register is cleared */
	UART_C3_REG(uartch) = 0;

	/* Calculate baud settings */
	sbr = (uint16_t)((sysclk)/(baud * 16));

	/* Save off the current value of the UARTx_BDH except for the SBR field */
	temp = UART_BDH_REG(uartch) & ~(UART_BDH_SBR(0x1F));

	UART_BDH_REG(uartch) = temp |  UART_BDH_SBR(((sbr & 0x1F00) >> 8));
	UART_BDL_REG(uartch) = (uint8_t)(sbr & UART_BDL_SBR_MASK);

	/* Determine if a fractional divider is needed to get closer to the baud rate 
	 * 
	 * 71.991296Mhz clk, and 115200 baud rate  sbr=39 and brfa=2
	 * 71991296 Hz /(16*(39+(2/32))=115186.07/115200=99.98%
	 * */
	brfa = ((sysclk*32)/(baud * 16));
    brfa =  brfa - (sbr * 32);
    //so why this calculation according to kinetis examples does not work?
    
    //force 2
    brfa=2;
	
    /* Save off the current value of the UARTx_C4 register except for the BRFA field */
	
	temp = UART_C4_REG(uartch) & ~(UART_C4_BRFA(0x1F));

	UART_C4_REG(uartch) = temp | UART_C4_BRFA(brfa);//2 is the righ value.  

	UART_C5_REG(uartch) = 0; //use interrupt not dma.


		/* set watermark in the almost full TX buffer */
	//	if ((( UART_PFIFO_REG(uartch) & UART_PFIFO_TXFIFOSIZE_MASK) >> UART_PFIFO_TXFIFOSIZE_SHIFT) == 0) {
			/* 1 dataword in D */
		UART_TWFIFO_REG(uartch) = UART_TWFIFO_TXWATER(0);
//		}
//		else {
//			pFIFO=UART_PFIFO_REG(uartch) & UART_PFIFO_TXFIFOSIZE_MASK;
//			shiftval=(((pFIFO) >> UART_PFIFO_TXFIFOSIZE_SHIFT) + 1);
//			txsize = 1 << shiftval;
//	    /* watermark for TX buffer generates interrupts below & equal to watermark */
//			txsize -= 1;
//	        UART_TWFIFO_REG(uartch) = UART_TWFIFO_TXWATER(txsize);
//	    
//		}


	UART_RWFIFO_REG(uartch) |= UART_RWFIFO_RXWATER(1); //rx buffer is 1 byte

	UART_CFIFO_REG(uartch)   |=UART_CFIFO_RXFLUSH_MASK|UART_CFIFO_TXFLUSH_MASK; //flush buffers

		//UART_PFIFO_REG(uartch)   |= UART_PFIFO_RXFE_MASK; //enable fifo
		//UART_PFIFO_REG(uartch)   |= UART_PFIFO_TXFE_MASK; //enable fifo

	temp=UART_S1_REG(uartch);//clear isr flags

	//see page 67 of the manual
	enable_irq(UART1_IRQ_NUM);

	/* Enable receiver and transmitter */
	UART_C2_REG(uartch) |= (UART_C2_TE_MASK| UART_C2_RE_MASK );
}


void uart_init ( )
{

	memset(&uart_vars,0,sizeof(uart_vars_t));
#if (defined(OPENMOTE_K20))
	PORTC_PCR3 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin
		/* Enable the UART1_RXD function on PTC3 */
    PORTC_PCR4 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin  PTC4
    
#elif (defined(	TOWER_K20))
	PORTE_PCR0 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin

	/* Enable the UART1_RXD function on PTE1 */
	PORTE_PCR1 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin
#endif

	private_uart_init(UART1_BASE_PTR,mcg_clk_hz,BAUD_RATE);
}



//=========================== prototypes ======================================

//=========================== public ==========================================


void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
	uart_vars.txCb = txCb;
	uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
	UART_C2_REG(UART1_BASE_PTR)|=UART_C2_RIE_MASK/*|UART_C2_TIE_MASK/*|UART_C2_TCIE_MASK/*|UART_C2_ILIE_MASK*/;
}

void    uart_disableInterrupts(){
	UART_C2_REG(UART1_BASE_PTR)&=~(UART_C2_RIE_MASK/*|UART_C2_TIE_MASK/*|UART_C2_TCIE_MASK/*|UART_C2_ILIE_MASK*/);
}

void    uart_clearRxInterrupts(){
	/* this is done when reading status*/	
}

void    uart_clearTxInterrupts(){
	/* this is done when reading status*/	
}

void uart_writeByte(uint8_t byteToWrite){
	while(!(UART_S1_REG(UART1_BASE_PTR) &  UART_S1_TDRE_MASK));	//wait tx complete flag

	UART_D_REG(UART1_BASE_PTR) = byteToWrite;

	UART_C2_REG(UART1_BASE_PTR)|=UART_C2_TIE_MASK;//enable interrupts

}

uint8_t uart_readByte(){
	while (!(UART_S1_REG(UART1_BASE_PTR) & UART_S1_RDRF_MASK));
	return  UART_D_REG(UART1_BASE_PTR); 
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


void uart_isr(void)
{
	uint32_t reg;
	debugpins_isr_set();

	reg=UART_S1_REG(UART1_BASE_PTR);
	//this clears the interrupt so now we have to see which one is:
	if (reg & UART_S1_TDRE_MASK){
		UART_C2_REG(UART1_BASE_PTR)&=~(UART_C2_TIE_MASK/*|UART_C2_TCIE_MASK*/);//disable interrupts
		uart_isr_tx();
		
		// tx complete isr.
	}
	if (reg & UART_S1_TC_MASK){
		
	}
	
   //Receive Data Register Full Flag
	if (reg & UART_S1_RDRF_MASK){
		//rx
		uart_isr_rx();
	}
	debugpins_isr_clr();
}
