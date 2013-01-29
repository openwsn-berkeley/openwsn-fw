/**
\brief Definition of the "openserial" driver.

\author Min Ting, October 2012.
\author Fabien Chraim <chraim@eecs.berkeley.edu>, October 2012.
*/

#include "hdlcserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

uint16_t crcIteration(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

//=========================== private =========================================
