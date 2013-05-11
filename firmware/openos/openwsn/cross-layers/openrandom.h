#ifndef __OPENRANDOM_H
#define __OPENRANDOM_H

/**
\addtogroup helpers
\{
\addtogroup Random
\{
*/

#include "openwsn.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   uint16_t shift_reg;  // Galois shift register used to obtain a pseudo-random number
} random_vars_t;

//=========================== prototypes ======================================

void     openrandom_init();
uint16_t openrandom_get16b();

/**
\}
\}
*/

#endif