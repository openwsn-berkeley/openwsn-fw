#include "gpio.h"  //for gpio driver functions
#define RED_LED  GPIO_PIN_4
#define GREEN_LED  GPIO_PIN_7

extern void mimsyLedInit(void);
extern void mimsyLedSet(uint8_t ui8Leds);
extern void mimsyLedClear(uint8_t ui8Leds);
extern void mimsyLedToggle(uint8_t ui8Leds);
