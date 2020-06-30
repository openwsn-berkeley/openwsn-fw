#ifndef OPENWSN_OPENRANDOM_H
#define OPENWSN_OPENRANDOM_H

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

void openrandom_init(void);

uint16_t openrandom_get16b(void);

uint16_t openrandom_getRandomizePeriod(uint16_t period, uint16_t range);

/**
\}
\}
*/

#endif /* OPENWSN_OPENRANDOM_H */
