/**
\brief openmoteSTM32 definition of the RCC.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#include "stm32f10x_conf.h"
#include "board_info.h"
#include "board.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void RCC_Configuration(void){
    // RCC system reset(for debug purpose)
    RCC_DeInit();
    
    
 
	// Enable the external oscillator  (12MHz)
	RCC_HSEConfig(RCC_HSE_ON);
    if( RCC_WaitForHSEStartUp() != SUCCESS){
		// if the external oscillator didn't start reset the board
		// Alternatively we could decide to use the internal oscillator
		// and run at a lower speed.
		board_reset();
	}

    // Enable Prefetch Buffer
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    // 2 wait states
    FLASH_SetLatency(FLASH_Latency_2);
	
	// Enable the RTC clock from external 32.768kHz Xtal
	RCC_LSEConfig(RCC_LSE_ON);
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	// enable the prediv1 clock source to be HSE
	RCC_PREDIV1Config(RCC_PREDIV1_Source_HSE, RCC_PREDIV1_Div1);
    // PLLCLK = 12MHz/1 * 6 = 72 MHz
    RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_6);
    
    // Enable PLL 
    RCC_PLLCmd(ENABLE);

    // Wait till PLL is ready 
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
  
    // Select PLL as system clock source 
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
    // Wait till PLL is used as system clock source 
    while(RCC_GetSYSCLKSource() != 0x08);
	
   // HCLK = SYSCLK 
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    // PCLK2 = HCLK
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    // PCLK1 = HCLK/2 
    RCC_PCLK1Config(RCC_HCLK_Div2);
     
    //enable AFIO 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

//when wakeup by alarm, configure rcc
void RCC_Wakeup(void){
    //enable PLL
    RCC_PLLCmd(ENABLE);

    // Wait till PLL is ready 
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    // Select PLL as system clock source 
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
    // Wait till PLL is used as system clock source 
    while(RCC_GetSYSCLKSource() != 0x08);
}
