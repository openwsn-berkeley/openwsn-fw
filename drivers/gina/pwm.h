#ifndef __TIMER_H
#define __TIMER_H

/**
\addtogroup drivers
\{
\addtogroup PWM
\{
*/

#include "msp430x26x.h" 

//=========================== define ==========================================

#define PWMPERIOD      6000  // 3 ms @ 2 MHz
#define PWMMIN         2000  // 1 ms
#define PWMMAX         4000  // 2 ms
#define PWM0           3000  // 1.5 ms
#define PWMMAXD        1000  // maximum difference
#define PWMBUF          300  // buffer at PWM extremes

#define MOTORPERIOD     800
#define MOTORMAX        800
#define MOTORMIN          0

#define pwm_getservo(which) TBCCR##which
#define pwm_getmotor(which) TACCR##which

#define PWM_WAIT pwm_tick = 0; while (!pwm_tick)

//=========================== typedef =========================================

//=========================== variables =======================================

volatile char pwm_tick;
volatile int  pwm_button_debounce;

//=========================== prototypes ======================================

void pwm_init(void);
void pwm_config(void);
void pwm_setservo(int which, int val);
void pwm_setmotor(int which, int val);

void pwm_nexttick(void);
void pwm_sleep(int ticks);
int  pwm_test(void);
void pwm_squiggle(void);

/**
\}
\}
*/

#endif
