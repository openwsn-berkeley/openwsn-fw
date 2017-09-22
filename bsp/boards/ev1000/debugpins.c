/**
\brief opemnstm32 definition of the "debugpins" bsp module.

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, February 2012.
*/
#include "stm32f10x_lib.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    // Configure PC.0, PC.1 PC.2, PC.3, PC.4 and PC.5 as Output push-pull
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    GPIOC->ODR |= 0X0020;      // frame, PC.5
    GPIOC->ODR |= 0X0010;      // slot,  PC.4
    GPIOC->ODR |= 0X0008;      // fsm,   PC.3
    GPIOC->ODR |= 0X0004;      // task,  PC.2
    GPIOC->ODR |= 0X0002;      // isr,   PC.1
    GPIOC->ODR |= 0X0001;      // radio, PC.0
   
    debugpins_frame_clr();
    debugpins_slot_clr();
    debugpins_fsm_clr();
    debugpins_task_clr();
    debugpins_isr_clr();
    debugpins_radio_clr();
}

// PC.5
void debugpins_frame_toggle(void){
    
    GPIOC->ODR ^= 0X0020;
}
void debugpins_frame_clr(void) {
    
    GPIOC->ODR &= ~0X0020;
}
void debugpins_frame_set(void) {
    
    GPIOC->ODR |=  0X0020;
}   

// PC.4
void debugpins_slot_toggle(void) {
    
    GPIOC->ODR ^=  0X0010;
}
void debugpins_slot_clr(void) {
    
    GPIOC->ODR &= ~0X0010;
}
void debugpins_slot_set(void) {
    
    GPIOC->ODR |=  0X0010;
}

// PC.3
void debugpins_fsm_toggle(void) {
    
    GPIOC->ODR ^=  0X0008;
}
void debugpins_fsm_clr(void) {
    
    GPIOC->ODR &= ~0X0008;
}
void debugpins_fsm_set(void) {
    
    GPIOC->ODR |=  0X0008;
}

// PC.2
void debugpins_task_toggle(void) {
    
    GPIOC->ODR ^=  0X0004;
}
void debugpins_task_clr(void) {
    
    GPIOC->ODR &= ~0X0004;
}
void debugpins_task_set(void) {
    
    GPIOC->ODR |= 0X0004;
}

// PC.1
void debugpins_isr_toggle(void) {
    
    GPIOC->ODR ^=  0X0002;
}
void debugpins_isr_clr(void) {
    
    GPIOC->ODR &= ~0X0002;
}
void debugpins_isr_set(void) {
    
    GPIOC->ODR |= 0X0002;
}

// PC.0
void debugpins_radio_toggle(void) {
    
    GPIOC->ODR ^=  0X0001;
}
void debugpins_radio_clr(void) {
    
    GPIOC->ODR &= ~0X0001;
}
void debugpins_radio_set(void) {
    
    GPIOC->ODR |=  0X0001;
}
