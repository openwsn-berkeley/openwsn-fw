/**
 * Author: Pere Tuset (peretuset@openmote.com)
           Jonathan Muñon (jonathan.munoz@inria.fr)
 * Date:   May 2016
 * Description: Register definitions for the Texas Instruments CC1200 radio chip.
 */

#ifndef __CC1200_H
#define __CC1200_H

#include <stdint.h>
#include <stdbool.h>

//=========================== defines =========================================

typedef struct {
  uint16_t address;
  uint8_t value;
} cc1200_register_settings_t;

typedef struct {
  const cc1200_register_settings_t* register_settings;
  uint16_t size_of_register_settings;
  uint32_t chan_center_freq0;
  uint16_t chan_spacing;
  uint8_t min_channel;
  uint8_t max_channel;
  int8_t min_txpower;
  int8_t max_txpower;
  int8_t cca_threshold;
} cc1200_rf_cfg_t;

typedef struct {
} cc1200_status_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void cc1200_init(void);
bool cc1200_on(void);
bool cc1200_off(void);
void cc1200_idle(void);
void cc1200_reset(void);
void cc1200_configure(void);

void cc1200_set_power(int8_t power);
void cc1200_set_frequency(uint8_t freqyency);
void cc1200_set_channel(uint8_t channel);

void cc1200_load_packet(void);
void cc1200_transmit(void);

void cc1200_receive(void);
void cc1200_get_packet(void);

void cc1200_gpio0_interrupt(void);
void cc1200_gpio2_interrupt(void);
void cc1200_gpio3_interrupt(void);

//=========================== public ==========================================


//=========================== private =========================================


//=========================== callbacks =======================================


//=========================== interrupt handlers ==============================

#endif
