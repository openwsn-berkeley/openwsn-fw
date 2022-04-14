/**
\brief A PWM module. 

\author Tengfei Chang     <tengfei.chang@inria.fr> April 2017
*/

#include <source/gptimer.h>
#include <headers/hw_memmap.h>

#include <source/ioc.h>
#include <source/gpio.h>

// ========================== define ==========================================

// ========================== variable ========================================

typedef struct {
    bool isStarted;
} pwm_vars_t;

pwm_vars_t pwm_vars;


// ========================== private =========================================

// ========================== protocol =========================================

/**
\brief Initialization pwm.
*/
void pwm_init(void) {

    // Configure GPIOPortB.3 as the Timer0_InputCapturePin.1
    IOCPinConfigPeriphOutput(GPIO_B_BASE, GPIO_PIN_3, IOC_MUX_OUT_SEL_GPT3_ICP1);

    TimerConfigure(GPTIMER3_BASE, (GPTIMER_CFG_SPLIT_PAIR | GPTIMER_CFG_A_PWM));
    TimerControlLevel(GPTIMER3_BASE, GPTIMER_A, false);

    // configure GPTimer to be 38khz (infrared standard modulation frequency)

//    // set the frequency (32,000,000/842=38khz)
//    TimerLoadSet(GPTIMER3_BASE, GPTIMER_A, 842);
//    // set 50% duty cycle
//    TimerMatchSet(GPTIMER3_BASE, GPTIMER_A, 421);

    // measured frequency: 32.719khz (from infrared of remote controller)
    TimerLoadSet(GPTIMER3_BASE, GPTIMER_A, 978);
    TimerMatchSet(GPTIMER3_BASE, GPTIMER_A, 489);
}

void pwm_enable(void) {
    TimerEnable(GPTIMER3_BASE, GPTIMER_A);
}

void pwm_disable(void) {
    TimerDisable(GPTIMER3_BASE, GPTIMER_A);
}

// ========================== private =========================================