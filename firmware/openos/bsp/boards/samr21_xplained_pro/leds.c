


#include "compiler.h"
#include "leds.h"
#include "samr21_xplained_pro.h"
#include "delay.h"

uint8_t sync_led_status = 0;

/* Initialize the LED Pins and Default state of each */
void leds_init(void)
{
 /* Nothing to be done here its already initialized in other place */
}

void leds_error_on(void)
{
 
}

void leds_error_off(void)
{
 
}

void leds_error_toggle(void)
{
 
}

uint8_t leds_error_isOn(void)
{
 return true;
}

void leds_error_blink(void)
{
 //LED_Toggle(LED0);
 //delay_ms(20000);
}

void leds_sync_on(void)
{
 sync_led_status = 1;
 LED_On(LED0);
}

void leds_sync_off(void)
{
 sync_led_status = 0;
 LED_Off(LED0);
}

void leds_sync_toggle(void)
{
 sync_led_status = 2;
 LED_Toggle(LED0);
}

uint8_t leds_sync_isOn(void)
{
 return sync_led_status;
}

void leds_radio_on(void)
{

}

void leds_radio_off(void)
{

}

void leds_radio_toggle(void)
{

}

uint8_t leds_radio_isOn(void)
{
 return true;
}

void leds_debug_on(void)
{

}

void leds_debug_off(void)
{

}

void leds_debug_toggle(void)
{

}

uint8_t leds_debug_isOn(void)
{
 return true;
}

void leds_all_on(void)
{

}

void leds_all_off(void)
{

}

void leds_all_toggle(void)
{

}

void leds_circular_shift(void)
{

}

void leds_increment(void)
{

}
