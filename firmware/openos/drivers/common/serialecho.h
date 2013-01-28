/**
\brief Declaration of the "serialecho" driver.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, January 2013.
*/

#ifndef __SERIALECHO_H
#define __SERIALECHO_H

/**
\addtogroup cross-layers
\{
\addtogroup SerialEcho
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== prototypes ======================================

void serialecho_init();
void serialecho_echo(uint8_t* but, uint8_t bufLen);

/**
\}
\}
*/

#endif