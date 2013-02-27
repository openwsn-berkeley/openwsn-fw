/**
\brief openmoteSTM32 definition of the "leds" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stm32f10x_lib.h"
#include "flash.h"
#include "leds.h"
#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

#define ID_ADDRESS  0x0807F800
#define ID_LENGTH   8

//=========================== variables =======================================
    uint8_t addressToWrite[ID_LENGTH];

//=========================== prototypes ======================================

//=========================== public ==========================================

void flash_init()
{
    RCC_HSICmd(ENABLE);
    
    FLASH_Unlock(); 
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
    
    addressToWrite[0] = 0x00;
    addressToWrite[1] = 0x01;
    addressToWrite[2] = 0x02;
    addressToWrite[3] = 0x03;
    addressToWrite[4] = 0x04;
    addressToWrite[5] = 0x05;    
    addressToWrite[6] = 0x06;
    addressToWrite[7] = 0x07;
}

void flash_write_ID()
{
    for(uint8_t i = 0; i<ID_LENGTH;i++)
    {
      FLASH_ProgramHalfWord(ID_ADDRESS + i*2, addressToWrite[i]);
      leds_sync_toggle();
      delay();
    }
    memset(addressToWrite , 0, ID_LENGTH);
}

void flash_read_ID()
{
    for(uint8_t i = 0; i<ID_LENGTH;i++)
    {
      addressToWrite[i] = *(uint16_t*)(ID_ADDRESS + i*2);
      leds_error_toggle();
      delay();
    }
}

void flash_getID(uint8_t* address)
{
  memcpy(address,addressToWrite,ID_LENGTH);
}

//=========================== private =========================================