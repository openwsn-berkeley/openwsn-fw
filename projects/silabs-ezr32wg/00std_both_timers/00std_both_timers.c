#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_chip.h"
#include "em_rtc.h"

int compare = 33;
int cmpTimes = 0;
int compare1 = 0;
int rtcCompare = 22;
int rtcTimes = 0;
int rtcCompare1 = 0;

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
    GPIO_PinOutToggle(gpioPortD, 6);}
}

void RTC_IRQHandler(void)
{
  /* Clear interrupt source */
 // RTC_IntClear(RTC_IFC_COMP0);
  
    if ((RTC->IF & 4) == 4){
    rtcCompare1 += rtcCompare;
    rtcTimes++;
    RTC_IntClear(RTC_IFC_COMP1);
    if (rtcTimes < 9){
      RTC_CompareSet( 1, rtcCompare1 );
      GPIO_PinOutToggle(gpioPortF, 3);}
    else {
      RTC_CompareSet( 1, rtcCompare);
      rtcCompare1 = rtcCompare;
      rtcTimes = 0;
    }
  }
  if ((RTC->IF & 2) == 2){
    RTC_IntClear(RTC_IFC_COMP0);
    //RTC_CompareSet(0, 361 - compare);
    GPIO_PinOutToggle(gpioPortD, 2);}
}

void LETIMER_setup(void)
{

  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_CORELE, true);
  CMU_ClockEnable(cmuClock_LETIMER0, true);  
  CMU_ClockEnable(cmuClock_GPIO, true);
 
  GPIO_PinModeSet(gpioPortF, 3, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 6, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 0);
  GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);
  
  LETIMER_CompareSet(LETIMER0, 0, 328);
  LETIMER_CompareSet(LETIMER0, 1, 328 - compare);
  
   /* Route LETIMER to location 0 (PD6 and PD7) and enable outputs */
 // LETIMER0->ROUTE = LETIMER_ROUTE_OUT0PEN | LETIMER_ROUTE_OUT1PEN | LETIMER_ROUTE_LOCATION_LOC0;
  
  
    const LETIMER_Init_TypeDef letimerInit = 
  {
  .enable         = true,                   /* Start counting when init completed. */ /*changed to false JMMS*/
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

void RTC_setup(void){
 /* Starting LFRCO and waiting until it is stable */
  //CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

  /* Routing the LFRCO clock to the RTC */
  //CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFRCO); already selected by 
  CMU_ClockEnable(cmuClock_RTC, true);
  
  /* Enabling clock to the interface of the low energy modules */
 // CMU_ClockEnable(cmuClock_CORELE, true);
   RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;
  
  rtcInit.enable   = true;      /* Enable RTC after init has run */
  rtcInit.comp0Top = true;      /* Clear counter on compare match */
  rtcInit.debugRun = false;     /* Counter shall keep running during debug halt. */

  /* Setting the compare value of the RTC */
  RTC_CompareSet(0, 361);
  RTC_CompareSet(1, 22);

  /* Enabling Interrupt from RTC */
 // RTC_IntEnable(RTC_IFC_COMP0);
 // RTC_IntEnable(RTC_IFC_COMP1);
 // NVIC_EnableIRQ(RTC_IRQn);

  /* Initialize the RTC */
  RTC_Init(&rtcInit);
  
}
int main(void)
{  
  /* Align different chip revisions */
 // CHIP_Init();
// CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
 CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
 
 //CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
 //CMU_ClockDivSet(cmuClock_CORE, cmuClkDiv_8);
 
 
  /* Initialize LETIMER */
  LETIMER_setup();
  
  RTC_setup();
  //-----------------------------------------------------------------------------------
  // THE INTERRUPT IS SIMPLY TO DECREASE THE VALUE OF COMP1 TO VARY THE PWM DUTY CYCLE
  //-----------------------------------------------------------------------------------
  /* Enable underflow interrupt */  
  LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP1);  
  LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP0);  
  RTC_IntEnable(RTC_IEN_COMP1);
  RTC_IntEnable(RTC_IEN_COMP0);
  
  /* Enable LETIMER0 interrupt vector in NVIC*/
  NVIC_EnableIRQ(LETIMER0_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);

  
  while(1)
  {
    /* Go to EM2 */
    EMU_EnterEM2(false);
  }
}

