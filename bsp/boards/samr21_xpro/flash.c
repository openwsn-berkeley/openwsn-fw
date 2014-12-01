/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

/* === INCLUDES ============================================================ */
#include "flash.h"
#include "leds.h"
#include "stdint.h"
#include "string.h"
#include "samr21_flash.h"


//=========================== defines =========================================


//=========================== variables =======================================
uint8_t addressToWrite[ID_LENGTH];

//=========================== prototypes ======================================
#define NVM_MEMORY        ((volatile uint16_t *)FLASH_ADDR)
//=========================== public ==========================================

/*
 * @brief  flash_init Init the Flash memory and 
 *         set the default config
 *
 * @param  None
 *
 */
void flash_init(void)
{
	nvm_init(2);
}

/*
 * @brief  flash_write_ID Write the ID to Flash memory
 *
 * @param  None
 *
 */
void flash_write_ID(void)
{
	//nvm_write(ID_ADDRESS, addressToWrite, ID_LENGTH);
	leds_sync_toggle();
    memset(addressToWrite , 0, ID_LENGTH);
}

/*
 * @brief  flash_read_ID Read the ID from Flash memory
 *
 * @param  None
 *
 */
void flash_read_ID(void)
{
  nvm_read(ID_ADDRESS, addressToWrite, ID_LENGTH);
  leds_sync_toggle();
}

