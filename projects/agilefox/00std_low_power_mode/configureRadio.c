#include "stm32f10x_lib.h"
#include "spi.h"
#include "radio.h"
#include "at86rf231.h"
#include "configureRadio.h"

void radio_configure()
{
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
    
    spi_init();
    radio_init();
    
    GPIOB->ODR |= 0X0002;
}