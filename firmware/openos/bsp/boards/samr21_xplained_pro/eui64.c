/* === INCLUDES ============================================================ */
#include "flash.h"
#include "eui64.h"
#include "compiler.h"

/*
 * @brief eui64_get Read the UID-64 address from the 
 *        user page flash memory
 *
 * @param addressToWrite buffer to read the data from nvm
 *
 */
void eui64_get(uint8_t* addressToWrite)
{
  /* Read the UID-64 of this device from user page */
  
   nvm_read(ID_ADDRESS, addressToWrite, ID_LENGTH);
}


