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

//=========================== prototypes ======================================

void    leds_init(void);

void    leds_error_on(void);
void    leds_error_off(void);
void    leds_error_toggle(void);
uint8_t leds_error_isOn(void);
void    leds_error_blink(void);

void    leds_radio_on(void);
void    leds_radio_off(void);
void    leds_radio_toggle(void);
uint8_t leds_radio_isOn(void);

void    leds_sync_on(void);
void    leds_sync_off(void);
void    leds_sync_toggle(void);
uint8_t leds_sync_isOn(void);

void    leds_debug_on(void);
void    leds_debug_off(void);
void    leds_debug_toggle(void);
uint8_t leds_debug_isOn(void);

void    leds_all_on(void);
void    leds_all_off(void);
void    leds_all_toggle(void);

void    leds_circular_shift(void);
void    leds_increment(void);

/**
\}
\}
*/

#endif
