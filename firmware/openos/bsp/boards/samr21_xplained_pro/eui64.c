#include "flash.h"
#include "eui64.h"
#include "compiler.h"

/* Read the UID-64 address from the user page flash memory */
void eui64_get(uint8_t* addressToWrite)
{
   nvm_read(ID_ADDRESS, addressToWrite, ID_LENGTH);
}


