/**
\brief openmoteSTM32 definition of the NVIC.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void NVIC_init(void) {
    
    // Set the Vector Table base location at 0x08000000
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
}

//configuration uart interrput
void NVIC_uart(void) {
#ifdef EV1000_USB    
    //Configure NVIC: Preemption Priority = 3 and Sub Priority = 3
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}

//configuration spi interrput
void NVIC_spi(void) {
    
#ifdef SPI_IN_INTERRUPT_MODE
    //Configure NVIC: Preemption Priority = 2 and Sub Priority = 2
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = SPI1_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}

//configuration sctimer interrput
void NVIC_sctimer(void) {
    
    NVIC_InitTypeDef NVIC_InitStructure;
    //Configure RTC Alarm interrupt:
    //Configure NVIC: Preemption Priority = 1 and Sub Priority = 1
    NVIC_InitStructure.NVIC_IRQChannel                      = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//configuration radio interrput
void NVIC_radio(void){
    
     //Configure NVIC: Preemption Priority = 1 and Sub Priority = 0
    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                      = EXTI9_5_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
}

// Configure usb interrupts
void NVIC_usb(void){
#ifdef EV1000_USB
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

//configuration of interrupt on openmotestm32
void NVIC_Configuration(void){
    
    //Set the Vector Table base location
    NVIC_init();
  
    //2 bits for Preemption Priority and 2 bits for Sub Priority
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}