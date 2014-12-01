/**
\brief iot-lab_M3 definition of the "eui64" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
*/

#include "string.h"
#include "eui64.h"

//=========================== defines =========================================
// stm32f103rey, 96-bit unique ID address
#define UNIQUE_ID_BASE_ADDRESS          0x1FFFF7E8

//=========================== variables =======================================

const uint8_t const *uid = (const uint8_t *const) UNIQUE_ID_BASE_ADDRESS;

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite)
{
  addressToWrite[0] = uid[7];
  addressToWrite[1] = uid[6];
  addressToWrite[2] = uid[5];
  addressToWrite[3] = uid[4];
  addressToWrite[4] = uid[3];
  addressToWrite[5] = uid[0];
  addressToWrite[6] = uid[1];
  addressToWrite[7] = uid[2];
}

//=========================== private =========================================
