/******************************************************************************
*  (c) copyright Freescale Semiconductor China Ltd. 2008
*  ALL RIGHTS RESERVED
*  File Name: SD.C
*  Description: SD Card Drivers
*  Assembler:  Codewarrior for HC(S)08 V6.0
*  Version: 1.0                                                         
*  Author: Jose Ruiz
*  Location: Guadalajuara, Mexico                                              
*                                                  
* UPDATED HISTORY:
* REV   YYYY.MM.DD  AUTHOR        DESCRIPTION OF CHANGE
* ---   ----------  ------        --------------------- 
* 1.0   2008.01.02  Jose Ruiz      Initial version
******************************************************************************/                                                                        
/* Freescale  is  not  obligated  to  provide  any  support, upgrades or new */
/* releases  of  the Software. Freescale may make changes to the Software at */
/* any time, without any obligation to notify or provide updated versions of */
/* the  Software  to you. Freescale expressly disclaims any warranty for the */
/* Software.  The  Software is provided as is, without warranty of any kind, */
/* either  express  or  implied,  including, without limitation, the implied */
/* warranties  of  merchantability,  fitness  for  a  particular purpose, or */
/* non-infringement.  You  assume  the entire risk arising out of the use or */
/* performance of the Software, or any systems you design using the software */
/* (if  any).  Nothing  may  be construed as a warranty or representation by */
/* Freescale  that  the  Software  or  any derivative work developed with or */
/* incorporating  the  Software  will  be  free  from  infringement  of  the */
/* intellectual property rights of third parties. In no event will Freescale */
/* be  liable,  whether in contract, tort, or otherwise, for any incidental, */
/* special,  indirect, consequential or punitive damages, including, but not */
/* limited  to,  damages  for  any loss of use, loss of time, inconvenience, */
/* commercial loss, or lost profits, savings, or revenues to the full extent */
/* such  may be disclaimed by law. The Software is not fault tolerant and is */
/* not  designed,  manufactured  or  intended by Freescale for incorporation */
/* into  products intended for use or resale in on-line control equipment in */
/* hazardous, dangerous to life or potentially life-threatening environments */
/* requiring  fail-safe  performance,  such  as  in the operation of nuclear */
/* facilities,  aircraft  navigation  or  communication systems, air traffic */
/* control,  direct  life  support machines or weapons systems, in which the */
/* failure  of  products  could  lead  directly to death, personal injury or */
/* severe  physical  or  environmental  damage  (High  Risk Activities). You */
/* specifically  represent and warrant that you will not use the Software or */
/* any  derivative  work of the Software for High Risk Activities.           */
/* Freescale  and the Freescale logos are registered trademarks of Freescale */
/* Semiconductor Inc.                                                        */ 
/*****************************************************************************/

/* Includes */
#include "types.h"
#include "SD_kinetis.h"

/* Gobal Variables */
uint_8  vSD_CSD[16];        /* SD card CSD vaule    */
uint_8  vSD_Bnum;           /* Block number         */
uint_32 vSD_LBA;            /* BLock address        */

T32_8 u32MaxBlocks;
T16_8 u16BlockSize;

T32_8 gu8SD_Argument;
uint_8 gu8SD_CID[16];

/************************************************/
uint_8 SD_Init(void) 
{
    /*Body*/
    SPI_Init();                 /* SPI Initialization*/
	
    SD_CLKDelay(10);            /* Send 80 clocks*/ 

    gu8SD_Argument.lword = 0;
    SD_CLKDelay(8);  
    
    /* IDLE Command */
    if(SD_SendCommand(SD_CMD0|0x40,SD_IDLE))
    {
        return(1);              /* Command IDLE fail*/
    }/*Endif*/

    (void)SPI_Receive_byte();   /* Dummy SPI cycle*/
    
    /*  Initialize SD Command */
    while(SD_SendCommand(SD_CMD1|0x40,SD_OK))
    {
        Watchdog_Reset();
    }/*EndWhile*/
    
    (void)SPI_Receive_byte();   /*Dummy SPI cycle */

     /*  Block Length */
    gu8SD_Argument.lword = BYTESWAP32(SD_BLOCK_SIZE); 
    
    if(SD_SendCommand(SD_CMD16|0x40,SD_OK))
    {       
        return(1);              /* Command IDLE fail */
    }/*Endif*/

    SPI_High_rate();
    
    SPI_Send_byte(0x00);
    SPI_Send_byte(0x00);
    return(0);

}/*End Body*/

/************************************************/
void SD_Write_Block(PTR_LBA_APP_STRUCT lba_data_ptr)
{
    /* Body */
    uint_32 u32Counter, time_out;

    gu8SD_Argument.lword = BYTESWAP32(lba_data_ptr->offset); 

    if(SD_SendCommand(SD_CMD24|0x40,SD_OK))
    {
        return;   /* Command IDLE fail */
    } /* EndIf */
    
    SPI_Send_byte(0xFE);
    
    for(u32Counter=0;u32Counter<lba_data_ptr->size;u32Counter++)  
    {
        SPI_Send_byte(*(lba_data_ptr->buff_ptr + u32Counter));
    } /* EndFor */

    SPI_Send_byte(0xFF);    /* checksum Bytes not needed */
    SPI_Send_byte(0xFF);

    
    for (time_out=0;time_out<1000;time_out++)
    {
        if((SPI_Receive_byte() & 0x1F) < 16)
        {
            break;
        } /* EndIf */
    }/* EndFor */
    
    if (time_out>=1000)
        return;

    while(SPI_Receive_byte()==0x00)
    {
        Watchdog_Reset();
    } /* EndWhile */      
       
    return;
}

/************************************************/
void SD_Read_Block(PTR_LBA_APP_STRUCT lba_data_ptr)
{
    uint_8 u8Temp=0;
    uint_32 u32Counter;

    gu8SD_Argument.lword = BYTESWAP32(lba_data_ptr->offset); 

    if(SD_SendCommand(SD_CMD17|0x40,SD_OK))
    {       
        return; /* Command IDLE fail*/
    }
    
    while(u8Temp!=0xFE)  
    {      
        u8Temp = SPI_Receive_byte(); 
    }/*EndWhile*/
    
    for(u32Counter=0;u32Counter<lba_data_ptr->size;u32Counter++)  
    {
        *(lba_data_ptr->buff_ptr + u32Counter) = SPI_Receive_byte(); 
    }
        
    (void)SPI_Receive_byte();  /*   Dummy SPI cycle */
    (void)SPI_Receive_byte();  /*   Dummy SPI cycle */

    (void)SPI_Receive_byte();  /*   Dummy SPI cycle */
    
    return;
}/*EndBody*/



/************************************************/
uint_8 SD_SendCommand(uint_8 u8SDCommand,uint_8 u8SDResponse) 
{
    /*Body*/
    uint_8 u8Counter;
    volatile uint_8 u8Temp=0;

    /* Send Start byte */
    SPI_Send_byte(u8SDCommand);

    /* Send Argument */
    for(u8Counter=0;u8Counter<4;u8Counter++) 
        SPI_Send_byte(gu8SD_Argument.bytes[u8Counter]);
  
    /* Send CRC */
    SPI_Send_byte(0x95);
  
    /* Response RHandler */
    u8Counter=SD_WAIT_CYCLES;
    do
    {
        u8Temp=SPI_Receive_byte();
        u8Counter--;
    }while((u8Temp != u8SDResponse) && u8Counter > 0);
    
    if(u8Counter)   return(0);
    else            return(1);
}/*EndBody*/

/************************************************/
void SD_CLKDelay(uint_8 u8Frames) 
{
    /*Body*/
    while(u8Frames--)
        SPI_Send_byte(0xFF);
}/*EndBody*/

/*********************************************************
* Name: SD_ReadCSD
* Desc: Read CSD vaule of SD card
* Parameter: None
* Return: Status of read -- Fail:04  Success:00
**********************************************************/

uint_8 SD_ReadCSD(void)
{
    uint_8 i;
    /*Body*/
    if(SD_SendCommand(SD_CMD9|0x40,SD_OK))
    { 
        return(4);      
    }/*EndIf*/
    
    while(i!=0xFE)
        i=SPI_Receive_byte();
    
    for(i=0;i<16;i++)
        vSD_CSD[i]=SPI_Receive_byte();

    (void)SPI_Receive_byte();  
    (void)SPI_Receive_byte();  
      
    (void)SPI_Receive_byte();  
    
    return(0);
}/*EndBody*/

/*********************************************************
* Name: SD_Card_Info
* Desc: Storage information of SD Card
* Parameter: None
* Return: None
**********************************************************/  
void SD_Card_Info(uint_32_ptr max_blocks_ptr, uint_32_ptr block_size_ptr) 
{
    uint_16 u16Temp;

    (void)SD_ReadCSD();

    /*  Block Size  */
    u16BlockSize.u16= (uint_16)(1<<(vSD_CSD[5] & 0x0F));

    /*  Max Blocks  */
    u16Temp= (uint_16)((vSD_CSD[10]&0x80)>>7);
    u16Temp+=(vSD_CSD[9]&0x03)<<1;
    u32MaxBlocks.lword= (uint_16)(vSD_CSD[6]&0x03);
    u32MaxBlocks.lword=u32MaxBlocks.lword<<8;
    u32MaxBlocks.lword+=vSD_CSD[7];
    u32MaxBlocks.lword=u32MaxBlocks.lword<<2;
    u32MaxBlocks.lword+=(vSD_CSD[8]&0xC0)>>6;
    u32MaxBlocks.lword++;
    u32MaxBlocks.lword=u32MaxBlocks.lword<<(u16Temp+2);
    
    /*  Patch for SD Cards of 2 GB  */
    if(u16BlockSize.u16 > 512)
    {
        u16BlockSize.u16=512;
        u32MaxBlocks.lword=u32MaxBlocks.lword<<1;
    }/*EndIf*/
    
    *max_blocks_ptr = u32MaxBlocks.lword;
    *block_size_ptr = (uint_32)u16BlockSize.u16;
}/*EndBody*/
