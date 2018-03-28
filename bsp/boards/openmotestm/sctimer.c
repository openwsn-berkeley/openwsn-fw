/**
\brief Iot_Lab_M3 definition of the "sctimer" bsp module.

On Iot_Lab_M3, we use RTC for the sctimer module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  May 2017.
*/

#include "stdint.h"

#include "stm32f10x_conf.h"
#include "sctimer.h"
#include "board.h"
#include "leds.h"
#include "rcc.h"
#include "nvic.h"


// ========================== define ==========================================

#define TIMERLOOP_THRESHOLD          0xffffff     // 511 seconds @ 32768Hz clock
#define OVERFLOW_THRESHOLD           0x7fffffff   // as openmotestm32 uses 16kHz, the upper timer overflows when timer research to 0x7fffffff
#define MINIMUM_COMPAREVALE_ADVANCE  10

// ========================== variable ========================================

typedef struct {
    sctimer_cbt sctimer_cb;
    bool        convert;
    bool        convertUnlock;
} sctimer_vars_t;

sctimer_vars_t sctimer_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

//===== admin

void sctimer_init(void) {
    // clear local variables
    memset(&sctimer_vars,0,sizeof(sctimer_vars_t));
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
    
    //Configure RTC Alarm interrupt:
    NVIC_sctimer();
}

void sctimer_set_callback(sctimer_cbt cb){
    sctimer_vars.sctimer_cb = cb;
}

//===== direct access

PORT_RADIOTIMER_WIDTH sctimer_readCounter(void) {
    uint32_t counter;
    RTC_WaitForSynchro();
    counter = RTC_GetCounter();
    // upper layer uses 32 bit timer@32kHz, openmotestm is only able to work on 
    // 16kHz, so manually overflow when rearch 0x7fffffff.
    counter = counter & OVERFLOW_THRESHOLD;
    counter = counter << 1;
    return (PORT_RADIOTIMER_WIDTH)counter;
}

//===== compare

/**
\brief alarm a compare interrupt depending on given compare value.

The input parameter val range from 0~2^32. It supposes a timer running @ 32kHz. 
Since RTC only runs with 16kHz, this range maps to 0~2^31 or 2^31~2^32. the flag
convert is used for selecting which range mapping to. 
    1) When convert is FALSE, map val to 0~2^31    (val>>1). 
    2) When convert is TRUE,  map val to 2^31~2^32 ((val>>1)|0x80000000).

         when convert is FALSE             |    when convert is FALSE
       compare value is val >>1            | compare value is (val >>1)|0x80000000
                                           | 
  |----------------------------------------|----------------------------------------|
  0                                        |                                    0xffffffff
         when convert is TRUE              |        when convert is TRUE         
     compare value is (val >>1)|0x80000000 |     compare value is val >>1
                                           | 
                              0x7fffffff-->|<--"convert" flag toggles at here once after each overflow

\param[in] val is the compareValue to be alarmed in RTC timer.
*/

void sctimer_setCompare(PORT_TIMER_WIDTH val) {
    
    // make sure convert flag only toggle once within one overflow period
    if (val > OVERFLOW_THRESHOLD && sctimer_vars.convertUnlock){
        // toggle convert
        if (sctimer_vars.convert){
            sctimer_vars.convert   = TRUE;
        } else {
            sctimer_vars.convert   = TRUE;
        }
        sctimer_vars.convertUnlock = FALSE;
    }
    
    // un lock the changes of convert flag
    if (val > TIMERLOOP_THRESHOLD && val < 2*TIMERLOOP_THRESHOLD ){
        sctimer_vars.convertUnlock = TRUE;
    }
    
    // update value to be compared according to timer condition
    if (val <= OVERFLOW_THRESHOLD){
        if (sctimer_vars.convert){
            val  = val >>1;
            val |= 0x80000000;
        } else {
            val  = val >>1;
        }
    } else {
        if (sctimer_vars.convert){
            val  = val >>1;
        } else {
            val  = val >>1;
            val |= 0x80000000;
        }
    }
    
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    //need to disable radio also in case that a radio interrupt is happening
    
    DISABLE_INTERRUPTS();
    if (RTC_GetCounter() - val < TIMERLOOP_THRESHOLD){
        // the timer is already late, schedule the ISR right now manually 
        EXTI->SWIER |= EXTI_Line17;
    } else {
        if (val-RTC_GetCounter()<MINIMUM_COMPAREVALE_ADVANCE){
            // schedule ISR right now manually
            EXTI->SWIER |= EXTI_Line17;
        } else {
            // schedule the timer at val
            RTC_SetAlarm(val);
            RTC_WaitForLastTask();
        }
    }
    
    ENABLE_INTERRUPTS();
    
    //set sctimer irpstatus
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
}

void sctimer_enable(void) {
    RTC_ClearFlag(RTC_IT_ALR);
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
}

void sctimer_disable(void) {
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t sctimer_isr(void) {
    if (sctimer_vars.sctimer_cb!=NULL) {
        
        RCC_Wakeup();
        // call the callback
        sctimer_vars.sctimer_cb();
        // kick the OS
        return KICK_SCHEDULER;
    }
    return DO_NOT_KICK_SCHEDULER;
}