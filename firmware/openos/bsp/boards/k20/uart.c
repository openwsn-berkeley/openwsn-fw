/*
 * File:        uart.c
 * Purpose:     Provide common UART routines for serial IO
 *
 * Notes:       
 *              
 */

#include "derivative.h"
#include "uart.h"
#include "uart_config.h"



typedef struct {
   uart_tx_cbt txCb;
   uart_rx_cbt rxCb;
} uart_vars_t;


uart_vars_t uart_vars;


void private_uart_init (UART_MemMapPtr uartch, int sysclk, int baud);
extern  core_clk_khz;



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
    uint16_t sbr, brfa;
    uint8_t temp;
    
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
    UART_C2_REG(uartch) &= ~(UART_C2_TE_MASK
				| UART_C2_RE_MASK );

    /* Configure the UART for 8-bit mode, no parity */
    UART_C1_REG(uartch) = 0;	/* We need all default settings, so entire register is cleared */
    
    /* Calculate baud settings */
    sbr = (uint16_t)((sysclk*1000)/(baud * 16));
        
    /* Save off the current value of the UARTx_BDH except for the SBR field */
    temp = UART_BDH_REG(uartch) & ~(UART_BDH_SBR(0x1F));
    
    UART_BDH_REG(uartch) = temp |  UART_BDH_SBR(((sbr & 0x1F00) >> 8));
    UART_BDL_REG(uartch) = (uint8_t)(sbr & UART_BDL_SBR_MASK);
    
    /* Determine if a fractional divider is needed to get closer to the baud rate */
    brfa = (((sysclk*32000)/(baud * 16)) - (sbr * 32));
    
    /* Save off the current value of the UARTx_C4 register except for the BRFA field */
    temp = UART_C4_REG(uartch) & ~(UART_C4_BRFA(0x1F));
    
    UART_C4_REG(uartch) = temp |  UART_C4_BRFA(brfa);    
    
    enable_irq(INT_UART1_RX_TX);
    
    /* Enable receiver and transmitter */
	UART_C2_REG(uartch) |= (UART_C2_TE_MASK| UART_C2_RE_MASK );
	
	
}


void uart_init ( )
{
	 
	 memset(&uart_vars,0,sizeof(uart_vars_t));
	 
	 PORTE_PCR0 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin
	 	
	   		/* Enable the UART1_RXD function on PTE1 */
	 PORTE_PCR1 = PORT_PCR_MUX(0x3); // UART is alt3 function for this pin
	 
	 
	 private_uart_init(UART1_BASE_PTR,core_clk_khz,115200);
}
/********************************************************************/
/*
 * Wait for a character to be received on the specified UART
 *
 * Parameters:
 *  channel      UART channel to read from
 *
 * Return Values:
 *  the received character
 */
char uart_getchar (UART_MemMapPtr channel)
{
    /* Wait until character has been received */
    while (!(UART_S1_REG(channel) & UART_S1_RDRF_MASK));
    
    /* Return the 8-bit data from the receiver */
    return UART_D_REG(channel);
}




//=========================== prototypes ======================================

//=========================== public ==========================================


void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   uart_vars.txCb = txCb;
   uart_vars.rxCb = rxCb;
}

void    uart_enableInterrupts(){
	UART_C2_REG(UART1_BASE_PTR)|=UART_C2_RIE_MASK|UART_C2_TIE_MASK;
	//UART1_C2|=UART_C2_RIE_MASK|UART_C2_TIE_MASK;
}

void    uart_disableInterrupts(){
	UART_C2_REG(UART1_BASE_PTR)&=~(UART_C2_RIE_MASK|UART_C2_TIE_MASK);
	//UART1_C2 &=~(UART_C2_RIE_MASK|UART_C2_TIE_MASK);
}

void    uart_clearRxInterrupts(){
    /* Wait until character has been received */
 //   while (!(UART_S1_REG(channel) & UART_S1_RDRF_MASK));//Receive Data Register Full Flag

}

void    uart_clearTxInterrupts(){
	// while(!(UART_S1_REG(channel) & UART_S1_TDRE_MASK));//Transmit Data Register Empty Flag
//done at the isr.
}

void    uart_writeByte(uint8_t byteToWrite){
	UART_D_REG(UART1_BASE_PTR) = byteToWrite;
}

uint8_t uart_readByte(){
  return UART_D_REG(UART1_BASE_PTR);
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
	uint32_t reg=UART_S1_REG(UART1_BASE_PTR);
	//this clears the interrupt so now we have to see which one is:
	
	//Receive Data Register Full Flag
	if (reg>>UART_S1_RDRF_SHIFT & 0x1){
		//rx
		uart_isr_rx();
		
	}else if (reg>>UART_S1_TDRE_SHIFT & 0x1){
		uart_isr_tx();
	}else{
		while(1); //to check what other isr happened.
	}
	
}
