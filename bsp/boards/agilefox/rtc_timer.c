/**
\brief openmoteSTM32 definition of the "rtc_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stm32f10x_lib.h"
#include "leds.h"
#include "rtc_timer.h"

#include "rcc.h"
#include "nvic.h"

//=========================== variables =======================================

typedef struct {
   rtc_timer_alarm_cbt    alarm_cb;
} rtc_timer_vars_t;

rtc_timer_vars_t rtc_timer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void rtc_timer_init() {
   // clear local variables
   memset(&rtc_timer_vars,0,sizeof(rtc_timer_vars_t));
}

void rtc_timer_setAlarmCb(rtc_timer_alarm_cbt cb) {
   rtc_timer_vars.alarm_cb    = cb;
}

void rtc_timer_start(u32 alarmValue) 
{
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
    
    RTC_SetPrescaler(0);                              //Set the RTC time base to 30.5us
    RTC_WaitForLastTask();                            //Wait until last write operation on RTC registers has finished

    //Set the RTC time counter to 0
    RTC_SetCounter(0);
    RTC_WaitForLastTask();

    // Set the RTC time alarm(the length of slot)
    RTC_SetAlarm(alarmValue);
    RTC_WaitForLastTask();
    
    //interrupt when reach alarm value
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
   
    //Configures EXTI line 17 to generate an interrupt on rising edge(alarm interrupt to wakeup board)
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitTypeDef  EXTI_InitStructure; 
    EXTI_InitStructure.EXTI_Line    = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
   
    //Configure RTC global interrupt:
    //Configure NVIC: Preemption Priority = 1 and Sub Priority = 1
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                    = RTC_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //Configure RTC Alarm interrupt:
    //Configure NVIC: Preemption Priority = 0 and Sub Priority = 1
    NVIC_InitStructure.NVIC_IRQChannel                    = RTCAlarm_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//===== direct access

uint16_t rtc_timer_getAlarm() {
    uint32_t alarmValue = RTC_GetAlarm();
    return (uint16_t)alarmValue;
}

void    rtc_timer_resetCounter() {
    RTC_SetCounter(0);                //Set RTC Counter to begin a new slot
    RTC_WaitForLastTask();            //Wait until last write operation on RTC registers has finished
}

//===== capture

uint16_t rtc_timer_getCapturedTime() 
{
    uint32_t counter = RTC_GetCounter();
    return (uint16_t)counter;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================


uint8_t rtc_timer_isr() 
{
    if (rtc_timer_vars.alarm_cb!=NULL)
    {
      // call the callback
      rtc_timer_vars.alarm_cb();
      // kick the OS
      return 1;
    }
    return 0;
}