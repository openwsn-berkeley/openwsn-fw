/**
\brief openmoteSTM32 definition of the "radiotimer" bsp module.

On openmoteSTM32, we use RTC for the radiotimer module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stdint.h"

#include "stm32f10x_lib.h"
#include "leds.h"
#include "radiotimer.h"
#include "board.h"

#include "rcc.h"
#include "nvic.h"


//=========================== variables =======================================

enum  radiotimer_irqstatus_enum{
    RADIOTIMER_NONE     = 0x00, //alarm interrupt default status
    RADIOTIMER_OVERFLOW = 0x01, //alarm interrupt caused by overflow
    RADIOTIMER_COMPARE  = 0x02, //alarm interrupt caused by compare
};

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
   uint8_t                   overflowORcompare;//indicate RTC alarm interrupt status
   uint16_t                  currentSlotPeriod;
} radiotimer_vars_t;

volatile radiotimer_vars_t radiotimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void radiotimer_init() {
   // clear local variables
   memset(&radiotimer_vars,0,sizeof(radiotimer_vars_t));
}

void radiotimer_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.overflow_cb    = cb;
}

void radiotimer_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_vars.compare_cb     = cb;
}

void radiotimer_setStartFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_setEndFrameCb(radiotimer_capture_cbt cb) {
   while(1);
}

void radiotimer_start(uint16_t period) {
    //enable BKP and PWR, Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP|RCC_APB1Periph_PWR , ENABLE);
    
    // RTC clock source configuration 
    PWR_BackupAccessCmd(ENABLE);                      //Allow access to BKP Domain
    RCC_LSEConfig(RCC_LSE_ON);                        //Enable LSE OSC
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET); //Wait till LSE is ready
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);           //Select the RTC Clock Source
    RCC_RTCCLKCmd(ENABLE);                            //enable RTC
    
    // RTC configuration 
    // Wait for RTC APB registers synchronisation 
    RTC_WaitForSynchro();
    
    RTC_SetPrescaler(1);                              //use 16KHz clock
    RTC_WaitForLastTask();                            //Wait until last write operation on RTC registers has finished

    //Set the RTC time counter to 0
    RTC_SetCounter(0);
    RTC_WaitForLastTask();

    // Set the RTC time alarm(the length of slot)
    RTC_SetAlarm(period);
    RTC_WaitForLastTask();
    
    radiotimer_vars.currentSlotPeriod = period;
    
    //interrupt when reach alarm value
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    
    //set radiotimer irpstatus
    radiotimer_vars.overflowORcompare = RADIOTIMER_OVERFLOW;
   
    //Configures EXTI line 17 to generate an interrupt on rising edge(alarm interrupt to wakeup board)
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
    
    //Configure RTC Alarm interrupt:
    //Configure NVIC: Preemption Priority = 1 and Sub Priority = 0
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                    = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//===== direct access

uint16_t radiotimer_getValue() {
    RTC_WaitForSynchro();
    uint32_t counter = RTC_GetCounter();
    return (uint16_t)counter;
}

void radiotimer_setPeriod(uint16_t period) {

    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    //need to disable radio also in case that a radio interrupt is happening when set Alarm value
    
    
    //Reset RTC Counter to begin a new slot
    RTC_SetAlarm(period);
    RTC_WaitForLastTask();
    
    radiotimer_vars.currentSlotPeriod = period;
    
    //set radiotimer irpstatus
    radiotimer_vars.overflowORcompare = RADIOTIMER_OVERFLOW;
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
}

uint16_t radiotimer_getPeriod() {
    RTC_WaitForSynchro();
    uint32_t period = RTC_GetAlarm();
    return (uint16_t)period;
}

//===== compare

void radiotimer_schedule(uint16_t offset) {
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    //need to disable radio also in case that a radio interrupt is happening
    
    // Set the RTC alarm(RTC timer will alarm at next state of slot)
    RTC_SetAlarm(offset);
    RTC_WaitForLastTask();
    
    //set radiotimer irpstatus
    radiotimer_vars.overflowORcompare = RADIOTIMER_COMPARE;
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
}

void radiotimer_cancel() {
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    //need to disable radio also in case that a radio interrupt is happening
    
    
    // set RTC alarm (slotlength) 
    RTC_SetAlarm(radiotimer_vars.currentSlotPeriod);
    RTC_WaitForLastTask();
    
    //set radiotimer irpstatus
    radiotimer_vars.overflowORcompare = RADIOTIMER_OVERFLOW;
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
}

//===== capture

inline uint16_t radiotimer_getCapturedTime() {
    RTC_WaitForSynchro();
    uint32_t counter = RTC_GetCounter();
    return (uint16_t)counter;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radiotimer_isr() {
   uint8_t taiv_temp = radiotimer_vars.overflowORcompare;
   switch (taiv_temp) {
      case RADIOTIMER_COMPARE:
         if (radiotimer_vars.compare_cb!=NULL) {
            RCC_Wakeup();
            // call the callback
            radiotimer_vars.compare_cb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
      case RADIOTIMER_OVERFLOW: // timer overflows
         if (radiotimer_vars.overflow_cb!=NULL) {
           
            //Wait until last write operation on RTC registers has finished
            RTC_WaitForLastTask();                            
            
            //Set the RTC time counter to 0
            RTC_SetCounter(0x00000000);
            RTC_WaitForLastTask();
            RCC_Wakeup();
            // call the callback
            radiotimer_vars.overflow_cb();
            // kick the OS
            return KICK_SCHEDULER;
         }
         break;
      case RADIOTIMER_NONE:                     // this should not happen
      default:
         while(1);                               // this should not happen
   }
  return DO_NOT_KICK_SCHEDULER;
}
