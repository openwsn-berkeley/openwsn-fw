#ifndef __OPENRANDOM_H
#define __OPENRANDOM_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenRandom
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

typedef struct {
   uint16_t shift_reg;  // Galois shift register used to obtain a pseudo-random number
} random_vars_t;

//=========================== prototypes ======================================

void     openrandom_init(void);
uint16_t openrandom_get16b(void);

/**
\}
\}
*/

#endif
