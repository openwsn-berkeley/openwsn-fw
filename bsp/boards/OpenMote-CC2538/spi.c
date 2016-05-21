/**
 * Author: Pere Tuset (peretuset@openmote.com)
 *         Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 * Date:   May 2016
 * Description: CC2538-specific definition of the "spi" bsp module.
 */

#include <headers/hw_ints.h>
#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_types.h>
#include <headers/hw_ssi.h>
#include <source/ssi.h>

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "spi.h"
#include "interrupt.h"
#include "sys_ctrl.h"
#include "gpio.h"
#include "board.h"
#include "ioc.h"
#include "debugpins.h"

//=========================== defines =========================================

#define SPI_PERIPHERAL          ( SYS_CTRL_PERIPH_SSI0 )
#define SPI_BASE                ( SSI0_BASE )
#define SPI_CLOCK               ( SSI_CLOCK_PIOSC )
#define SPI_INT                 ( INT_SSI0 )
#define SPI_MODE                ( SSI_MODE_MASTER )
#define SPI_PROTOCOL            ( SSI_FRF_MOTO_MODE_0 )
#define SPI_DATAWIDTH           ( 8 )
#define SPI_BAUDRATE            ( 8000000 )

#define SPI_MISO_BASE           ( GPIO_A_BASE )
#define SPI_MISO_PIN            ( GPIO_PIN_4 )
#define SPI_MISO_IOC            ( IOC_SSIRXD_SSI0 )
#define SPI_MOSI_BASE           ( GPIO_A_BASE )
#define SPI_MOSI_PIN            ( GPIO_PIN_5 )
#define SPI_MOSI_IOC            ( IOC_MUX_OUT_SEL_SSI0_TXD )
#define SPI_CLK_BASE            ( GPIO_A_BASE )
#define SPI_CLK_PIN             ( GPIO_PIN_2 )
#define SPI_CLK_IOC             ( IOC_MUX_OUT_SEL_SSI0_CLKOUT )

#define SPI_nCS_BASE            ( GPIO_A_BASE )
#define SPI_nCS_PIN             ( GPIO_PIN_3 )

//=========================== variables =======================================

typedef struct {
   uint8_t* pNextTxByte;
   uint8_t  numTxedBytes;
   uint8_t  txBytesLeft;
   uint8_t* pNextRxByte;
   uint8_t  maxRxBytes;
   uint8_t  busy;
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================

static void select(void);
static void deselect(void);

//=========================== public ==========================================

void spi_init(void) {   
  // Enable peripheral except in deep sleep modes (e.g. LPM1, LPM2, LPM3)
  SysCtrlPeripheralEnable(SPI_PERIPHERAL);
  SysCtrlPeripheralSleepEnable(SPI_PERIPHERAL);
  SysCtrlPeripheralDeepSleepDisable(SPI_PERIPHERAL);

  // Reset peripheral previous to configuring it
  SSIDisable(SPI_BASE);

  // Set IO clock as SPI0 clock source
  SSIClockSourceSet(SPI_BASE, SPI_CLOCK);

  // Configure the MISO, MOSI, CLK and nCS pins as peripheral
  IOCPinConfigPeriphInput(SPI_MISO_BASE, SPI_MISO_PIN, SPI_MISO_IOC);
  IOCPinConfigPeriphOutput(SPI_MOSI_BASE, SPI_MOSI_PIN, SPI_MOSI_IOC);
  IOCPinConfigPeriphOutput(SPI_CLK_BASE, SPI_CLK_PIN, SPI_CLK_IOC);

  // Configure MISO, MOSI, CLK and nCS GPIOs
  GPIOPinTypeSSI(SPI_MISO_BASE, SPI_MISO_PIN);
  GPIOPinTypeSSI(SPI_MOSI_BASE, SPI_MOSI_PIN);
  GPIOPinTypeSSI(SPI_CLK_BASE, SPI_CLK_PIN);

  // Set the nCS pin as output low
  GPIOPinTypeGPIOOutput(SPI_nCS_BASE, SPI_nCS_PIN);
  if (SPI_PROTOCOL == SSI_FRF_MOTO_MODE_0 ||
    SPI_PROTOCOL == SSI_FRF_MOTO_MODE_1) {
    GPIOPinWrite(SPI_nCS_BASE, SPI_nCS_PIN, SPI_nCS_PIN);
  } else {
    GPIOPinWrite(SPI_nCS_BASE, SPI_nCS_PIN, 0);
  }

  // Configure the SPI0 clock
  SSIConfigSetExpClk(SPI_BASE, SysCtrlIOClockGet(), SPI_PROTOCOL, \
                     SPI_MODE, SPI_BAUDRATE, SPI_DATAWIDTH);

  // Enable the SPI0 module
  SSIEnable(SPI_BASE);
}

void spi_setCallback(spi_cbt cb) {
}

void spi_txrx(uint8_t*     bufTx,
              uint8_t      lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint8_t      maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {
  uint32_t data;

  // Register SPI frame to send
  spi_vars.pNextTxByte  = bufTx;
  spi_vars.numTxedBytes = 0;
  spi_vars.txBytesLeft  = lenbufTx;
  spi_vars.pNextRxByte  = bufRx;
  spi_vars.maxRxBytes   = maxLenBufRx;

  // SPI is now busy
  spi_vars.busy = 1;

  // Select the SPI device
  select();

  // Wait until all bytes are transmitted
  while (spi_vars.txBytesLeft > 0) {
    // Push a byte
    SSIDataPut(SPI_BASE, *spi_vars.pNextTxByte++);

    // Wait until it is complete
    while (SSIBusy(SPI_BASE))
      ;

    // Read a byte
    SSIDataGet(SPI_BASE, &data);

    // Save the byte in the buffer, but
    // skip the first one since it's nonsense
    if (spi_vars.numTxedBytes > 0) {
      *spi_vars.pNextRxByte++ = data;  
    }

    // one byte less to go
    spi_vars.numTxedBytes++;
    spi_vars.txBytesLeft--;
  }

  // Unselect the SPI device
  deselect();

  // SPI is not busy anymore
  spi_vars.busy = 0;
}

//=========================== private =========================================

static void select(void) {
  if (SPI_PROTOCOL == SSI_FRF_MOTO_MODE_0 ||
      SPI_PROTOCOL == SSI_FRF_MOTO_MODE_1) {
    GPIOPinWrite(SPI_nCS_BASE, SPI_nCS_PIN, 0);
  } else {
    GPIOPinWrite(SPI_nCS_BASE, SPI_nCS_PIN, SPI_nCS_PIN);
  }
}

static void deselect(void) {
  if (SPI_PROTOCOL == SSI_FRF_MOTO_MODE_0 ||
      SPI_PROTOCOL == SSI_FRF_MOTO_MODE_1) {
    GPIOPinWrite(SPI_nCS_BASE, SPI_nCS_PIN, SPI_nCS_PIN);
  } else {
    GPIOPinWrite(SPI_nCS_BASE, SPI_nCS_PIN, 0);
  }
}

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr(void) {
  return DO_NOT_KICK_SCHEDULER;
}
