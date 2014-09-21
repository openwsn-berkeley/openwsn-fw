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

#define ID_ADDRESS  0x1FFFF7E8
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
}

void flash_write_ID()
{
    for(uint8_t i = 0; i<ID_LENGTH;i++)
    {
      FLASH_ProgramHalfWord(ID_ADDRESS + i*2, addressToWrite[i]);
      leds_sync_toggle();
    }
    memset(addressToWrite , 0, ID_LENGTH);
}

void flash_read_ID()
{
    for(uint8_t i = 0; i<ID_LENGTH;i++)
    {
      addressToWrite[i] = *(uint8_t*)(ID_ADDRESS + i);
      leds_error_toggle();
    }
}

void flash_getID(uint8_t* address)
{
  memcpy(address,addressToWrite,ID_LENGTH);
}

//=========================== private =========================================