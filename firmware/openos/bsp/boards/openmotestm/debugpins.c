/**
\brief opemnstm32 definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/
#include "stm32f10x_lib.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
  
    GPIO_InitTypeDef GPIO_InitStructure;
    // Configure PC.4 and PC.5 as Output push-pull 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  /*
   P4DIR |=  0x20;      // frame
   P4DIR |=  0x02 ;     // slot
   P4DIR |=  0x04;      // fsm
   P4DIR |=  0x08;      // task
   P4DIR |=  0x10;      // isr
   P1DIR |=  0x02;      // radio
   
   debugpins_frame_clr();
   debugpins_slot_clr();
   debugpins_fsm_clr();
   debugpins_task_clr();
   debugpins_isr_clr();
   debugpins_radio_clr();
  */
}

// A.4
void debugpins_frame_toggle() {
  GPIOC->ODR ^= 0X0010;
}
void debugpins_frame_clr() {
  GPIOC->ODR &= ~0X0010;
}
void debugpins_frame_set() {
  GPIOC->ODR |=  0X0010;
}

// PA.5
void debugpins_slot_toggle() {
  GPIOC->ODR ^=  0X0020;
}
void debugpins_slot_clr() {
  GPIOC->ODR &= ~0X0020;
}
void debugpins_slot_set() {
  GPIOC->ODR |=  0X0020;
}

// P4.2
void debugpins_fsm_toggle() {
//   P4OUT ^=  0x04;
}
void debugpins_fsm_clr() {
//   P4OUT &= ~0x04;
}
void debugpins_fsm_set() {
//   P4OUT |=  0x04;
}

// P4.3
void debugpins_task_toggle() {
//   P4OUT ^=  0x08;
}
void debugpins_task_clr() {
//   P4OUT &= ~0x08;
}
void debugpins_task_set() {
//   P4OUT |=  0x08;
}

// P4.4
void debugpins_isr_toggle() {
//   P4OUT ^=  0x10;
}
void debugpins_isr_clr() {
//   P4OUT &= ~0x10;
}
void debugpins_isr_set() {
//   P4OUT |=  0x10;
}

// P1.1
void debugpins_radio_toggle() {
//   P1OUT ^=  0x02;
}
void debugpins_radio_clr() {
//   P1OUT &= ~0x02;
}
void debugpins_radio_set() {
//   P1OUT |=  0x02;
}

void    leds_toggle_2x(){
}

void    leds_toggle_4x(){
}