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


#include <sam.h>
#include "samr21_flash.h"

/** \brief  nvm_read will read the internal flash content from the 
			given address. the length and fill buffer should be passed 
			while calling this function.     

    \param [in]  address
	\param [in]  buffer
	\param [in]  len
    \return None
 */

void nvm_read(uint32_t address, uint8_t *const buffer, uint32_t len)
{
	/* Get a pointer to the module hardware instance */
	Nvmctrl *const nvm_module = NVMCTRL;
	
	/* Check if the module is busy */
	while(!(nvm_module->INTFLAG.reg & NVMCTRL_INTFLAG_READY)){
		
	}

	/* Clear error flags */
	nvm_module->STATUS.reg |= NVMCTRL_STATUS_MASK;

	uint32_t page_address = address / 2;

	/* NVM _must_ be accessed as a series of 16-bit words, perform manual copy
	 * to ensure alignment */
	for (uint16_t i = 0; i < len; i += 2) {
		/* Fetch next 16-bit chunk from the NVM memory space */
		uint16_t data = NVM_MEMORY[page_address++];

		/* Copy first byte of the 16-bit chunk to the destination buffer */
		buffer[i] = (data & 0xFF);

		/* If we are not at the end of a read request with an odd byte count,
		 * store the next byte of data as well */
		if (i < (len - 1)) 
		{
			buffer[i + 1] = (data >> 8);
		}
	}
	
}

/*
 * @brief  flash_init Init the Flash memory and 
 *         set the default config
 *
 * @param[in] wait_state
 *
 */
void nvm_init(uint8_t wait_state)
{
 /* Get a pointer to the module hardware instance */
 Nvmctrl *const nvm_module = NVMCTRL;

 /* Turn on module in PM */
 PM->APBBMASK.reg |= PM_APBBMASK_NVMCTRL;
 
 /* Clear error flags */
 nvm_module->STATUS.reg |= NVMCTRL_STATUS_MASK;

 /* Check if the module is busy */
 while(!(nvm_module->INTFLAG.reg & NVMCTRL_INTFLAG_READY)){
	 
 }

 /* Writing configuration to the CTRLB register */
 nvm_module->CTRLB.reg = NVMCTRL_CTRLB_SLEEPPRM_WAKEONACCESS | ((0 & 0x01) << NVMCTRL_CTRLB_MANW_Pos) |\
						 NVMCTRL_CTRLB_RWS(wait_state) | ((0 & 0x01) << NVMCTRL_CTRLB_CACHEDIS_Pos) |\
						 NVMCTRL_CTRLB_READMODE(0);

 /* If the security bit is set, the auxiliary space cannot be written */
 while (nvm_module->STATUS.reg & NVMCTRL_STATUS_SB) {
	 
 }

}