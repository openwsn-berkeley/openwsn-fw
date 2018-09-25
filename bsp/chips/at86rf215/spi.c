/**
\brief definition of the "spi" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, September 2017.
*/
#include "board.h"
#include "board_info.h"
#include "ssi.h"
#include "spi.h"
#include "gpio.h"
#include "ioc.h"
#include "sys_ctrl.h"

#include <headers/hw_ints.h>
#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>


//=========================== defines =========================================
#define SPI_PIN_SSI_CLK             GPIO_PIN_2      //    CLK
#define SPI_PIN_SSI_FSS             GPIO_PIN_3      //    CSn
#define SPI_PIN_SSI_RX              GPIO_PIN_4      //    MISO
#define SPI_PIN_SSI_TX              GPIO_PIN_5      //    MOSI
#define SPI_GPIO_SSI_BASE           GPIO_A_BASE

//=========================== variables =======================================

typedef struct {
    // information about the current transaction
    uint8_t*        pNextTxByte;
    uint16_t        numTxedBytes;
    uint16_t        txBytesLeft;
    spi_return_t    returnType;
    uint8_t*        pNextRxByte;
    uint16_t        maxRxBytes;
    spi_first_t     isFirst;
    spi_last_t      isLast;
    // state of the module
    uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
    // callback when module done
    spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

//=========================== prototypes ======================================
static void disableInterrupts(void);
static void enableInterrupts(void);
//=========================== public ==========================================

void spi_init() {
    // clear variables
    memset(&spi_vars,0,sizeof(spi_vars_t));

    // set the clk miso and cs pins as output
    GPIOPinTypeGPIOOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK);
    GPIOPinTypeGPIOOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX);
    GPIOPinTypeGPIOOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS);

    //set cs to high
    GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, SPI_PIN_SSI_FSS);
    //set pins to low
    GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX, 0);
    GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK, 0);

    SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_SSI0);
    SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_SSI0);
    SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI0);

    SSIDisable(SSI0_BASE);
    SSIClockSourceSet(SSI0_BASE, SSI_CLOCK_PIOSC);

    IOCPinConfigPeriphOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK, IOC_MUX_OUT_SEL_SSI0_CLKOUT);
    IOCPinConfigPeriphOutput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX, IOC_MUX_OUT_SEL_SSI0_TXD);
    IOCPinConfigPeriphInput(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_RX, IOC_SSIRXD_SSI0);

    GPIOPinTypeSSI(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_CLK );
    GPIOPinTypeSSI(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_RX );
    GPIOPinTypeSSI(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_TX );

    SSIConfigSetExpClk(SSI0_BASE, SysCtrlIOClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, /*SysCtrlIOClockGet()/2*/16000000, 8);

    // Enable the SSI0 module.
    SSIEnable(SSI0_BASE);
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCb(spi_cbt cb) {
   spi_vars.spi_cb = cb;
}
#endif

void    spi_txrx(uint8_t*     bufTx,
                 uint16_t     lenbufTx,
                 spi_return_t returnType,
                 uint8_t*     bufRx,
                 uint16_t     maxLenBufRx,
                 spi_first_t  isFirst,
                 spi_last_t   isLast) {

    uint32_t data,i;
    GPIOPinWrite(GPIO_B_BASE, GPIO_PIN_1, GPIO_PIN_1);
    // register spi frame to send
    spi_vars.pNextTxByte      =  bufTx;
    spi_vars.numTxedBytes     =  0;
    spi_vars.txBytesLeft      =  lenbufTx;
    spi_vars.returnType       =  returnType;
    spi_vars.pNextRxByte      =  bufRx;
    spi_vars.maxRxBytes       =  maxLenBufRx;
    spi_vars.isFirst          =  isFirst;
    spi_vars.isLast           =  isLast;

    // SPI is now busy
    spi_vars.busy             =  1;

    // lower CS signal to have slave listening
    if (spi_vars.isFirst==SPI_FIRST) {
       GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, 0);
    }

    for ( i =  0; i < lenbufTx; i++)
    {
        // Push a byte
        SSIDataPut(SSI0_BASE, spi_vars.pNextTxByte[i]);

        // Wait until it is complete
        while(SSIBusy(SSI0_BASE));

        // Read a byte
        SSIDataGet(SSI0_BASE, &data);

        // Store the result
        spi_vars.pNextRxByte[i] = (uint8_t)(data & 0xFF);
        // one byte less to go
     }

     if (spi_vars.isLast==SPI_LAST) {
        GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, SPI_PIN_SSI_FSS);
     }

    // SPI is not busy anymore
    spi_vars.busy             =  0;
    GPIOPinWrite(GPIO_B_BASE, GPIO_PIN_1, 0);
}

//=========================== private =========================================

port_INLINE void enableInterrupts(void)
{
    // Enable the SPI interrupt
    SSIIntEnable(SSI0_BASE, (SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR));

    // Enable the SPI interrupt
    IntEnable(INT_SSI0);
}

port_INLINE void disableInterrupts(void)
{
    // Disable the SPI interrupt
    SSIIntDisable(SSI0_BASE, (SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR));

    // Disable the SPI interrupt
    IntDisable(INT_SSI0);
}

//=========================== interrupt handlers ==============================

kick_scheduler_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE
    uint32_t data;
    // save the byte just received in the RX buffer
    status = SSIIntStatus(SSI0_BASE, true);

    // Clear SPI interrupt in the NVIC
    IntPendClear(INT_SSI0);

    SSIDataGet(SSI0_BASE, &data);

    // Store the result
    spi_vars.pNextRxByte = (uint8_t)(data & 0xFF);

    // one byte less to go
    spi_vars.pNextTxByte++;
    spi_vars.pNextRxByte++;
    spi_vars.txBytesLeft--;

    if (spi_vars.txBytesLeft>0) {
        // write next byte to TX buffer
        SSIDataPut(SSI0_BASE, *spi_vars.pNextTxByte);

    } else {
        // SPI is not busy anymore
        spi_vars.busy             =  0;

        // SPI is done!
        if (spi_vars.callback!=NULL) {
           // call the callback
            spi_vars.spi_cb();
            // kick the OS
            return KICK_SCHEDULER;
        }
    }

#endif
    return DO_NOT_KICK_SCHEDULER;
}
