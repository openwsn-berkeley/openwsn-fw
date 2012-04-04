///***********************************************************************
// * $Id::                                                               $
// *
// * Project:	spi:	A simple SSP demo for LPCXpresso 1700.
// * 					Uses SSP1 to display numerals 0 through 9 on
// * 					the 7-Segment display.
// * File:	main.c
// * Description:
// * 			SSP1 used to display numerals. Uses SPI frame format.
// *
// ***********************************************************************
// * Software that is described herein is for illustrative purposes only
// * which provides customers with programming information regarding the
// * products. This software is supplied "AS IS" without any warranties.
// * NXP Semiconductors assumes no responsibility or liability for the
// * use of the software, conveys no license or title under any patent,
// * copyright, or mask work right to the product. NXP Semiconductors
// * reserves the right to make changes in the software without
// * notification. NXP Semiconductors also make no representation or
// * warranty that such application will be suitable for the specified
// * use without further testing or modification.
// **********************************************************************/
//
//#ifdef __USE_CMSIS
//#include "LPC17xx.h"
//#endif
//
//#include "lpc17xx_ssp.h"
//
//#define SSP_CHANNEL LPC_SSP1
//#define PORT_CS	LPC_GPIO2
//#define PIN_MASK_CS (1<<2)
//static const uint8_t segmentLUT[10] =
//{
//		//FCPBAGED
//		(uint8_t) ~0b11011011, // 0
//		(uint8_t) ~0b01010000, // 1
//		(uint8_t) ~0b00011111, // 2
//		(uint8_t) ~0b01011101, // 3
//		(uint8_t) ~0b11010100, // 4
//		(uint8_t) ~0b11001101, // 5
//		(uint8_t) ~0b11001111, // 6
//		(uint8_t) ~0b01011000, // 7
//		(uint8_t) ~0b11011111, // 8
//		(uint8_t) ~0b11011101, // 9
//};
//
//int main(void){
//	volatile int timeKeeper=0;
//	unsigned char i=0;
//	SSP_CFG_Type sspChannelConfig;
//	SSP_DATA_SETUP_Type sspDataConfig;
//
//	uint8_t rxBuff[5];
//	uint8_t txBuff[5];
//
//	LPC_PINCON->PINSEL0 |= 0x2<<14; //SCK1
//	LPC_PINCON->PINSEL0 |= 0x2<<18;	//MOSI1
//	PORT_CS->FIODIR 	|= 1<<2;	//P2.2 as CSn
//
//	sspDataConfig.length = 1;
//	sspDataConfig.tx_data = txBuff;
//	sspDataConfig.rx_data = rxBuff;
//
//	SSP_ConfigStructInit(&sspChannelConfig);
//	SSP_Init(SSP_CHANNEL, &sspChannelConfig);
//	SSP_Cmd(SSP_CHANNEL, ENABLE);
//
//	while(1)
//	{
//		if (timeKeeper++ % 500000 == 0)
//		{
//			PORT_CS->FIOCLR |= PIN_MASK_CS;	//CS low
//
//			txBuff[0] = segmentLUT[i++];	//Buffer next numeral
//
//			//Only display 0-9
//			if (i==10) i=0;
//
//			//Transfer to 7-Segment Display Driver
//			SSP_ReadWrite(SSP_CHANNEL, &sspDataConfig, SSP_TRANSFER_POLLING);
//
//			PORT_CS->FIOSET |= PIN_MASK_CS; //CS High
//		}
//	}
//	return 0 ;
//}
