/**
\brief GINA-specific definition of the "board" bsp module.

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
//#include "radiotimer.h"
#include "rtc_timer.h"

// Private typedef 变量-----------------------------------------------------------*/
unsigned char read_temp;
// Private define 定义------------------------------------------------------------*/

// Private macro 宏定义-----------------------------------------------------------*/
// Private variables 变量---------------------------------------------------------*/
// Private function prototypes 函数原型-------------------------------------------*/
// Private functions 函数---------------------------------------------------------*/

GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;

void NVIC_Configuration(void);
void SysTick_Config(void);
void RCC_Configuration(void);

 void delay(void)
  {
    unsigned long ii; 
    for(ii=0;ii<0xffff8;ii++);
  }
 
/* Private functions ---------------------------------------------------------*/

//=========================== main ============================================

extern int mote_main();

int main() {
   return mote_main();
}

//=========================== public ==========================================

void board_init()
{
      
    RCC_Configuration();//配置系统时钟
    NVIC_Configuration();//配置  NVIC 和 Vector Table 

    //for dante
    EXTI_InitTypeDef  EXTI_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       //SLP_TR
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;// configure as output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;       //RST
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;// configure as output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    GPIO_SetBits(GPIOC, GPIO_Pin_2);
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0);//定义PC0为外部中断
  
    //enable RF interrupt
    // EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;   //外部中断0
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
      
    //定义外部中断0中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);

   // initialize board
   // board_init();
   // gpio_init();
   leds_init();
   uart_init();
   spi_init();
   bsp_timer_init();
   radiotimer_init();
   radio_init();
  /*
    // setup clock speed
    RCC_Configuration();   //配置系统时钟，设置系统时钟为72M 
    NVIC_Configuration();  //配置中断


    //for dante
    EXTI_InitTypeDef  EXTI_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
  
    GPIO_InitStructure.GPIO_Pin      = GPIO_Pin_1;       //SLP_TR
    GPIO_InitStructure.GPIO_Mode     = GPIO_Mode_Out_PP;// configure as output
    GPIO_InitStructure.GPIO_Speed    = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Pin      = GPIO_Pin_2;       //RST
    GPIO_InitStructure.GPIO_Mode     = GPIO_Mode_Out_PP;// configure as output
    GPIO_InitStructure.GPIO_Speed    = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    GPIO_SetBits(GPIOC, GPIO_Pin_2);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0);//定义PC0为外部中断
  
    //enable RF interrupt
    //EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line     = EXTI_Line0;   //外部中断0
    EXTI_InitStructure.EXTI_Mode     = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger  = EXTI_Trigger_Rising;  //上升沿触发
    EXTI_InitStructure.EXTI_LineCmd  = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);

    //定义外部中断0中断优先级
    NVIC_InitStructure.NVIC_IRQChannel                     = EXTI0_IRQChannel; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);


    //initialize bsp modules
    leds_init();
    uart_init();
    spi_init();

    bsp_timer_init();
    radiotimer_init();
    radio_init();
    
    leds_all_off();
    */
}

void board_sleep() {
   //__bis_SR_register(GIE+LPM3_bits);             // sleep, but leave ACLK on
}

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{   
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

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

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
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
}

/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
* Output         : None
* Return         : None
*******************************************************************************/

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
