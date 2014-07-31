/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:25.384976.
*/
/**
\brief Definition of the "openserial" driver.

\author Min Ting <tingm417@gmail.com>, October 2012.
\author Fabien Chraim <chraim@eecs.berkeley.edu>, October 2012.
*/

#include "openhdlc_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

uint16_t crcIteration(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

//=========================== private =========================================
