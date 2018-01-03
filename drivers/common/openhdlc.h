/**
\brief Declaraion of the "openserial" driver.

\author Min Ting <tingm417@gmail.com>, October 2012.
\author Fabien Chraim <chraim@eecs.berkeley.edu>, October 2012.
*/

#ifndef __OPENHDLC_H
#define __OPENHDLC_H

#include "opendefs.h"

/**
\addtogroup drivers
\{
\addtogroup HDLC
\{
*/

//=========================== define ==========================================

#define HDLC_FLAG            0x7e
#define HDLC_ESCAPE          0x7d
#define HDLC_ESCAPE_MASK     0x20
#define HDLC_CRCINIT         0xffff
#define HDLC_CRCGOOD         0xf0b8

//this table is used to expedite execution (at the expense of memory usage)
const uint16_t fcstab[256];

//=========================== typedef =========================================

//=========================== prototypes ======================================

uint16_t crcIteration(uint16_t crc, uint8_t byte);

/**
\}
\}
*/

#endif