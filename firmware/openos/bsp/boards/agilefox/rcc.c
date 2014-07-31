/**
\brief openmoteSTM32 definition of the RCC.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
#include "stm32f10x_lib.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;
  
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
      /* Enable Prefetch Buffer */
      FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

      /* Flash 2 wait state */
      FLASH_SetLatency(FLASH_Latency_2);
 	
      /* HCLK = SYSCLK */
      RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
      /* PCLK2 = HCLK */
      RCC_PCLK2Config(RCC_HCLK_Div1); 

      /* PCLK1 = HCLK/2 */
      RCC_PCLK1Config(RCC_HCLK_Div2);

      /* PLLCLK = 16MHz / 2 * 9 = 72 MHz */
      RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_9);

      /* Enable PLL */ 
      RCC_PLLCmd(ENABLE);

      /* Wait till PLL is ready */
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
      {
      }

      /* Select PLL as system clock source */
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

      /* Wait till PLL is used as system clock source */
      while(RCC_GetSYSCLKSource() != 0x08)
      {
      }
    }
    
    //enable AFIO 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

//when wakeup by alarm, configure rcc
void RCC_Wakeup(void)
{
    ErrorStatus HSEStartUpStatus;

    //enable HSE
    RCC_HSEConfig(RCC_HSE_ON);
    //Wait till HSE is ready
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if(HSEStartUpStatus == SUCCESS)
    {
      //enable PLL
      RCC_PLLCmd(ENABLE);
      //Wait till PLL is ready
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
      {}

      // Select PLL as system clock source
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

      //Wait till PLL is used as system clock source
      while(RCC_GetSYSCLKSource() != 0x08)
      {}
     }
}