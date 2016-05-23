/**
 * Author: Pere Tuset (peretuset@openmote.com)
           Jonathan Muñon (jonathan.munoz@inria.fr)
 * Date:   May 2016
 * Description: Register definitions for the Texas Instruments CC1200 radio chip.
 */

#ifndef __CC1200_ARCH_H
#define __CC1200_ARCH_H

//=========================== defines =========================================

#include "cc1200.h"

//=========================== variables =======================================


//=========================== prototypes ======================================

void cc1200_arch_init(void);

void cc1200_arch_spi_select(void);
void cc1200_arch_spi_deselect(void);

//=========================== public ==========================================


//=========================== private =========================================


//=========================== callbacks =======================================


//=========================== interrupt handlers ==============================


#endif
