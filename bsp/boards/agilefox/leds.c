/**
\brief agilefox definition of the "leds" bsp module (based on openmoteSTM32 code).

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
\author Alaeddine Weslati <alaeddine.weslati@inria.fr>,  August 2013.
*/
#include "stm32f10x_lib.h"
#include "leds.h"

//=========================== defines =========================================
#define LED_RED_PIN             (1<<12)
#define LED_GREEN_PIN           (1<<10)

//=========================== variables =======================================

//=========================== prototypes ======================================

void Delay(void);

//=========================== public ==========================================

void leds_init(void) {
   // Enable GPIOB clock 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
   
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// red
void leds_error_on(void) {
   GPIOB->ODR &= ~LED_RED_PIN;
}
void leds_error_off(void) {
   GPIOB->ODR |= LED_RED_PIN;
}
void leds_error_toggle(void) {
   GPIOB->ODR ^= LED_RED_PIN;
}
uint8_t leds_error_isOn(void) {
   u8 bitstatus = 0x00;
   if ((GPIOB->ODR & LED_RED_PIN) != (u32)0) {
      bitstatus = 0x00;
   } else {
      bitstatus = 0x01;
   }
   return bitstatus;
}
void leds_error_blink(void) {
}

// green
void leds_sync_on(void) {
   GPIOB->ODR &= ~LED_GREEN_PIN;
}
void leds_sync_off(void) {
   GPIOB->ODR |= LED_GREEN_PIN;
}
void leds_sync_toggle(void) {
   GPIOB->ODR ^= LED_GREEN_PIN;
}
uint8_t leds_sync_isOn(void) {
   u8 bitstatus = 0x00;
   if ((GPIOB->ODR & LED_GREEN_PIN) != (u32)0) {
      bitstatus = 0x00;
   } else {
      bitstatus = 0x01;
   }
   return bitstatus;
}

// orange
void leds_radio_on(void) {
}
void leds_radio_off(void) {
}
void leds_radio_toggle(void) {
}
uint8_t leds_radio_isOn(void) {
   return FALSE;
}

// yellow
void leds_debug_on(void) {
}
void leds_debug_off(void) {
}
void leds_debug_toggle(void) {
}
uint8_t leds_debug_isOn(void) {
   return FALSE;
}

void leds_all_on(void) {
   leds_error_on();
   leds_sync_on();
}
void leds_all_off(void) {
   leds_error_off();
   leds_sync_off();
}
void leds_all_toggle(void) {
   leds_error_toggle();
   leds_sync_toggle();
}

void leds_circular_shift(void) {
   GPIOB->ODR ^= LED_RED_PIN;
   Delay();
   GPIOB->ODR ^= LED_RED_PIN;
   Delay();
   GPIOB->ODR ^= LED_GREEN_PIN;
   Delay();
   GPIOB->ODR ^= LED_GREEN_PIN;
   Delay();
}

void leds_increment(void) {
}

//=========================== private =========================================

void Delay(void){
  unsigned long ik;
  for(ik=0;ik<0xffff8;ik++);
}
