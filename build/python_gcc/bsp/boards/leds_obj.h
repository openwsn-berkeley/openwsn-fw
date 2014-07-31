/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:31.593907.
*/
#ifndef __LEDS_H
#define __LEDS_H

/**
\addtogroup BSP
\{
\addtogroup leds
\{

\brief Cross-platform declaration "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "stdint.h"
 
//=========================== define ==========================================

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//=========================== typedef =========================================

//=========================== variables =======================================

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

void leds_init(OpenMote* self);

void leds_error_on(OpenMote* self);
void leds_error_off(OpenMote* self);
void leds_error_toggle(OpenMote* self);
uint8_t leds_error_isOn(OpenMote* self);
void leds_error_blink(OpenMote* self);

void leds_radio_on(OpenMote* self);
void leds_radio_off(OpenMote* self);
void leds_radio_toggle(OpenMote* self);
uint8_t leds_radio_isOn(OpenMote* self);

void leds_sync_on(OpenMote* self);
void leds_sync_off(OpenMote* self);
void leds_sync_toggle(OpenMote* self);
uint8_t leds_sync_isOn(OpenMote* self);

void leds_debug_on(OpenMote* self);
void leds_debug_off(OpenMote* self);
void leds_debug_toggle(OpenMote* self);
uint8_t leds_debug_isOn(OpenMote* self);

void leds_all_on(OpenMote* self);
void leds_all_off(OpenMote* self);
void leds_all_toggle(OpenMote* self);

void leds_circular_shift(OpenMote* self);
void leds_increment(OpenMote* self);

/**
\}
\}
*/

#endif
