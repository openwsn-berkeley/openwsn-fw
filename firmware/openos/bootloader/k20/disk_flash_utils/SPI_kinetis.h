/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file spi.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief  This file is SPI Driver Header File
 *****************************************************************************/
#ifndef __SPI__
#define __SPI__


/* Includes */
#include "derivative.h"


/* Defines */

/* Prototypes */
void   SPI_Init(void);
uint_8 SPI_Receive_byte(void);
void   SPI_Send_byte(uint_8 data);
void   SPI_High_rate(void);
void   SPI_CS_assert(uint_8 state);

#endif /* __SPI__ */
