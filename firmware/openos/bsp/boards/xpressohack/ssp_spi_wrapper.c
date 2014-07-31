/**
\brief LPC17XX-specific definition of the "spi" bsp module. Adapted from CMSIS LPC SSP module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 **/

#include "board.h"
#include "clkpwr.h"
#include "LPC17xx.h"
#include "ssp.h"
#include "spi.h"
#include "lpc17xx_pinsel.h"

#define SPI_SCK_PIN          7        /* Clock       */
#define SPI_SSEL_PIN         6        /* CS-Select    */
#define SPI_MISO_PIN         8        /* from Slave    */
#define SPI_MOSI_PIN         9        /* to Slave       */

#define PIN_MASK_CS (1<<6)

void    ssp_spi_init(){
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.6 - SSEL
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 */

	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);

	LPC_PINCON->PINSEL0     &= ~(0x3<<12);  //  [P0.6] SSEL1 as GPIO. auto ssp does not work because it toggles cs between bytes.

	LPC_GPIO0->FIODIR |= (1 << SPI_SCK_PIN) | (1 << SPI_MOSI_PIN) | (1 << SPI_SSEL_PIN);
	LPC_GPIO0->FIODIR &= ~(1 << SPI_MISO_PIN);

	LPC_GPIO0->FIOSET       |= PIN_MASK_CS;      //             - set high

	SSP_Init(LPC_SSP1,&SSP_ConfigStruct);
	SSP_Cmd(LPC_SSP1, ENABLE);

}

void ssp_spi_txrx(uint8_t* bufTx, uint8_t lenbufTx,  spi_return_t returnType, uint8_t*     bufRx,   uint8_t maxLenBufRx, spi_first_t  isFirst,  spi_last_t   isLast){
	uint32_t stat=0;
	SSP_DATA_SETUP_Type data;
	if (isFirst==SPI_FIRST) {
		LPC_GPIO0->FIOCLR                 |=PIN_MASK_CS;
	}

	data.tx_data=bufTx;				/**< Pointer to transmit data */
	data.rx_data=bufRx;				/**< Pointer to transmit data */
	data.length=lenbufTx;			/**< Length of transfer data */

	stat= SSP_ReadWrite (LPC_SSP1, &data, SSP_TRANSFER_POLLING);

	if (isLast==SPI_LAST) {
		LPC_GPIO0->FIOSET                 |=PIN_MASK_CS;
	}
}
