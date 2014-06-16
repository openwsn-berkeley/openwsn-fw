/* === INCLUDES ============================================================ */
#include "compiler.h"
#include "leds.h"
#include "samr21_xplained_pro.h"
#include "delay.h"

/* === GLOBALS ============================================================= */
uint8_t sync_led_status = 0;

/*
 * @brief leds_init Initialize the LED Pins and Default 
 *        state of each LED
 *
 * @param None
 *
 */
void leds_init(void)
{
 /* Nothing to be done here its already initialized in other place */
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
 LED_On(LED0);
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
 LED_Off(LED0);
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
 LED_Toggle(LED0);
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
