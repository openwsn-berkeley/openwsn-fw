//includes 
#include "gpio.h"  //for gpio driver functions
#include "led_mimsy.h"
#include "hw_memmap.h" //contains GPIO_C_BASE and other hardware mappings

#define LED_BASE  GPIO_C_BASE //memory base for gpio bank c, which has all the leds
void
mimsyLedInit(void)
{
    //
    // Set GPIO pins as output low
    //
    GPIOPinTypeGPIOOutput(LED_BASE, RED_LED | GREEN_LED);
    GPIOPinWrite(LED_BASE, RED_LED|GREEN_LED, 0);
}

void
mimsyLedSet(uint8_t ui8Leds)
{
    //
    // Turn on specified LEDs
    //
    GPIOPinWrite(LED_BASE, ui8Leds, ui8Leds);
}

/**************************************************************************//**
* @brief    This function clears LED(s) specified by \e ui8Leds.
*           This function assumes that LED pins have been initialized by,
*           for example, bspLedInit().
*
* @param    ui8Leds      is an ORed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
mimsyLedClear(uint8_t ui8Leds)
{
    //
    // Turn off specified LEDs
    //
    GPIOPinWrite(LED_BASE, ui8Leds, 0);
}

void
mimsyLedToggle(uint8_t ui8Leds)
{
    //
    // Get current pin values of selected bits
    //
    uint32_t ui32Toggle = GPIOPinRead(LED_BASE, ui8Leds);

    //
    // Invert selected bits
    //
    ui32Toggle = (~ui32Toggle) & ui8Leds;

    //
    // Set GPIO
    //
    GPIOPinWrite(LED_BASE, ui8Leds, ui32Toggle);
}
