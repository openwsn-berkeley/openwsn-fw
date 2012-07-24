/**
\brief GINA-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/

#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "leds.h"
#include "rtc_timer.h"

//=========================== variables =======================================

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_vars_t;

radiotimer_vars_t radiotimer_vars;

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

void radiotimer_start(u32 period) 
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP|RCC_APB1Periph_PWR , ENABLE);
    
    PWR_BackupAccessCmd(ENABLE);// 使能写 Backup domain 

    RCC_LSEConfig(RCC_LSE_ON);//打开外部低频晶振

    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET);//等待外部低频晶振工作正常

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//外部低频晶振作为RTC晶振源

    RCC_RTCCLKCmd(ENABLE);//使能RTC

    RTC_WaitForSynchro();

    RTC_WaitForLastTask();
    
    RTC_ClearFlag(RTC_IT_OW);
    RTC_ITConfig(RTC_IT_OW, ENABLE);
    RTC_WaitForLastTask();

    RTC_WaitForLastTask(); //等待

    RTC_SetPrescaler(0);//(32.768 KHz)/(32767+1)
  
    RTC_WaitForLastTask();
    RTC_SetCounter(0xFFFFFFFF-period);
        
   
    //########### 有关NVIC的设置部分 ##############################################
    NVIC_InitTypeDef NVIC_InitStructure; 
  
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}

//===== direct access

u32 radiotimer_getValue() 
{
    return RTC_GetCounter();
}

void radiotimer_setPeriod(u32 period) 
{
    RTC_WaitForLastTask();
    RTC_SetCounter(0xFFFFFFFF-period);
}

u32 radiotimer_getPeriod() {
    return PORT_TsSlotDuration;//need to modify
}

//===== compare

void radiotimer_schedule(u32 period,u32 offset) 
{
    // offset when to fire
    RTC_WaitForLastTask();
    RTC_SetAlarm(0xFFFFFFFF - period + offset);
   
    // enable CCR1 interrupt (this also cancels any pending interrupts)
    RTC_WaitForLastTask();
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);

}

void radiotimer_cancel() 
{
   // reset CCR1 value (also resets interrupt flag)
    RTC_WaitForLastTask();
    RTC_SetAlarm(0);
    RTC_WaitForLastTask();
   // disable CCR1 interrupt
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
}

//===== capture

u32 radiotimer_getCapturedTime() 
{
   return RTC_GetCounter();
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

uint8_t radiotimer_isr() 
{
  if(RTC_GetFlagStatus(RTC_IT_ALR) != RESET)
  {
    RTC_ClearFlag(RTC_IT_ALR);
    if (radiotimer_vars.compare_cb!=NULL)
    {
      // call the callback
      radiotimer_vars.compare_cb();
      // kick the OS
      return 1;
    }
  }
  else
  {
      if(RTC_GetFlagStatus(RTC_IT_OW) != RESET)
      {
        RTC_ClearFlag(RTC_IT_OW);
        if (radiotimer_vars.overflow_cb!=NULL)
        {
          // call the callback
          radiotimer_vars.overflow_cb();
          // kick the OS
          return 1;
        }
      }
      else
      {
        while(1);
      }
  }
   return 0;
}