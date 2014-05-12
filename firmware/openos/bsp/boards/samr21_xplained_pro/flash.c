#include "flash.h"
#include "leds.h"
#include "stdint.h"
#include "string.h"
#include "nvm.h"
#include "system_interrupt.h"

//=========================== defines =========================================


//=========================== variables =======================================
uint8_t addressToWrite[ID_LENGTH];

//=========================== prototypes ======================================
#define NVM_MEMORY        ((volatile uint16_t *)FLASH_ADDR)
//=========================== public ==========================================

void flash_init(void)
{
	struct nvm_config config;
	/* Get the default configuration */
	nvm_get_config_defaults(&config);

	/* Set wait state to 2 */
	config.wait_states = 2;

	/* Set the NVM configuration */
	nvm_set_config(&config);
}

void flash_write_ID(void)
{
	nvm_write(ID_ADDRESS, addressToWrite, ID_LENGTH);
	leds_sync_toggle();
    memset(addressToWrite , 0, ID_LENGTH);
}

void flash_read_ID(void)
{
  nvm_read(ID_ADDRESS, addressToWrite, ID_LENGTH);
  leds_sync_toggle();
}

void flash_getID(uint8_t* address)
{
 memcpy(address, (uint8_t *)addressToWrite, ID_LENGTH);
}


enum status_code nvm_write(const uint32_t destination_address,
							uint8_t *const buffer,
							uint16_t length)
{
	enum status_code error_code = STATUS_OK;
	uint8_t row_buffer[NVMCTRL_ROW_PAGES * FLASH_PAGE_SIZE];
	volatile uint8_t *dest_add = (uint8_t *)destination_address;
	const uint8_t *src_buf = buffer;
	uint32_t i;

	/* Calculate the starting row address of the page to update */
	uint32_t row_start_address =
	destination_address & ~((FLASH_PAGE_SIZE * NVMCTRL_ROW_PAGES) - 1);

	while (length) 
	{
		/* Backup the contents of a row */
		for (i = 0; i < NVMCTRL_ROW_PAGES; i++) 
		{
			do
			{
				error_code = nvm_read_buffer(row_start_address + (i * FLASH_PAGE_SIZE),
											(row_buffer + (i * FLASH_PAGE_SIZE)), FLASH_PAGE_SIZE);
			} while (error_code == STATUS_BUSY);
		}
		
		/* Update the buffer if necessary */
		for (i = row_start_address; i < row_start_address + (FLASH_PAGE_SIZE * NVMCTRL_ROW_PAGES); i++)
		{
			if (length && ((uint8_t *)i == dest_add)) 
			{
				row_buffer[i-row_start_address] = *src_buf++;
				dest_add++;
				length--;
			}
		}
		
		system_interrupt_enter_critical_section();

		/* Erase the row */
		do
		{
			error_code = nvm_erase_row(row_start_address);
		} while (error_code == STATUS_BUSY);

		/* Write the updated row contents to the erased row */
		for (i = 0; i < NVMCTRL_ROW_PAGES; i++) 
		{
			do
			{
				error_code = nvm_write_buffer(row_start_address + (i * FLASH_PAGE_SIZE),
				                              (row_buffer + (i * FLASH_PAGE_SIZE)), FLASH_PAGE_SIZE);
			} while (error_code == STATUS_BUSY);
		}

		system_interrupt_leave_critical_section();

		row_start_address += NVMCTRL_ROW_PAGES * NVMCTRL_PAGE_SIZE;
	}

	return error_code;
}


status_code_t nvm_read(uint32_t address, uint8_t *const buffer,
						uint32_t len)
{
	/* Get a pointer to the module hardware instance */
	Nvmctrl *const nvm_module = NVMCTRL;
	    
	/* Check if the module is busy */
	if (!nvm_is_ready()) 
	{
		return STATUS_BUSY;
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
	return STATUS_OK;
}


//=========================== private =========================================