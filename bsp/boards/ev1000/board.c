/**
\brief ev1000 definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
\author Jean-Michel Rubillon <jmrubillon@theiet.org>, April 2017
*/
#include "stm32f10x_conf.h"
#include "board.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "sctimer.h"
#include "rcc.h"
#include "nvic.h"
#include "debugpins.h"

//=========================== variable ========================================

//=========================== private =========================================

//Configures the different GPIO ports as Analog Inputs.
void GPIO_Config_ALL_AIN(void);
// configure the hard fault exception
void board_enableHardFaultExceptionHandler(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}

//=========================== public ==========================================

void board_init(void){
    uint32_t i;
    //Configure rcc
    RCC_Configuration();
    //configure NVIC and Vector Table
    NVIC_Configuration();
    
    // configure hardfault exception
    board_enableHardFaultExceptionHandler();
    
    //configure ALL GPIO to AIN to get lowest power
    GPIO_Config_ALL_AIN();
    
    GPIO_InitTypeDef  GPIO_InitStructure;
  
    //enable GPIOA,GPIOB
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
    
    //Configure PB.00 as DW_WUP pin of RF
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    //Configure PA.00 as nRST pin of RF
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // Reset the DW1000 chip
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	for(i=0;i<0x0FFFFL; i++); // insert a delay before taking the chip out of reset
    // The nRST pin is a floating input.
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	for(i=0;i<0x00FFFL; i++); // insert a delay to let the DW1000 initialise itself
    
    // Configure PB.05 as input floating (EXTI Line10) DW1000 IRQ
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIOB->ODR |= 0X0010;//set low
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);//Connect EXTI Line5 to PB.05
    EXTI_ClearITPendingBit(EXTI_Line5);

    //Configures EXTI line 5 to generate an interrupt on rising edge
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
    
    // initialize board
    leds_init();
    debugpins_init();
    uart_init();
    spi_init();
    sctimer_init();
    radio_init();
    //enable nvic for the radio
    NVIC_radio();
}

void board_sleep(void) {
    DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
    // Enable PWR and BKP clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Desable the SRAM and FLITF clock in sleep mode
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_FLITF, DISABLE);
    // enter sleep mode
    __WFI();
}



void board_reset(void){
    NVIC_SystemReset();
}

// ========================== private =========================================

/**
  * @brief  Configures the different GPIO ports as Analog Inputs.
  * @param  None
  * @retval : None
  */
void GPIO_Config_ALL_AIN(void){
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOD and GPIOE clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB 
                         | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD 
                         | RCC_APB2Periph_AFIO, ENABLE);

    /* PA  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
      /* PB  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
      /* PC  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
        /* PD  */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void board_enableHardFaultExceptionHandler(void){
    // Configures:
    //    bit9. stack alignment on exception entry 
    //    bit4. enables faulting
    //    bit3. unaligned access traps
    SCB->CCR = 0x00000210;
}

 

