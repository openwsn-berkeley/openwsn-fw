/**
 * Author: Pere Tuset (peretuset@openmote.com)
           Jonathan Muñon (jonathan.munoz@inria.fr)
 * Date:   May 2016
 * Description: Register definitions for the Texas Instruments CC1200 radio chip.
 */

#ifndef __CC1200_ARCH_H
#define __CC1200_ARCH_H

//=========================== defines =========================================

#include <stdint.h>
#include <stdbool.h>

#include "cc1200.h"

//=========================== variables =======================================


//=========================== prototypes ======================================

void cc1200_arch_init(void);

void cc1200_arch_spi_select(void);
void cc1200_arch_spi_deselect(void);

void cc1200_arch_clock_delay(uint32_t microseconds);

uint8_t cc1200_arch_spi_rw_byte(uint8_t byte);
void cc1200_arch_spi_rw(uint8_t* read, uint8_t* write, uint16_t length);

void cc1200_arch_gpio0_setup(bool rising);
void cc1200_arch_gpio0_enable(void);
void cc1200_arch_gpio0_disable(void);
bool cc1200_arch_gpio0_read(void);
void cc1200_arch_gpio0_interrupt(void);

void cc1200_arch_gpio2_setup(bool rising);
void cc1200_arch_gpio2_enable(void);
void cc1200_arch_gpio2_disable(void);
bool cc1200_arch_gpio2_read(void);
void cc1200_arch_gpio2_interrupt(void);

void cc1200_arch_gpio3_setup(bool rising);
void cc1200_arch_gpio3_enable(void);
void cc1200_arch_gpio3_disable(void);
bool cc1200_arch_gpio3_read(void);
void cc1200_arch_gpio3_interrupt(void);

//=========================== public ==========================================


//=========================== private =========================================


//=========================== callbacks =======================================


//=========================== interrupt handlers ==============================


#endif
