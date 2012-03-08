/**
\brief LPC17XX-specific definition of the "spi" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */


/**
 * The SPI is configured using the following registers: (page 401 of the manual)
 *	1. Power: In the PCONP register (Table 46), set bit PCSPI.
 *	Remark: On reset, the SPI is enabled (PCSPI = 1).
 *	2. Clock: In the PCLKSEL0 register (Table 40), set bit PCLK_SPI. In master mode, the
 *	clock must be an even number greater than or equal to 8 (see Section 17.7.4).
 *	3. Pins: The SPI pins are configured using both PINSEL0 (Table 79) and PINSEL1
 *	(Table 80), as well as the PINMODE (Section 8.4) register. PINSEL0[31:30] is used to
 *	configure the SPI CLK pin. PINSEL1[1:0], PINSEL1[3:2] and PINSEL1[5:4] are used
 * 	to configure the pins SSEL, MISO and MOSI, respectively.
 *	4. Interrupts: The SPI interrupt flag is enabled using the S0SPINT[0] bit (Section 17.7.7).
 * 	The SPI interrupt flag must be enabled in the NVIC, see Table 50.
 *
 *
 */
#include "spi.h"
#include "board.h"
#include "clkpwr.h"
#include "LPC17xx.h"

#define PORT_CS	LPC_GPIO2
#define PIN_MASK_CS (1<<2)
#define SPI_IE (1<<7)
#define SPI_MSB (1<<6)
#define SPI_MST (1<<5)
#define SPI_SPIF (1<<7)//transfer complete flag
#define SPI_CPOL (1<<4)
#define SPI_CPHA (1<<3)
//=========================== variables =======================================

typedef struct {
	// information about the current transaction
	uint8_t*        pNextTxByte;
	uint8_t         numTxedBytes;
	uint8_t         txBytesLeft;
	spi_return_t    returnType;
	uint8_t*        pNextRxByte;
	uint8_t         maxRxBytes;
	spi_first_t     isFirst;
	spi_last_t      isLast;
	// state of the module
	uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
	// callback when module done
	spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================



extern  void  SPI_IRQHandler(void);
//=========================== public ==========================================
void spi_init(){

	/*Power: In the PCONP register (Table 46), set bit PCSPI.*/

	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCSPI,ENABLE);

	/*Clock: In the PCLKSEL0 register (Table 40), set bit PCLK_SPI. In master mode, the
	clock must be an even number greater than or equal to 8 (see Section 17.7.4).*/

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_SPI,CLKPWR_PCLKSEL_CCLK_DIV_4);//default clock

	/* SPCCR clock prescale register, master mode min value 8*/
	LPC_SPI->SPCCR = 0x8;

	/*Pins: The SPI pins are configured using both PINSEL0 (Table 79) and PINSEL1
	(Table 80), as well as the PINMODE (Section 8.4) register. PINSEL0[31:30] is used to
	configure the SPI CLK pin. PINSEL1[1:0], PINSEL1[3:2] and PINSEL1[5:4] are used
	to configure the pins SSEL, MISO and MOSI, respectively.*/

	LPC_PINCON->PINSEL0 |= 0x2<<14;  //SCK1 - 0b10 at pin 14,15
	//LPC_PINCON->PINSEL0 |= 0x2<<12;	//SSEL1 - 0b10 at pin 12,13 --used by SSP as Chip select.
	LPC_PINCON->PINSEL0 |= 0x2<<18;	//MOSI1 -(Master out, slave in) - 0b10 at pin 18,19
	LPC_PINCON->PINSEL0 |= 0x2<<16;	//MISO1 (Master in, slave out)- 0b10 at pin 16,17

	PORT_CS->FIODIR 	|= 1<<2;	//P2.2 as CSn


	/*		4. Interrupts: The SPI interrupt flag is enabled using the S0SPINT[0] bit (Section 17.7.7).
		The SPI interrupt flag must be enabled in the NVIC, see Table 50.
	 */
	LPC_SPI->SPCR|=SPI_MSB|SPI_MST;// MSB first, master mode
	LPC_SPI->SPCR&=~SPI_CPOL; //clock polarity high
	LPC_SPI->SPCR&=~SPI_CPHA; //clock phase to 0

	/*  0b111000XX
	 *    ||||||||__ Reserved
	 *    |||||||___ Reserved
	 *    ||||||____ BitEnable --> 0 if 8 bits per transfer. 1 if bits 8:11 specify the amount of bits per transfer (8-16)
	 *    |||||_____ CPHA -->  clock phase control. 0-first clock edge of SCK, 1-2nd clock edge
	 *    ||||______ CPOL --> clock polarity - 0 SCK is active.
	 *    |||_______ MSTR --> 0 slave, 1 master
	 *    ||________ LSBF --> least significant bit first.0 is LSBF, 1 is MSBF.
	 *    |_________ SPIE --> Interrupt enable. 1 if active.
	 *               (bits 8-11) When bit 2 of this register is 1, this field controls the number of bits per transfer (8-16. pag 407)
	 *
	 */

	NVIC_EnableIRQ(SPI_IRQn);
#ifdef SPI_IN_INTERRUPT_MODE
	LPC_SPI->SPCR|=SPI_IE; //enable interrupt if interrupt mode on.
#endif
}

#ifdef SPI_IN_RTOS_MODE
void spi_setCallback(spi_cbt cb) {
	spi_vars.callback = cb;
}
#endif



void    spi_txrx(uint8_t*     bufTx,
		uint8_t      lenbufTx,
		spi_return_t returnType,
		uint8_t*     bufRx,
		uint8_t      maxLenBufRx,
		spi_first_t  isFirst,
		spi_last_t   isLast){

	uint32_t reg;


#ifdef SPI_IN_INTERRUPT_MODE
	// disable interrupts
	__disable_irq();
#endif

	// register spi frame to send
	spi_vars.pNextTxByte      =  bufTx;
	spi_vars.numTxedBytes     =  0;
	spi_vars.txBytesLeft      =  lenbufTx;
	spi_vars.returnType       =  returnType;
	spi_vars.pNextRxByte      =  bufRx;
	spi_vars.maxRxBytes       =  maxLenBufRx;
	spi_vars.isFirst          =  isFirst;
	spi_vars.isLast           =  isLast;

	// SPI is now busy
	spi_vars.busy             =  1;

	// lower CS signal to have slave listening
	if (spi_vars.isFirst==SPI_FIRST) {
		PORT_CS->FIOCLR                 |=1<<2;
	}

#ifdef SPI_IN_INTERRUPT_MODE
	// implementation 1. use a callback function when transaction finishes
	// re-enable interrupts
		__enable_irq();

	// write first byte to TX buffer
	LPC_SPI->SPDR                 = *spi_vars.pNextTxByte;
//reg=LPC_SPI->SPSR;
//reg=LPC_SPI->SPINT;

#else
	// implementation 2. busy wait for each byte to be sent

	//	 send all bytes
	while (spi_vars.txBytesLeft>0) {
		// write next byte to TX buffer
		LPC_SPI->SPDR              = *spi_vars.pNextTxByte;
		// busy wait on the interrupt flag
		while ((LPC_SPI->SPSR & SPI_SPIF )==0);
		// clear the interrupt flag
		LPC_SPI-> SPINT                 |= 1;//cleared by writting 1 on this register.
		// save the byte just received in the RX buffer
		switch (spi_vars.returnType) {
		case SPI_FIRSTBYTE:
			if (spi_vars.numTxedBytes==0) {
				*spi_vars.pNextRxByte   = LPC_SPI->SPDR  ;
			}
			break;
		case SPI_BUFFER:
			*spi_vars.pNextRxByte      = LPC_SPI->SPDR  ;
			spi_vars.pNextRxByte++;
			break;
		case SPI_LASTBYTE:
			*spi_vars.pNextRxByte      = LPC_SPI->SPDR  ;
			break;
		}
		// one byte less to go
		spi_vars.pNextTxByte++;
		spi_vars.numTxedBytes++;
		spi_vars.txBytesLeft--;
	}

	// put CS signal high to signal end of transmission to slave
	if (spi_vars.isLast==SPI_LAST) {
		PORT_CS->FIOSET                 |=1<<2;
	}

	// SPI is not busy anymore
	spi_vars.busy             =  0;
#endif



/**
 *  Read the SPIF bit in the SPI status register.
 *  Read the SPI Data register.
 *  Finally, clear the SPIF bit in the SPI interrupt register.
 *
 */
	void  SPI_IRQHandler(void){
#ifdef SPI_IN_INTERRUPT_MODE

		//Read the SPIF bit in the SPI status register.
		while ((LPC_SPI->SPSR & SPI_SPIF )==0);

		// save the byte just received in the RX buffer
		// Read the SPI Data register.
		switch (spi_vars.returnType) {
		case SPI_FIRSTBYTE:
			if (spi_vars.numTxedBytes==0) {
				*spi_vars.pNextRxByte = LPC_SPI->SPDR ;
			}
			break;
		case SPI_BUFFER:
			*spi_vars.pNextRxByte    = LPC_SPI->SPDR ;
			spi_vars.pNextRxByte++;
			break;
		case SPI_LASTBYTE:
			*spi_vars.pNextRxByte    = LPC_SPI->SPDR ;
			break;
		}

		// one byte less to go
		spi_vars.pNextTxByte++;
		spi_vars.numTxedBytes++;
		spi_vars.txBytesLeft--;

		LPC_SPI-> SPINT                 |= 1;//cleared by writting 1 on this register.

		if (spi_vars.txBytesLeft>0) {
			// write next byte to TX buffer
			LPC_SPI->SPDR               = *spi_vars.pNextTxByte;
		} else {
			// put CS signal high to signal end of transmission to slave
			if (spi_vars.isLast==SPI_LAST) {
				PORT_CS->FIOSET                 |=1<<2;
			}
			// SPI is not busy anymore
			spi_vars.busy          =  0;


			// SPI is done!
			if (spi_vars.callback!=NULL) {
				// call the callback
				spi_vars.callback();
				// kick the OS
				return 1;
			}
		}
#else
		while(1);// this should never happen
#endif
	}

}
