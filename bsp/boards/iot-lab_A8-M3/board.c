/**
\brief iot-lab_A8-M3 definition of the "board" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
\author Tengfei Chang <tengfei.chang@inria.fr>,         september 2016.
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

void board_init()
{
    RCC_Configuration();//Configure rcc
    NVIC_Configuration();//configure NVIC and Vector Table

    board_enableHardFaultExceptionHandler();

    //configure ALL GPIO to AIN to get lowest power
    GPIO_Config_ALL_AIN();

    GPIO_InitTypeDef  GPIO_InitStructure;

    //enable GPIOC and GPIOA, Clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

    //Configure PA.02 as SLP_TR pin of RF
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //Configure PC.01 as RST pin of RF
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //set /RST pin high(never reset)
    GPIO_SetBits(GPIOC, GPIO_Pin_1);

    // Configure PC.04 as input floating (EXTI Line4)
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIOC->ODR |= 0x0010;//set low

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);//Connect EXTI Line4 to PC.4
    EXTI_ClearITPendingBit(EXTI_Line4);

    //Configures EXTI line 4 to generate an interrupt on rising edge
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
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

void board_sleep(){
    DBGMCU_Config(DBGMCU_STOP, ENABLE);
    // Enable PWR and BKP clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Desable the SRAM and FLITF clock in Stop mode
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_FLITF, DISABLE);
    // enter sleep mode
    __WFI();
}

void board_reset(){
    NVIC_GenerateSystemReset();
}

//=========================== private =========================================


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

