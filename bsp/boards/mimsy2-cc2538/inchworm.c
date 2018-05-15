/******************************************************************************
* INCLUDES
*/
#include <stdio.h>
//#include "bsp.h"
//#include "bsp_led.h"
#include "gptimer.h"
#include "sys_ctrl.h"
#include "hw_gptimer.h"
#include "hw_ints.h"
#include "gpio.h"
#include "interrupt.h"
//#include "led.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "ioc.h"
#include "inchworm.h"

/*GLOBALS
*/
volatile uint32_t inchwormTimer=0;
volatile uint32_t phaseTimer=0;
volatile uint32_t stepCountList[16];
volatile bool actuating=false;
volatile uint32_t stepTargetList[16];
volatile  uint32_t timerIntA;
volatile  uint32_t timerIntB;
volatile bool activeDriveList[16];
volatile uint32_t motorsNum=0;
volatile uint32_t startSecondTimer=false;
InchwormMotor motorList[16];
uint32_t value;
/******************************************************************************
* DEFINES
*/
#define GP4 
#define FREQ_CNT SysCtrlClockGet()/frequency 
#define OVERLAP_MATCH (100-dutycycle)*SysCtrlClockGet()/frequency/100 



/*********************************************************************************
*STRUCTURES
*/


/******************************************************************************
* FUNCTIONS
*/
void
PwmTimerAIntHandler(void)
{
    //
    // Clear the timer interrupt flag.
    //
     TimerIntClear(inchwormTimer, GPTIMER_CAPA_EVENT );
//    mimsyLedToggle(GPIO_PIN_4);
    
    
    //optimize this for loop, it think it will be a bit much
    for(uint8_t i=0;i<motorsNum;i++){
      if(activeDriveList[i]){
        stepCountList[i]=stepCountList[i]+2;
        if(stepCountList[i]>=stepTargetList[i]){
          stepCountList[i]=0;
          activeDriveList[i]=false;
          inchwormHold(motorList[i]);      
        }
      }
    }

}
void
PwmTimerBIntHandler(void)
{
    //
    // Clear the timer interrupt flag.
    //
     TimerIntClear(inchwormTimer, GPTIMER_CAPB_EVENT);
  //  mimsyLedToggle(GPIO_PIN_7);

      
      
    

}
void
PhaseTimerAIntHandler(void)
{
       TimerIntClear(phaseTimer, GPTIMER_TIMA_TIMEOUT|GPTIMER_TIMB_TIMEOUT);
       startSecondTimer=true;
}

//TODO: rename motor
void inchwormInit(struct InchwormSetup setup){
  uint32_t freqCnt=SysCtrlClockGet()/setup.motorFrequency;
  uint32_t match=(100-setup.dutyCycle)*SysCtrlClockGet()/setup.motorFrequency/100 ;
  uint32_t pwmTimerClkEnable=5;
  uint32_t pwmTimerBase=5;
  uint32_t phaseTimerBase=5;
  uint32_t phaseTimerClkEnable=5;
  uint32_t x=5;
  uint32_t ui32Loop=5;
  uint32_t phaseIntA=5;
  //setup active motors list
  motorsNum=setup.numOfMotors;
  
  for(uint8_t i=0;i<motorsNum;i++){
    activeDriveList[i]=false;
    motorList[i]=setup.iwMotors[i];
    stepCountList[i]=0;
    stepTargetList[i]=0;
  }
  
  
  
  //find timer modules used
  switch(setup.timer){
      
  case 0: 
    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT0; 
    pwmTimerBase=GPTIMER0_BASE;
    timerIntA=INT_TIMER0A;
    timerIntB=INT_TIMER0B;
    break;

  case 1: 
    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT1;
    pwmTimerBase=GPTIMER1_BASE;
    timerIntA=INT_TIMER1A;
    timerIntB=INT_TIMER1B;
    break;
    
  case 2: 
    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT2;
    pwmTimerBase=GPTIMER2_BASE;
    timerIntA=INT_TIMER2A;
    timerIntB=INT_TIMER2B;
    break;
    
  case 3: 
    pwmTimerClkEnable=SYS_CTRL_PERIPH_GPT3;
    pwmTimerBase=GPTIMER3_BASE;
    timerIntA=INT_TIMER3A;
    timerIntB=INT_TIMER3B;
    break;    
    
    
  }
  
  switch(setup.phaseTimer){
  case 0: 
    phaseTimerClkEnable=SYS_CTRL_PERIPH_GPT0;
    phaseTimerBase=GPTIMER0_BASE;
    phaseIntA=INT_TIMER0A;

    break;

  case 1: 
    phaseTimerClkEnable=SYS_CTRL_PERIPH_GPT1;
    phaseTimerBase=GPTIMER1_BASE;
    phaseIntA=INT_TIMER1A;

    break;
    
  case 2: 
    phaseTimerClkEnable=SYS_CTRL_PERIPH_GPT2;
    phaseTimerBase=GPTIMER2_BASE;
    phaseIntA=INT_TIMER2A;

    break;
    
  case 3: 
    phaseTimerClkEnable=SYS_CTRL_PERIPH_GPT3;
    phaseTimerBase=GPTIMER3_BASE;
    phaseIntA=INT_TIMER3A;

    break; 
  }
  
    inchwormTimer=pwmTimerBase;//updates global value of current inchworm timer so interrupt handler knows which module to clear.
  
  
    SysCtrlPeripheralEnable(pwmTimerClkEnable); //enables timer module
   
   
    
    TimerConfigure(pwmTimerBase, GPTIMER_CFG_SPLIT_PAIR |GPTIMER_CFG_A_PWM | GPTIMER_CFG_B_PWM); //configures timers as pwm timers
  
  
    
    
   // TimerControlWaitOnTrigger( pwmTimerBase,GPTIMER_A,true); //configures 1a as a wait on trigger timer
   // TimerConfigure(pwmTimerBase,GPTIMER_CFG_ONE_SHOT); //timer 0b configured as a one shot timer. this will be used to daisy chain start timer 1a 
   
    
    TimerLoadSet(pwmTimerBase,GPTIMER_A,freqCnt); //1a load
    TimerLoadSet(pwmTimerBase,GPTIMER_B,freqCnt); //1b load
    
    
    
  //  load=TimerLoadGet(pwmTimerBase,GPTIMER_A);

    
    //set output pins for pwm//////////////////////////////
    
    for(uint8_t i=0;i<setup.numOfMotors;i++){
  
    IOCPinConfigPeriphOutput(setup.iwMotors[i].GPIObase1,setup.iwMotors[i].GPIOpin1,IOC_MUX_OUT_SEL_GPT3_ICP1); //maps pwm1 output to pin1
    IOCPinConfigPeriphOutput(setup.iwMotors[i].GPIObase2,setup.iwMotors[i].GPIOpin2,IOC_MUX_OUT_SEL_GPT3_ICP2); //maps pwm2 output to pin2
    }
    
    //set pwm polarities 
    TimerControlLevel(pwmTimerBase,GPTIMER_A,true); //active high pwm
    TimerControlLevel(pwmTimerBase,GPTIMER_B,true); //active high pwm
    
    //set pwm duty cycles
    TimerMatchSet(pwmTimerBase,GPTIMER_A,match);
    TimerMatchSet(pwmTimerBase,GPTIMER_B,match);
       
      //interrupts
    TimerIntClear(pwmTimerBase, GPTIMER_TIMB_TIMEOUT);
    TimerIntClear(pwmTimerBase, GPTIMER_TIMA_TIMEOUT);
    
    TimerIntRegister(pwmTimerBase, GPTIMER_A, PwmTimerAIntHandler);       //sets timer a interrupt handler
    TimerIntRegister(pwmTimerBase, GPTIMER_B, PwmTimerBIntHandler);      //sets timer 1b interrupt handler
    
    //
    // setup phase offset timer
    //
     SysCtrlPeripheralEnable(phaseTimerClkEnable); 
     TimerConfigure(phaseTimerBase,GPTIMER_CFG_ONE_SHOT); //configures one shot timer for phase offset
     TimerLoadSet(phaseTimerBase,GPTIMER_A,freqCnt/2);//offset timer
   //  TimerIntRegister(phaseTimerBase, GPTIMER_A, PhaseTimerAIntHandler);
   //  TimerIntClear(phaseTimerBase, GPTIMER_TIMA_TIMEOUT|GPTIMER_TIMB_TIMEOUT);
   //  TimerIntEnable(phaseTimerBase, GPTIMER_TIMA_TIMEOUT);
     
    // IntEnable(phaseIntA);
    //
    // enable interrupts for pos edge pwm 
    //
    TimerIntEnable(pwmTimerBase, GPTIMER_CAPA_EVENT| GPTIMER_CAPB_EVENT);
    TimerControlEvent(pwmTimerBase,GPTIMER_BOTH,GPTIMER_EVENT_POS_EDGE);


    //
    // Enable the Timer interrupts on the processor (NVIC).
    //
    IntEnable(timerIntA);
    IntEnable(timerIntB);

       //timer enables
  x=freqCnt/16;
  
    TimerEnable(pwmTimerBase,GPTIMER_B);
    //wait for a 1/8 period to initialize next timer ? this might be a bit sketchy and not perfect. this is really weird, when i added the list of iw motors i had to increase the divisor to 16 from 8 
   // for(ui32Loop=1;ui32Loop<x;ui32Loop++) {
  //  }
    TimerEnable(phaseTimerBase,GPTIMER_A);
   
    //wait for offest timer to expire and throw interrupt
    while(TimerValueGet(phaseTimerBase,GPTIMER_A)>10 &&TimerValueGet(phaseTimerBase,GPTIMER_A)!=freqCnt/2){
      
    }
    TimerEnable(pwmTimerBase,GPTIMER_A);
   
   
}

void inchwormRelease(InchwormMotor motor){

    GPIOPinTypeGPIOOutput(motor.GPIObase1,motor.GPIOpin1); //sets inchworm motor pis to gpio output setting which disables PWM output
    GPIOPinTypeGPIOOutput(motor.GPIObase2,motor.GPIOpin2);

    GPIOPinWrite(motor.GPIObase1,motor.GPIOpin1,255);   //releases inchworm motor palls
    GPIOPinWrite(motor.GPIObase2,motor.GPIOpin2,255);

}

void inchwormFreerun(InchwormMotor motor){
    
    GPIOPinTypeTimer(motor.GPIObase1,motor.GPIOpin1); //enables hw muxing of pin outputs
    GPIOPinTypeTimer(motor.GPIObase2,motor.GPIOpin2); //enables hw muxing of pin outputs

    IOCPadConfigSet(motor.GPIObase1,motor.GPIOpin1,IOC_OVERRIDE_OE|IOC_OVERRIDE_PUE); // enables pins as outputs, necessary for this code to work correctly
    IOCPadConfigSet(motor.GPIObase2,motor.GPIOpin2,IOC_OVERRIDE_OE|IOC_OVERRIDE_PUE); // enables pins as outputs, necessary for this code to work correctly

}

//holds inchworm motors
void inchwormHold(InchwormMotor motor){
    
    GPIOPinTypeGPIOOutput(motor.GPIObase1,motor.GPIOpin1); //sets pins to normal gpio mode; disables pwm output 
    GPIOPinTypeGPIOOutput(motor.GPIObase2,motor.GPIOpin2);

    GPIOPinWrite(motor.GPIObase1,motor.GPIOpin1,0);  //closes inwhorm palls 
    GPIOPinWrite(motor.GPIObase2,motor.GPIOpin2,0);
}

//drives inchworms a certain number of steps
void inchwormDriveToPosition(InchwormMotor motor, uint32_t steps){
    activeDriveList[motor.motorID]=true; //tells isr that this inchworm is actively driving
    stepTargetList[motor.motorID]=steps;  //tells isr what the target number of steps is for this motor
    inchwormFreerun(motor);

  
}

