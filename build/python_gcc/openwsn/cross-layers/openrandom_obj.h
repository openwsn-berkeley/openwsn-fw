/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:01:59.069390.
*/
#ifndef __OPENRANDOM_H
#define __OPENRANDOM_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenRandom
\{
*/

#include "openwsn_obj.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   uint16_t shift_reg;  // Galois shift register used to obtain a pseudo-random number
} random_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void openrandom_init(OpenMote* self);
uint16_t openrandom_get16b(OpenMote* self);

/**
\}
\}
*/

#endif
