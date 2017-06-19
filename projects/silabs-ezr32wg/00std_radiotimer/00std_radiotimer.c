#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_chip.h"

int compare = 33;
int cmpTimes = 0;
int compare1 = 0;

void LETIMER0_IRQHandler(void)
{
  
  
  if ((LETIMER0->IF & 2) == 2){
    compare1 += compare;
    cmpTimes++;
    LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);
    if (cmpTimes < 9){
      LETIMER_CompareSet(LETIMER0, 1, 328 - (compare1));
      GPIO_PinOutToggle(gpioPortD, 7);}
    else {
      LETIMER_CompareSet(LETIMER0, 1, 328 - compare);
      compare1 = 33;
      cmpTimes = 0;
    }
  }
  if ((LETIMER0->IF & 1) == 1){
    LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP0);
    //LETIMER_CompareSet(LETIMER0, 1, 328 - compare);
    GPIO_PinOutToggle(gpioPortD, 2);}
}

void LETIMER_setup(void)
{

  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_CORELE, true);
  CMU_ClockEnable(cmuClock_LETIMER0, true);  
  CMU_ClockEnable(cmuClock_GPIO, true);
 
  GPIO_PinModeSet(gpioPortD, 6, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);
  
  LETIMER_CompareSet(LETIMER0, 0, 328);
  LETIMER_CompareSet(LETIMER0, 1, 328 - compare);
  
   /* Route LETIMER to location 0 (PD6 and PD7) and enable outputs */
 // LETIMER0->ROUTE = LETIMER_ROUTE_OUT0PEN | LETIMER_ROUTE_OUT1PEN | LETIMER_ROUTE_LOCATION_LOC0;
  
  
    const LETIMER_Init_TypeDef letimerInit = 
  {
  .enable         = true,                   /* Start counting when init completed. */
  .debugRun       = false,                  /* Counter shall not keep running during debug halt. */
  .rtcComp0Enable = false,                  /* Don't start counting on RTC COMP0 match. */
  .rtcComp1Enable = false,                  /* Don't start counting on RTC COMP1 match. */
  .comp0Top       = true,                   /* Load COMP0 register into CNT when counter underflows. COMP0 is used as TOP */
  .bufTop         = false,                  /* Don't load COMP1 into COMP0 when REP0 reaches 0. */
  .out0Pol        = 0,                      /* Idle value for output 0. */
  .out1Pol        = 0,                      /* Idle value for output 1. */
  .ufoa0          = letimerUFOANone,      
  .ufoa1          = letimerUFOANone,      
  .repMode        = letimerRepeatFree       /* Count until stopped */
  };
  
  /* Initialize LETIMER */
  LETIMER_Init(LETIMER0, &letimerInit); 
  LETIMER0->REP0 = 1 ;
  LETIMER0->REP1 = 1 ;
}

int main(void)
{  
  /* Align different chip revisions */
 // CHIP_Init();

  /* Initialize LETIMER */
  LETIMER_setup();

  

  //-----------------------------------------------------------------------------------
  // THE INTERRUPT IS SIMPLY TO DECREASE THE VALUE OF COMP1 TO VARY THE PWM DUTY CYCLE
  //-----------------------------------------------------------------------------------
  /* Enable underflow interrupt */  
  LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP1);  
  LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP0);  
  
  /* Enable LETIMER0 interrupt vector in NVIC*/
  NVIC_EnableIRQ(LETIMER0_IRQn);
  
  while(1)
  {
    /* Go to EM2 */
    EMU_EnterEM2(false);
  }
}

