
#ifndef __FLASH_H
#define __FLASH_H

#include "stdint.h"
#include "status_codes.h"
//=========================== defines =========================================
#define ID_ADDRESS  (0x003FFC0)
#define ID_LENGTH   8
//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================


void flash_init(void);
void flash_write_ID(void);
void flash_read_ID(void);
void flash_getID(uint8_t* address);
enum status_code nvm_write(const uint32_t destination_address,
							uint8_t *const buffer,
							uint16_t length);
status_code_t nvm_read(uint32_t address, uint8_t *const buffer,
						uint32_t len);					
#endif