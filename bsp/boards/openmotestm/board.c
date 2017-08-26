/**
\brief openmoteSTM32 definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#include "stm32f10x_lib.h"
#include "board.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "bsp_timer.h"
#include "radio.h"
#include "radiotimer.h"
#include "rcc.h"
#include "nvic.h"
#include "debugpins.h"

//=========================== variable ========================================

//=========================== private =========================================

//Configures the different GPIO ports as Analog Inputs.
void GPIO_Config_ALL_AIN(void);
// configure the hard fault exception
void board_enableHardFaultExceptionHandler();

//=========================== main ============================================

extern int mote_main();

int main() {
    return mote_main();
}

//=========================== public ==========================================

void board_init(){
    
    //Configure rcc
    RCC_Configuration();
    //configure NVIC and Vector Table
    NVIC_Configuration();
    
    // configure hardfault exception
    board_enableHardFaultExceptionHandler();
    
    //configure ALL GPIO to AIN to get lowest power
    GPIO_Config_ALL_AIN();
    
    GPIO_InitTypeDef  GPIO_InitStructure;
  
    //enable GPIOB, Clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
    
    //Configure PB.01 as SLP_TR pin of RF
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    //Configure PB.11 as RST pin of RF
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //set /RST pin high(never reset)
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
    
    // Configure PB.10 as input floating (EXTI Line10)
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIOB->ODR |= 0X0400;//set low
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);//Connect EXTI Line10 to PB.10
    EXTI_ClearITPendingBit(EXTI_Line10);

    //Configures EXTI line 10 to generate an interrupt on rising edge
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
    
    // initialize board
    leds_init();
    uart_init();
    spi_init();
    bsp_timer_init();
    radio_init();
    radiotimer_init();
    debugpins_init();
    //enable nvic for the radio
    NVIC_radio();
}

void board_sleep() {
    DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
    // Enable PWR and BKP clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Desable the SRAM and FLITF clock in sleep mode
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_FLITF, DISABLE);
    // enter sleep mode
    __WFI();
}



void board_reset(){
    NVIC_GenerateSystemReset();
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

void board_enableHardFaultExceptionHandler(){
    // Configures:
    //    bit9. stack alignment on exception entry 
    //    bit4. enables faulting
    //    bit3. unaligned access traps
    SCB->CCR = 0x00000210;
}

 

