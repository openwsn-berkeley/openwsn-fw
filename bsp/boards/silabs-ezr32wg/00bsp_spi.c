

#include "stdint.h"
#include "board.h"
#include "spi.h"

#include "ecode.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
   uint8_t    txBuf[8];
   uint8_t    rxBuf[8];
} app_vars_t;

app_vars_t app_vars;



//=========================== prototypes ======================================

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
  
  /* HFXO 48MHz, divided by 1 */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  GPIO_PinModeSet( (GPIO_Port_TypeDef) RF_USARTRF_CS_PORT, RF_USARTRF_CS_PIN, gpioModePushPull, 1 );

   /* Setup enable and interrupt pins to radio */
   GPIO_PinModeSet( (GPIO_Port_TypeDef) RF_SDN_PORT, RF_SDN_PIN, gpioModePushPull,  0 );
   GPIO_PinModeSet( (GPIO_Port_TypeDef) RF_INT_PORT, RF_INT_PIN, gpioModeInputPull, 1 );
   
   /* Setup PRS for PTI pins */
   CMU_ClockEnable(cmuClock_PRS, true);

   /* Configure RF_GPIO0 and RF_GPIO1 to inputs. */
   GPIO_PinModeSet((GPIO_Port_TypeDef)RF_GPIO0_PORT, RF_GPIO0_PIN, gpioModeInput, 0);
   GPIO_PinModeSet((GPIO_Port_TypeDef)RF_GPIO1_PORT, RF_GPIO1_PIN, gpioModeInput, 0);

   /* Pin PA0 and PA1 output the GPIO0 and GPIO1 via PRS to PTI */
   GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0);
   GPIO_PinModeSet(gpioPortA, 1, gpioModePushPull, 0);

   /* Disable INT for PRS channels */
   GPIO_IntConfig((GPIO_Port_TypeDef)RF_GPIO0_PORT, RF_GPIO0_PIN, false, false, false);
   GPIO_IntConfig((GPIO_Port_TypeDef)RF_GPIO1_PORT, RF_GPIO1_PIN, false, false, false);

   /* Setup PRS for RF GPIO pins  */
   PRS_SourceAsyncSignalSet(0, PRS_CH_CTRL_SOURCESEL_GPIOH, PRS_CH_CTRL_SIGSEL_GPIOPIN15);
   PRS_SourceAsyncSignalSet(1, PRS_CH_CTRL_SOURCESEL_GPIOH, PRS_CH_CTRL_SIGSEL_GPIOPIN14);
   PRS->ROUTE = (PRS_ROUTE_CH0PEN | PRS_ROUTE_CH1PEN);

   /* Make sure PRS sensing is enabled (should be by default) */
   GPIO_InputSenseSet(GPIO_INSENSE_PRS, GPIO_INSENSE_PRS);
   
}