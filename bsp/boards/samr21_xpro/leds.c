/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

/* === INCLUDES ============================================================ */
#include "leds.h"
#include "delay.h"
#include "samr21_gpio.h"

/* === GLOBALS ============================================================= */
uint8_t sync_led_status = 0;

/** \name LED0 definitions
 *  @{ */
#define LED0_PIN                  PIN_PA19
#define LED0_ACTIVE               false
#define LED0_INACTIVE             !LED0_ACTIVE
/** @} */

/*
 * @brief leds_init Initialize the LED Pins and Default 
 *        state of each LED
 *
 * @param None
 *
 */
void leds_init(void)
{
	port_config(LED0_PIN, PORT_PIN_DIR_OUTPUT);
	port_pin_set_level(LED0_PIN, LED0_INACTIVE);
}

/*
 * @brief leds_error_on Error LED is set to ON 
 *
 * @param None
 *
 */
void leds_error_on(void)
{
 
}

/*
 * @brief leds_error_off Error LED is set to Off 
 *
 * @param None
 *
 */
void leds_error_off(void)
{
 
}

/*
 * @brief leds_error_toggle toggle Error LED 
 *
 * @param None
 *
 */
void leds_error_toggle(void)
{
 
}

/*
 * @brief leds_error_isOn get the Error LED Status 
 *
 * @param bool Error LED On Status
 *
 */
uint8_t leds_error_isOn(void)
{
 return true;
}

/*
 * @brief leds_error_blink Blink the Error LED 
 *
 * @param None
 *
 */
void leds_error_blink(void)
{
 //LED_Toggle(LED0);
 //delay_ms(20000);
}

/*
 * @brief leds_sync_on Set the Sync LED to On 
 *
 * @param None
 *
 */
void leds_sync_on(void)
{
 sync_led_status = 1;
 port_pin_set_level(LED0_PIN, LED0_ACTIVE);
}

/*
 * @brief leds_sync_off Set the Sync LED to Off 
 *
 * @param None
 *
 */
void leds_sync_off(void)
{
 sync_led_status = 0;
 port_pin_set_level(LED0_PIN, LED0_INACTIVE);
}

/*
 * @brief leds_sync_toggle Set the Sync LED to toggle 
 *
 * @param None
 *
 */
void leds_sync_toggle(void)
{
 sync_led_status = 2;
 port_pin_toggle(LED0_PIN);
}

/*
 * @brief leds_sync_isOn Get the Sync LED Status 
 *
 * @param uint8_t sync LED status
 *
 */
uint8_t leds_sync_isOn(void)
{
 return sync_led_status;
}

/*
 * @brief leds_radio_on Switch on the Radio LED 
 *
 * @param None
 *
 */
void leds_radio_on(void)
{

}

/*
 * @brief leds_radio_off Switch off the Radio LED 
 *
 * @param None
 *
 */
void leds_radio_off(void)
{

}

/*
 * @brief leds_radio_toggle Toggle the Radio LED 
 *
 * @param None
 *
 */
void leds_radio_toggle(void)
{

}

/*
 * @brief leds_radio_isOn Check the Status of the Radio LEd
 *
 * @param uint8_t bool
 *
 */
uint8_t leds_radio_isOn(void)
{
 return true;
}

/*
 * @brief leds_debug_on Set Debug LED to On
 *
 * @param None
 *
 */
void leds_debug_on(void)
{

}

/*
 * @brief leds_debug_off Set Debug LED to Off
 *
 * @param None
 *
 */
void leds_debug_off(void)
{

}

/*
 * @brief leds_debug_toggle Toggle the Debug LED
 *
 * @param None
 *
 */
void leds_debug_toggle(void)
{

}

/*
 * @brief leds_debug_isOn Check the Status of the Debug LED
 *
 * @param uint8_t bool
 *
 */
uint8_t leds_debug_isOn(void)
{
 return true;
}

/*
 * @brief leds_all_on Switch On all the LED
 *
 * @param None
 *
 */
void leds_all_on(void)
{

}

/*
 * @brief leds_all_off Switch Off all the LED
 *
 * @param None
 *
 */
void leds_all_off(void)
{

}

/*
 * @brief leds_all_toggle toggle all the LED
 *
 * @param None
 *
 */
void leds_all_toggle(void)
{

}

/*
 * @brief leds_circular_shift Do the circular shift using LED
 *
 * @param None
 *
 */
void leds_circular_shift(void)
{

}

/*
 * @brief leds_increment Change the LED status in linear
 *
 * @param None
 *
 */
void leds_increment(void)
{

}
