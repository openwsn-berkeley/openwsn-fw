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
#include "board_info.h"
#include "openserial.h"


//=========================== main ============================================

extern int mote_main(void);
int main(void) {
   return mote_main();
}
//=========================== defines =========================================

void uart_reConfiguration();
void bsp_timer_reConfiguration();
void spi_reConfiguration();
void radiotimer_reConfiguration();
void radio_reConfiguration();

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
  
//    DBGMCU_Config(DBGMCU_STOP, ENABLE);
//
//    // Enable PWR and BKP clock
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
//    // Desable the SRAM and FLITF clock in Stop mode
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM|RCC_AHBPeriph_FLITF, DISABLE);
//    uint16_t temp = TIM_GetCounter(TIM2);
//
//    PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
//    
//    RCC_Configuration();//Configure rcc
//    
//    uart_reConfiguration();
//    bsp_timer_reConfiguration();
}

void board_reset(){
}

void uart_reConfiguration()
{
    // Configure PA.09 as alternate function push-pull
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    //Configure PA.10 as input floating 
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void bsp_timer_reConfiguration()
{
    //Configure TIM2, Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);

    //Configure TIM2: Period = 0xffff, prescaler = 71(72M/(71+1) = 1MHz), CounterMode  = upCounting mode
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure ;
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler     = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    //Configure TIM2's out compare mode:  out compare mode = toggle, out compare value = 0 (useless before enable compare interrupt), enable TIM2_CH1
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_Pulse       = 0;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    
    bsp_timer_cancel_schedule();
    
    //enable TIM2
    TIM_Cmd(TIM2, ENABLE); 
}

void spi_reConfiguration()
{
    SPI_InitTypeDef  SPI_InitStructure;

    //enable SPI2 and GPIOB, Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    //Configure SPI2
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //Full-duplex synchronous transfers on two lines
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;//Master Mode
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b; //8-bit transfer frame format
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;  //the SCK pin has a low-level idle state 
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge; //the first rising edge on the SCK pin is the MSBit capture strobe,
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;//Software NSS mode
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//BaudRate Prescaler = 8 
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;//data order with MSB-first
    SPI_InitStructure.SPI_CRCPolynomial     = 7;//CRC Polynomial = 7
    SPI_Init(SPI2, &SPI_InitStructure);
  
    //enable SPI2 
    SPI_Cmd(SPI2, ENABLE);
}

void radiotimer_reConfiguration()
{
  
}

void radio_reConfiguration()
{
  
}

 

