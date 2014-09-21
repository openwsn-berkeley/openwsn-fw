/**
\brief iot-lab_M3 definition of the "leds" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
*/

#include "stm32f10x_lib.h"
#include "leds.h"

//=========================== defines =========================================
#define LED_RED_PIN             (1<<2)
#define LED_GREEN_PIN           (1<<5)
#define LED_ORANGE_PIN          (1<<10)

//=========================== variables =======================================

//=========================== prototypes ======================================

void Delay(void);

//=========================== public ==========================================

void leds_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

// red
void leds_error_on()
{
  GPIOD->ODR &= ~LED_RED_PIN;
}
void leds_error_off()
{
  GPIOD->ODR |= LED_RED_PIN;
}
void leds_error_toggle()
{
  GPIOD->ODR ^= LED_RED_PIN;
}
uint8_t leds_error_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOD->ODR & LED_RED_PIN) != (u32)0)
  {
    bitstatus = 0x00;
  }
  else
  {
    bitstatus = 0x01;
  }
  return bitstatus;
}
void leds_error_blink() {}

// green
void leds_sync_on()
{
  GPIOB->ODR &= ~LED_GREEN_PIN;
}
void leds_sync_off()
{
  GPIOB->ODR |= LED_GREEN_PIN;
}
void leds_sync_toggle()
{
  GPIOB->ODR ^= LED_GREEN_PIN;
}
uint8_t leds_sync_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOB->ODR & LED_GREEN_PIN) != (u32)0)
  {
    bitstatus = 0x00;
  }
  else
  {
    bitstatus = 0x01;
  }
  return bitstatus;
}

// orange
void leds_radio_on()
{
  GPIOC->ODR &= ~LED_ORANGE_PIN;
}
void leds_radio_off()
{
  GPIOC->ODR |= LED_ORANGE_PIN;
}
void leds_radio_toggle()
{
  GPIOC->ODR ^= LED_ORANGE_PIN;
}
uint8_t leds_radio_isOn()
{
  u8 bitstatus = 0x00;
  if ((GPIOC->ODR & LED_ORANGE_PIN) != (u32)0)
  {
    bitstatus = 0x00;
  }
  else
  {
    bitstatus = 0x01;
  }
  return bitstatus;
}
// yellow
void leds_debug_on() {}
void leds_debug_off() {}
void leds_debug_toggle() {}
uint8_t leds_debug_isOn() {}

void leds_all_on()
{
  leds_error_on();
  leds_sync_on();
  leds_radio_on();
}
void leds_all_off()
{
  leds_error_off();
  leds_sync_off();
  leds_radio_off();
}
void leds_all_toggle()
{
  leds_error_toggle();
  leds_sync_toggle();
  leds_radio_toggle();
}

void leds_circular_shift()
{
  leds_error_toggle();
  Delay();
  leds_sync_toggle();
  Delay();
  leds_radio_toggle();
  Delay();
}

void leds_increment() {}

//=========================== private =========================================

void Delay(void)
{
  uint32_t i;
  for(i=0; i<0xfffff; i++);
}

