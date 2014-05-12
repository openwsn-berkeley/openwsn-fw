



#include "flash.h"
#include "eui64.h"
#include "compiler.h"

#define DEVICE_EUI64_ID  (0x14159212FFFF1201UL)

void eui64_get(uint8_t* addressToWrite)
{
   nvm_read(ID_ADDRESS, addressToWrite, ID_LENGTH);
   addressToWrite[7] = (uint8_t)DEVICE_EUI64_ID;
   addressToWrite[6] = (uint8_t)(DEVICE_EUI64_ID >> 8);
   addressToWrite[5] = (uint8_t)(DEVICE_EUI64_ID >> 16);
   addressToWrite[4] = (uint8_t)(DEVICE_EUI64_ID >> 24);
   addressToWrite[3] = (uint8_t)(DEVICE_EUI64_ID >> 32);
   addressToWrite[2] = (uint8_t)(DEVICE_EUI64_ID >> 40);
   addressToWrite[1] = (uint8_t)(DEVICE_EUI64_ID >> 48);
   addressToWrite[0] = (uint8_t)(DEVICE_EUI64_ID >> 56);
}

