/*
 * Drivers for the sensitive accelerometer and temperature sensor of the GINA2.2b/c board.
 *
 * Author:
 * Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
 */

#ifndef __ADC_CHANNEL_H
#define __ADC_CHANNEL_H

#include "msp430x26x.h"
#include "stdint.h"

//prototypes
void ADC_init();
void ADC_disable();
void ADC_get_config();
void ADC_getvoltage(uint16_t* spaceToWrite);

#endif
