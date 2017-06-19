#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "em_device.h"
#include "em_gpio.h"
#include "em_int.h"
#include "em_usart.h"
#include "ustimer.h"

#include "dmadrv.h"
#include "spi.h"
#include "ezradio_hal.h"
#include "ezradio_comm.h"
#include "ezradio_cmd.h"
#include "ezradio_prop.h"
#include "gpiointerrupt.h"
#include "ezradio_api_lib.h"
#include "em_cmu.h"
#include "em_prs.h"
#include "radio-config-wds-gen.h"
#include "spidrv.h"


#include "ecode.h"
#include "spidrv_config.h"

/* Radio configuration data array. */
static const uint8_t Radio_Configuration_Data_Array[]  = \
                        RADIO_CONFIGURATION_DATA_ARRAY;

static SPIDRV_HandleData_t  ezradioSpiHandle;
static SPIDRV_Handle_t      ezradioSpiHandlePtr = &ezradioSpiHandle;

static SPIDRV_Init_t        ezradioSpiInitData = SPIDRV_MASTER_USARTRF0;

/*************************************************************************
*
*       INSTANCES and default configurations
*/

typedef struct {
   uint8_t* pNextTxByte;
   uint8_t  numTxedBytes;
   uint8_t  txBytesLeft;
   uint8_t* pNextRxByte;
   uint8_t  maxRxBytes;
   uint8_t  busy;
} spi_vars_t;

spi_vars_t spi_vars;

/* Radio interrupt receive flag */
static bool    ezradioIrqReceived = false;

static bool     spidrvIsInitialized = false;

static void ezradioPowerUp(void);

static void GPIO_EZRadio_INT_IRQHandler( uint8_t pin );

/*****************************************************************
SPI instance initialisation


*/

void spi_init(){

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
   
   //TO BE ENABLED: /* Register callback and enable interrupt */
    GPIOINT_CallbackRegister( RF_INT_PIN, GPIO_EZRadio_INT_IRQHandler );
    GPIO_IntConfig( (GPIO_Port_TypeDef) RF_INT_PORT, RF_INT_PIN, false, true, true ); 
        
    ezradio_hal_SpiInit();
    
    /* Power Up the radio chip */
    ezradioPowerUp();  
}

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast){
                

                switch (returnType){
                case 0: 
                  ezradio_comm_GetResp(maxLenBufRx, bufRx); 
                  break;
                case 1:
                  ezradio_comm_SendCmd(lenbufTx, bufTx);
                  break;
                case 2:
                  if (isFirst == 0)
                    ezradio_comm_ReadData( EZRADIO_CMD_ID_READ_RX_FIFO, 0, maxLenBufRx, bufRx);
                  else 
                    ezradio_comm_WriteData( EZRADIO_CMD_ID_WRITE_TX_FIFO, 0, lenbufTx, bufTx);
                  break;
                }
              }

static void ezradioPowerUp(void)
{
  /* Hardware reset the chip */
  ezradio_reset();

  /* Initialize ustimer */
  USTIMER_Init();
  /* Delay for preconfigured time */
  USTIMER_Delay( /*RADIO_CONFIG_DATA_RADIO_DELAY_AFTER_RESET_US*/ 10000 );
  /* Deinit ustimer */
  USTIMER_DeInit();
}

static void GPIO_EZRadio_INT_IRQHandler( uint8_t pin )
{
  (void)pin;

  /* Sign radio interrupt received */
  ezradioIrqReceived = true;
}

