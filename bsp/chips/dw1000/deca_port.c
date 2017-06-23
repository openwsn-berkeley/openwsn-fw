/*
 * deca_port.c
 *
 *  Created on: 21 Mar 2017
 *      Author: JeanMichel.Rubillon
 */
#include "board.h"
#include "opendefs.h"
#include "bsp_timer.h"
#include "spi.h"
#include "deca_device_api.h"
#include "deca_regs.h"

int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readLength, uint8 *readBuffer)
{
	//TODO: Need to handle larger SPI read operations. Right now truncated to 255 bytes max
	spi_txrx(headerBuffer, headerLength, SPI_BUFFER, readBuffer, readLength, SPI_FIRST, SPI_LAST);
	return 0;
}

int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodyLength, const uint8 *bodyBuffer)
{
	uint8_t dummy_buffer[3];
	spi_txrx(headerBuffer, headerLength, SPI_BUFFER, dummy_buffer, 0, SPI_FIRST, SPI_NOTLAST);
	spi_txrx(bodyBuffer, bodyLength, SPI_BUFFER, dummy_buffer, 0, SPI_NOTFIRST, SPI_LAST);
	return 0;
}

void deca_sleep( unsigned int time_ms)
{
#if INCLUDE_vTaskDelay
	// FREERTOS delay function
	TickType_t ticks = millisec / portTICK_PERIOD_MS;
	vTaskDelay(ticks ? ticks : 1);
#else
	// Make use of the bsp_timer module
	PORT_TIMER_WIDTH end_time = 0;
	end_time = bsp_timer_get_currentValue() + time_ms;
	while( bsp_timer_get_currentValue() < end_time);
#endif
}

decaIrqStatus_t decamutexon(void)
{
#ifdef taskENTER_CRITICAL
	taskENTER_CRITICAL();
#else
#endif
	return 0;
}

void decamutexoff(decaIrqStatus_t state)
{
#ifdef taskEXIT_CRITICAL
	taskEXIT_CRITICAL();
#else
	
#endif
}

void deca_spi_init(uint8_t speed){
    SPI_InitTypeDef  SPI_InitStructure;

    SPI_Cmd(SPI1, DISABLE);
	//Configure SPI1
	SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //Full-duplex synchronous transfers on two lines
	SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;//Master Mode
	SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b; //8-bit transfer frame format
	SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;  //the SCK pin has a low-level idle state 
	SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge; //the first rising edge on the SCK pin is the MSBit capture strobe,
	SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;//Software NSS mode
	SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;//data order with MSB-first
	SPI_InitStructure.SPI_CRCPolynomial     = 10;//CRC Polynomial = 10
	if(!speed){
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//BaudRate Prescaler = 32 Slow SPI clock
	}
	else{
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//BaudRate Prescaler = 4 Fast SPI clock
	}
	SPI_Init(SPI1, &SPI_InitStructure);
  
    //enable SPI1
    SPI_Cmd(SPI1, ENABLE);
}

