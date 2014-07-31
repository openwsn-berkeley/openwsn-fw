/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:20.375878.
*/
#ifndef __EUI64_H
#define __EUI64_H

/**
\addtogroup BSP
\{
\addtogroup eui64
\{

\brief Cross-platform declaration "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include <stdint.h>
 
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void eui64_get(OpenMote* self, uint8_t* addressToWrite);

/**
\}
\}
*/

#endif