/**
\brief agilefox definition of the "debugpins" bsp module (based on openmoteSTM32 code).

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Alaeddine Weslati <alaeddine.weslati@inria.fr>,  August 2013.
*/
#include "stm32f10x_lib.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
  
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
//    
//    GPIO_InitTypeDef GPIO_InitStructure;
//    // Configure PC.0, PC.1 and PC.5 as Output push-pull 
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &GPIO_InitStructure);
//    
//    // Configure PA.5, PA.6 and PA.7 as Output push-pull 
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//  
//     GPIOC->ODR |= 0X0020;      // frame, PC.5
//     GPIOA->ODR |= 0X0080;      // slot, PA.7
//     GPIOA->ODR |= 0X0020;      // fsm, PA.5
//     GPIOA->ODR |= 0X0040;      // task, PA.6
//     GPIOC->ODR |= 0X0002;      // isr, PC.1
//     GPIOC->ODR |= 0X0001;      // radio, PC.0
//   
//     debugpins_frame_clr();
//     debugpins_slot_clr();
//     debugpins_fsm_clr();
//     debugpins_task_clr();
//     debugpins_isr_clr();
//     debugpins_radio_clr();
}

// PC.5
void debugpins_frame_toggle() {
  //GPIOC->ODR ^= 0X0020;
}
void debugpins_frame_clr() {
  //GPIOC->ODR &= ~0X0020;
}
void debugpins_frame_set() {
  //GPIOC->ODR |=  0X0020;
}

// PA.7
void debugpins_slot_toggle() {
  //GPIOA->ODR ^=  0X0080;
}
void debugpins_slot_clr() {
  //GPIOA->ODR &= ~0X0080;
}
void debugpins_slot_set() {
  //GPIOA->ODR |=  0X0080;
}

// PA.5
void debugpins_fsm_toggle() {
  //GPIOA->ODR ^=  0X0020;
}
void debugpins_fsm_clr() {
  //GPIOA->ODR &= ~0X0020;
}
void debugpins_fsm_set() {
  //GPIOA->ODR |=  0X0020;
}

// PA.6
void debugpins_task_toggle() {
  //GPIOA->ODR ^=  0X0040;
}
void debugpins_task_clr() {
  //GPIOA->ODR &= ~0X0040;
}
void debugpins_task_set() {
  //GPIOA->ODR |= 0X0040;
}

// PC.1
void debugpins_isr_toggle() {
  //GPIOC->ODR ^=  0X0002;
}
void debugpins_isr_clr() {
  //GPIOC->ODR &= ~0X0002;
}
void debugpins_isr_set() {
  //GPIOC->ODR |= 0X0002;
}

// PC.0
void debugpins_radio_toggle() {
  //GPIOC->ODR ^=  0X0001;
}
void debugpins_radio_clr() {
  //GPIOC->ODR &= ~0X0001;
}
void debugpins_radio_set() {
  //GPIOC->ODR |=  0X0001;
}
