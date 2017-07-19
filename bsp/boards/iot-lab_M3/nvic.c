/**
\brief iot-lab_M3 definition of the "nvic" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
*/
#include "stm32f10x_conf.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void NVIC_init(void){   
  #ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
  #else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
  #endif
}

//configuration uart interrput
void NVIC_uart(void){
  //Configure NVIC: Preemption Priority = 3 and Sub Priority = 3
  NVIC_InitTypeDef 	NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel                    = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

//configuration spi interrput
void NVIC_spi(void){
#ifdef SPI_IN_INTERRUPT_MODE
  //Configure NVIC: Preemption Priority = 1 and Sub Priority = 1
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel	                   = SPI1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority	         = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd	                 = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
}

//configuration radiotimer interrput
void NVIC_sctimer(void) {
    
    NVIC_InitTypeDef NVIC_InitStructure;
    //Configure RTC Alarm interrupt:
    //Configure NVIC: Preemption Priority = 0 and Sub Priority = 1
    NVIC_InitStructure.NVIC_IRQChannel                      = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//configuration radio interrput
void NVIC_radio(void){
   //Configure NVIC: Preemption Priority = 2 and Sub Priority = 0
  NVIC_InitTypeDef  NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel                     = EXTI4_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 0; 
  NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE; 
  NVIC_Init(&NVIC_InitStructure);
}

//configuration of interrupt
void NVIC_Configuration(void){
  //Set the Vector Table base location
  NVIC_init();

  //2 bits for Preemption Priority and 2 bits for Sub Priority
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

