/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file SD.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief  This file is SD Card Driver Header File 
 *****************************************************************************/
#ifndef __SD__
#define __SD__

/* Includes */
#include "derivative.h"
#include "SPI_kinetis.h"
#include "usb_msc.h"
#include "types.h"

/* User definitions */
#define SD_BLOCK_512     
#define SD_WAIT_CYCLES 10

#if (defined __MK_xxx_H__)
    #define SD_DE                       (GPIOA_PDOR | GPIO_PSOR_PTSO(16))// Card detection (AN0 input register)
    #define _SD_DE                      GPIOA_PCOR = 1 << 16// Card detection (AN0 direction: input)
    
    #define kSD_Insert  0
    #define kSD_Desert  (1<<15)
#endif

#if (defined __MK_xxx_H__)
    #define SD_WR                       GPIOE_PSOR | GPIO_PSOR_PTSO(27))// Card detection (AN0 input register)
    #define _SD_WR                      GPIOE_PSOR |= 1 << 27// Card detection (AN0 direction: input)
    
    #define kSD_WP      (1<<27)
    #define kSD_WnP     0
#endif

/* TypeDef */
typedef union
{
    uint_8  bytes[4];
    uint_32 lword;
}T32_8;

typedef union
{
    uint_8  u8[2];
    uint_16 u16;
}T16_8;

/* Static Definitions */
/******************************* SD Card Standard Commands **********************************/
enum{
  SD_CMD0,  /* Resets the SD Memory Card */
  SD_CMD1,  /* Sends host capacity support information and activates the card's 
               initialization process. HCS is effective when card receives SEND_IF_COND 
               command. Reserved bits shall be set to '0'. */
  SD_CMD2,
  SD_CMD3,
  SD_CMD4,
  SD_CMD5,  
  SD_CMD6,  /* Checks switchable function (mode 0) and switches card function (mode 1).*/
  SD_CMD7,
  SD_CMD8,  /* Sends SD Memory Card interface condition that includes host supply voltage 
               information and asks the accessed card whether card can operate in supplied 
               voltage range. Reserved bits shall be set to '0'.*/
  SD_CMD9,  /* Asks the selected card to send its cardspecific data (CSD)*/
  SD_CMD10, /* Asks the selected card to send its card identification (CID) */
  SD_CMD11,
  SD_CMD12, /* Forces the card to stop transmission in Multiple Block Read Operation */
  SD_CMD13, /* Asks the selected card to send its status register. */
  SD_CMD14,
  SD_CMD15,
  SD_CMD16, /* Sets a block length (in bytes) for all following block commands (read and
               write) of a Standard Capacity Card. Block length of the read and write 
               commands are fixed to 512 bytes in a High Capacity Card. The length of 
               LOCK_UNLOCK command is set by this command in both capacity cards.*/
  SD_CMD17, /* Reads a block of the size selected by the SET_BLOCKLEN command.*/
  SD_CMD18, /* Continuously transfers data blocks from card to host until interrupted by a
               STOP_TRANSMISSION command.*/
  SD_CMD19, 
  SD_CMD20,
  SD_CMD21,
  SD_CMD22,
  SD_CMD23,
  SD_CMD24, /* Writes a block of the size selected by the SET_BLOCKLEN command. */
  SD_CMD25, /* Continuously writes blocks of data until ’Stop Tran’ token is sent
               (instead ’Start Block’).*/
  SD_CMD26,
  SD_CMD27, /* Programming of the programmable bits of the CSD. */
  SD_CMD28, /* If the card has write protection features, this command sets the write protection bit
               of the addressed group. The properties of write protection are coded in the card 
               specific data (WP_GRP_SIZE). The High Capacity Card does not support this command.*/
  SD_CMD29, /* If the card has write protection features, this command clears the write protection
               bit of the addressed group. The High Capacity Card does not support this command. */
  SD_CMD30, /* If the card has write protection features, this command asks the card to send the 
               status of the write protection bits.6 The High Capacity Card does not support this command. */
  SD_CMD31,
  SD_CMD32, /* Sets the address of the first write block to be erased.*/
  SD_CMD33, /* Sets the address of the last write block of the continuous range to be erased. */
  SD_CMD34,
  SD_CMD35,
  SD_CMD36,
  SD_CMD37,
  SD_CMD38, /* Erases all previously selected write blocks */
  SD_CMD39,
  SD_CMD40,
  SD_CMD41,
  SD_CMD42, /* Used to Set/Reset the Password or lock/unlock the card. A transferred data block includes 
               all the command details - refer to Chapter 4.3.7. The size of the Data Block is defined 
               with SET_BLOCK_LEN command. Reserved bits in the argument and in Lock Card Data Structure 
               shall be set to 0. */
  SD_CMD43,
  SD_CMD44,
  SD_CMD45,
  SD_CMD46,
  SD_CMD47,
  SD_CMD48,
  SD_CMD49,
  SD_CMD50,
  SD_CMD51,
  SD_CMD52,
  SD_CMD53,
  SD_CMD54,
  SD_CMD55, /* Defines to the card that the next command is an application specific command 
               rather than a standard command */
  SD_CMD56, /* Used either to transfer a Data Block to the card or to get a Data Block from the card 
               for general purpose/application specific commands. In case of Standard Capacity SD 
               Memory Card, the size of the Data Block shall be defined with SET_BLOCK_LEN command. 
               Block length of this command is fixed to 512-byte in High Capacity Card. */
  SD_CMD57,
  SD_CMD58, /* Reads the OCR register of a card. CCS bit is assigned to OCR[30]. */
  SD_CMD59, /* Turns the CRC option on or off. A ‘1’ in the CRC option bit will turn the option on,
               a ‘0’ will turn it off */
  SD_CMD60,
  SD_CMD61,
  SD_CMD62,
  SD_CMD63
};
 
enum
{
    SD_OK,
    SD_IDLE
};


#ifdef SD_BLOCK_512
    #define SD_BLOCK_SIZE   (0x00000200)
    #define SD_BLOCK_SHIFT  (9)
    #define BLOCK_SIZE      512
#endif




/* Prototypes */
uint_8 SD_Init(void);
uint_8 SD_SendCommand(uint_8,uint_8);
void   SD_CLKDelay(uint_8);
void   SD_Write_Block(PTR_LBA_APP_STRUCT);
void   SD_Read_Block(PTR_LBA_APP_STRUCT);
uint_8 SD_ReadCSD(void);
void   SD_Card_Info(uint_32_ptr max_blocks_ptr, uint_32_ptr block_size_ptr); 

#endif /* __SD__ */

