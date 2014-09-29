#include "pwm.h"
#include "leds.h"

void pwm_init(void) {
   pwm_tick = 0;
   pwm_button_debounce = 0;

   TBCTL = TBSSEL_2 + ID_3 + MC_1 + TBIE;        // SMCLK / 8, count up to TBCL0, Interrupt on overflow

   TBCCR0 = PWMPERIOD;                           // PWM Period
   TBCCR1 = PWM0;                                // CCR1 PWM duty cycle
   TBCCTL1 = OUTMOD_7;                           // CCR1 reset/set
   TBCCR2 = PWM0;
   TBCCTL2 = OUTMOD_7;
   TBCCR3 = PWM0;
   TBCCTL3 = OUTMOD_7;
   TBCCR4 = PWM0;
   TBCCTL4 = OUTMOD_7;

   //TBCCR5 = PWMMIN;                            // 1 ms timer for loop
   //TBCCTL5 = OUTMOD_7 + CCIE;
   //TBCCR6 = PWMMAX;                            // 2 ms timer for loop
   //TBCCTL6 = OUTMOD_7 + CCIE;

   TACTL = TBSSEL_2 + ID_3 + MC_1;               // SMCLK / 8, count up to TACCR0
   TACCR0 = MOTORPERIOD;                         // 2.5 kHz frequency
   TACCTL1 = OUTMOD_7;
   TACCR1 = MOTORMIN;
   TACCTL2 = OUTMOD_7;
   TACCR2 = MOTORMIN;
}

int pwm_test(void) {
   return 0; // success
}

void pwm_config(void) {
   P4DIR |= 0x1f;                                // P4.0 - P4.3 output
   P4SEL |= 0x1f;                                // P4.0 - P4.3 TBx options

   P1DIR |= 0x0c;                                // P1.2 - P1.3 output
   P1SEL |= 0x0c;                                // P1.2 - P1.3 TBx options
}

void pwm_setservo(int which, int val) {
   val = val + PWM0;
   if (val < PWMMIN + PWMBUF) val = PWMMIN + PWMBUF;
   if (val > PWMMAX - PWMBUF) val = PWMMAX - PWMBUF;
   switch (which) {
      case 1:
         TBCCR1 = val;
         break;
      case 2:
         TBCCR2 = val;
         break;
      case 3:
         TBCCR3 = val;
         break;
      case 4:
         TBCCR4 = val;
         break;
   }
}

void pwm_setmotor(int which, int val) {
   if (val < MOTORMIN) val = MOTORMIN;
   if (val > MOTORMAX) val = MOTORMAX;
   switch (which) {
      case 1:
         TACCR1 = val;
         break;
      case 2:
         TACCR2 = val;
         break;
   }
}

void pwm_nexttick(void) {
   pwm_tick = 0;
   while (!pwm_tick) {
   };
}

void pwm_sleep(int ticks) {
   while (ticks--) {
      pwm_nexttick();
   }
}

void pwm_squiggle(void) {
   int i=0;
   while (1) {
      for (; i<PWMMAXD; i++) {
         pwm_setservo(1,i);
         pwm_setservo(2,i);
         pwm_setservo(3,i);
         pwm_setservo(4,i);
      }
      for (; i>-PWMMAXD; i--) {
         pwm_setservo(1,i);
         pwm_setservo(2,i);
         pwm_setservo(3,i);
         pwm_setservo(4,i);
      }
   }
}

#pragma vector=TIMERB1_VECTOR
   __interrupt void timerb_isr(void) {
      if (pwm_button_debounce)
         pwm_button_debounce--;
      pwm_tick = TBIV;
   }
