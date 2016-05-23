/**
 * Author: Pere Tuset (peretuset@openmote.com)
           Jonathan Muñon (jonathan.munoz@inria.fr)
 * Date:   May 2016
 * Description: Register definitions for the Texas Instruments CC1200 radio chip.
 */

#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>
#include <stdbool.h>

//=========================== defines =========================================

#define GPIO_A_PORT                 ( 0 ) // GPIO_A
#define GPIO_B_PORT                 ( 1 ) // GPIO_B
#define GPIO_C_PORT                 ( 2 ) // GPIO_C
#define GPIO_D_PORT                 ( 3 ) // GPIO_D

//=========================== variables =======================================


//=========================== prototypes ======================================

typedef void (* gpio_callback_t)(void);

void gpio_init(void);

void gpio_config_output(uint8_t port, uint8_t pin);
void gpio_config_input(uint8_t port, uint8_t pin, uint8_t edge);

void gpio_register_callback(uint8_t port, uint8_t pin, gpio_callback_t callback);
void gpio_clear_callback(uint8_t port, uint8_t pin);

void gpio_enable_interrupt(uint8_t port, uint8_t pin);
void gpio_disable_interrupt(uint8_t port, uint8_t pin);

void gpio_on(uint8_t port, uint8_t pin);
void gpio_off(uint8_t port, uint8_t pin);
void gpio_toggle(uint8_t port, uint8_t pin);
bool gpio_read(uint8_t port, uint8_t pin);

//=========================== public ==========================================


//=========================== private =========================================


//=========================== callbacks =======================================


//=========================== interrupt handlers ==============================

#endif
