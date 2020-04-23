/**
\brief iot-lab_M3 definition of the "board" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
\author Tengfei Chang <tengfei.chang@inria.fr>,         May 2017.
*/
#include "stm32f10x_conf.h"
#include "board.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "spi.h"
#include "sctimer.h"
#include "radio.h"
#include "rcc.h"
#include "nvic.h"
#include "debugpins.h"
#include "opentimers.h"
#include "gpio.h"
#include "cryptoengine.h"


//=========================== variables ============================================
slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
slotType_t selected_slot_type;

//=========================== main ============================================

extern int mote_main(void);

int main(void){
    return mote_main();
}

//=========================== private =========================================

// configure the hard fault exception
void board_enableHardFaultExceptionHandler(void);

//=========================== public ==========================================

void board_init(void)
{
    RCC_Configuration();//Configure rcc
    NVIC_Configuration();//configure NVIC and Vector Table

    board_enableHardFaultExceptionHandler();

    //configure ALL GPIO to AIN to get lowest power
    GPIO_Config_ALL_AIN();
    //configuration GPIO to measure the time from sleep to 72MHz
    GPIO_Configuration();

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
    sctimer_init();
    radio_init();
    debugpins_init();
    //enable nvic for the radio
    NVIC_radio();
    cryptoengine_init();
    board_init_slot_vars();
}


//==== bootstrapping slot info lookup table
void board_init_slot_vars(void){
    
    // 20ms slot
    slot_board_vars [SLOT_20ms_24GHZ].PORT_SLOTDURATION                   =  20   ; // ms  
    slot_board_vars [SLOT_20ms_24GHZ].PORT_TsSlotDuration                 =  655  ; //    20ms
    slot_board_vars [SLOT_20ms_24GHZ].PORT_maxTxDataPrepare               =  110  ; //  3355us (not measured)
    slot_board_vars [SLOT_20ms_24GHZ].PORT_maxRxAckPrepare                =  20   ; //   610us (not measured)
    slot_board_vars [SLOT_20ms_24GHZ].PORT_maxRxDataPrepare               =  33   ; //  1000us (not measured)
    slot_board_vars [SLOT_20ms_24GHZ].PORT_maxTxAckPrepare                =  50   ; //  1525us (not measured)
    slot_board_vars [SLOT_20ms_24GHZ].PORT_delayTx                        =  18   ; //   549us (not measured)
    slot_board_vars [SLOT_20ms_24GHZ].PORT_delayRx                        =  0    ; //     0us (can not measure)
}

// To get the current slotDuration at any time
// used during initialization by sixtop to fire the first sixtop EB
uint16_t board_getSlotDuration (time_type_t time_type)
{
  if (time_type == TIME_MS)
    return slot_board_vars [selected_slot_type].PORT_SLOTDURATION;
 else 
   return slot_board_vars [selected_slot_type].PORT_TsSlotDuration;
}

// Getter function for slot_board_vars
slot_board_vars_t board_getSlotTemplate (void)
{
  return slot_board_vars [selected_slot_type];
}

// Getter function for selected_slot_type
void board_setSlotType(slotType_t slot_type)
{
  selected_slot_type = slot_type;
}


void board_sleep(void) {
    DBGMCU_Config(DBGMCU_STOP, ENABLE);
    // Enable PWR and BKP clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Desable the SRAM and FLITF clock in Stop mode
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_FLITF, DISABLE);
    // enter sleep mode
    __WFI();
}

void board_reset(void) {
    NVIC_SystemReset();
}

//=========================== private =========================================

void board_enableHardFaultExceptionHandler(void){
    // Configures:
    //    bit9. stack alignment on exception entry 
    //    bit4. enables faulting
    //    bit3. unaligned access traps
    SCB->CCR = 0x00000210;
}

