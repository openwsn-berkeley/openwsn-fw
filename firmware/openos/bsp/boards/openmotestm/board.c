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
#include "rtc_timer.h"
#include "rcc.h"
#include "nvic.h"
#include "debugpins.h"


//=========================== main ============================================

extern int mote_main(void);
int main(void) {
  
  
  
   return mote_main();
}

//=========================== public ==========================================

void board_init()
{
  
    RCC_Configuration();//Configure rcc
    NVIC_Configuration();//configure NVIC and Vector Table
    
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
  
    //Configures EXTI line 10 to generate an interrupt on rising edge
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
      
    //Configure NVIC: Preemption Priority = 2 and Sub Priority = 0
    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                     = EXTI15_10_IRQChannel; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);

//    //configure sheduler interrupt by EXTI_Line1
//    EXTI_ClearITPendingBit(EXTI_Line1);
//    EXTI_InitStructure.EXTI_Line    = EXTI_Line1;
//    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
//    EXTI_Init(&EXTI_InitStructure);
//    
//    //Configure NVIC: Preemption Priority = 2 and Sub Priority = 3
//    NVIC_InitStructure.NVIC_IRQChannel                     = EXTI1_IRQChannel; 
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 2;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 3; 
//    NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE; 
//    NVIC_Init(&NVIC_InitStructure);
    
    // initialize board
    leds_init();
    uart_init();
    spi_init();
    bsp_timer_init();
    radio_init();
    radiotimer_init();
    debugpins_init();
}

void board_sleep() {
  /*
    PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
    
    //configure system clock after wakeup from stop mode
    
    ErrorStatus HSEStartUpStatus;
    
    //turn on HSE
    RCC_HSEConfig(RCC_HSE_ON);
	
    //Wait till HSE is ready and if Time out is reached exit 
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if(HSEStartUpStatus == SUCCESS)
    {
	//enable PLL
	RCC_PLLCmd(ENABLE);
        
	//Wait till PLL is ready 
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{}

	//Select PLL as system clock source
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	//Wait till PLL is used as system clock source
	while(RCC_GetSYSCLKSource() != 0x08)
	{}
    }
    */
}

void board_reset()
{
  
}