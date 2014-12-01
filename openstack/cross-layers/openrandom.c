#include "opendefs.h"
#include "openrandom.h"
#include "idmanager.h"

//=========================== variables =======================================

random_vars_t random_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void openrandom_init() {
   // seed the random number generator with the last 2 bytes of the MAC address
   random_vars.shift_reg  = 0;
   random_vars.shift_reg += idmanager_getMyID(ADDR_16B)->addr_16b[0]*256;
   random_vars.shift_reg += idmanager_getMyID(ADDR_16B)->addr_16b[1];
}

uint16_t openrandom_get16b() {
   uint8_t  i;
   uint16_t random_value;
   random_value = 0;
   for(i=0;i<16;i++) {
      // Galois shift register
      // taps: 16 14 13 11
      // characteristic polynomial: x^16 + x^14 + x^13 + x^11 + 1
      random_value          |= (random_vars.shift_reg & 0x01)<<i;
      random_vars.shift_reg  = (random_vars.shift_reg>>1)^(-(int16_t)(random_vars.shift_reg & 1)&0xb400);
   }
   return random_value;
}

//=========================== private =========================================