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

//=========================== prototypes ======================================

//=========================== public ==========================================

void flash_init()
{   
    FLASH_Unlock(); 
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);
}

uint8_t flash_erasePage(uint32_t address)
{
  uint8_t status;
  status = FLASH_ErasePage(address);
  return status;
}

uint8_t  flash_erase_optByte()
{
  uint8_t status;
  status = FLASH_EraseOptionBytes();
  return status;
}

uint8_t flash_write(uint32_t address,uint16_t data)
{
  uint8_t status;
  status = FLASH_ProgramHalfWord(address,data);
  return status;
}

uint8_t flash_write_optByte(uint32_t address,uint8_t data)
{
  uint8_t status;
  status = FLASH_ProgramOptionByteData(address,data);
  return status;
}

uint16_t flash_read(uint32_t address)
{
  uint16_t temp = 0x00;
  temp = *(uint32_t*)address;
  return temp;
}

uint16_t flash_read_optByte(uint32_t address)
{
  uint16_t temp = 0x00;
  temp = *(uint32_t*)address;
  return temp;
}

void flash_getID(uint32_t address)
{
  
}
//=========================== private =========================================