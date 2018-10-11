/**
 * Author: Tengfei Chang(tengfei.chang@inria.fr)
 * Date:   October 2018
 * Description: standalone project to test at86rf215 radio module.
 */

#include "hw_ioc.h"
#include "hw_memmap.h"
#include "hw_ssi.h"
#include "hw_sys_ctrl.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"

#include "flash.h"
#include "interrupt.h"
#include "ioc.h"
#include "gpio.h"
#include "gptimer.h"
#include "uarthal.h"
#include "sys_ctrl.h"
#include "ssi.h"

//=========================== defines =======================================

#define BSP_ANTENNA_BASE            GPIO_D_BASE
#define BSP_ANTENNA_CC2538_24GHZ    GPIO_PIN_4      //!< PD4 -- 2.4ghz
#define BSP_ANTENNA_AT215_24GHZ     GPIO_PIN_3      //!< PD3 -- subghz

#define PIN_UART_RXD                GPIO_PIN_0      // PA0 is UART RX
#define PIN_UART_TXD                GPIO_PIN_1      // PA1 is UART TX

#define SPI_PIN_SSI_CLK             GPIO_PIN_2      //    CLK
#define SPI_PIN_SSI_FSS             GPIO_PIN_3      //    CSn
#define SPI_PIN_SSI_RX              GPIO_PIN_4      //    MISO
#define SPI_PIN_SSI_TX              GPIO_PIN_5      //    MOSI
#define SPI_GPIO_SSI_BASE           GPIO_A_BASE

#define FLAG_WRITE        0x80
#define FLAG_READ         0x00

#define RG_RF_PN                    (0x0D)
#define RG_RF_VN                    (0x0E)

#define RG_RF_RST         0x05
#define CMD_RF_RESET      0x7

#define AT86RF215_PART_NUMBER       0x34
#define AT86RF215_VERSION_NUMBER    0x03        // the value can be 0x01 or 0x03

typedef enum {
   SPI_FIRSTBYTE        = 0,
   SPI_BUFFER           = 1,
   SPI_LASTBYTE         = 2,
} spi_return_t;

typedef enum {
   SPI_NOTFIRST         = 0,
   SPI_FIRST            = 1,
} spi_first_t;

typedef enum {
   SPI_NOTLAST          = 0,
   SPI_LAST             = 1,
} spi_last_t;

//=========================== prototypes ======================================

static void SysCtrlDeepSleepSetting(void);
static void SysCtrlSleepSetting(void);
static void SysCtrlRunSetting(void);
static void SysCtrlWakeupSetting(void);

uint8_t at86rf215_spiReadReg(uint16_t regAddr16);
void at86rf215_spiWriteReg(uint16_t reg, uint8_t regValueToWrite);

//=========================== main ============================================

int main(void) {

    bool bIntDisabled;
    volatile uint32_t delay;
    uint8_t partNumber;
    uint8_t versionNumber;
    uint16_t i,j;

    //======================= gpio init =======================================
    /* Configure all GPIO as input */
    GPIOPinTypeGPIOInput(GPIO_A_BASE, 0xFF);
    GPIOPinTypeGPIOInput(GPIO_B_BASE, 0xFF);
    GPIOPinTypeGPIOInput(GPIO_C_BASE, 0xFF);
    GPIOPinTypeGPIOInput(GPIO_D_BASE, 0xFF);

    //======================= clock init ======================================

    bIntDisabled = IntMasterDisable();

    /* Configure the 32 kHz pins, PD6 and PD7, for crystal operation */
    /* By default they are configured as GPIOs */
    GPIODirModeSet(GPIO_D_BASE, 0x40, GPIO_DIR_MODE_IN);
    GPIODirModeSet(GPIO_D_BASE, 0x80, GPIO_DIR_MODE_IN);
    IOCPadConfigSet(GPIO_D_BASE, 0x40, IOC_OVERRIDE_ANA);
    IOCPadConfigSet(GPIO_D_BASE, 0x80, IOC_OVERRIDE_ANA);

    /* Set the real-time clock to use the 32 kHz external crystal */
    /* Set the system clock to use the external 32 MHz crystal */
    /* Set the system clock to 32 MHz */
    SysCtrlClockSet(true, false, SYS_CTRL_SYSDIV_32MHZ);

    /* Set the IO clock to operate at 32 MHz */
    /* This way peripherals can run while the system clock is gated */
    SysCtrlIOClockSet(SYS_CTRL_SYSDIV_32MHZ);

    /* Wait until the selected clock configuration is stable */
    while (!((HWREG(SYS_CTRL_CLOCK_STA)) & (SYS_CTRL_CLOCK_STA_XOSC_STB)));

    /* Define what peripherals run in each mode */
    SysCtrlRunSetting();
    SysCtrlSleepSetting();
    SysCtrlDeepSleepSetting();
    SysCtrlWakeupSetting();

    /* Re-enable interrupt if initially enabled */
    if (!bIntDisabled) {
        IntMasterEnable();
    }

    //======================== antenna init ===================================
    /* Configure GPIO as output */
    GPIOPinTypeGPIOOutput(BSP_ANTENNA_BASE, BSP_ANTENNA_CC2538_24GHZ);
    GPIOPinTypeGPIOOutput(BSP_ANTENNA_BASE, BSP_ANTENNA_AT215_24GHZ);

    /* Use CC2538 antenna by default */
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_CC2538_24GHZ, 0);
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_AT215_24GHZ, BSP_ANTENNA_AT215_24GHZ);

    //========================== uart init ====================================
    // Disable UART function
    UARTDisable(UART0_BASE);

    // Disable all UART module interrupts
    UARTIntDisable(UART0_BASE, 0x1FFF);

    // Set IO clock as UART clock source
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Map UART signals to the correct GPIO pins and configure them as
    // hardware controlled. GPIO-A pin 0 and 1
    IOCPinConfigPeriphOutput(GPIO_A_BASE, PIN_UART_TXD, IOC_MUX_OUT_SEL_UART0_TXD);
    GPIOPinTypeUARTOutput(GPIO_A_BASE, PIN_UART_TXD);
    IOCPinConfigPeriphInput(GPIO_A_BASE, PIN_UART_RXD, IOC_UARTRXD_UART0);
    GPIOPinTypeUARTInput(GPIO_A_BASE, PIN_UART_RXD);

    // Configure the UART for 115,200, 8-N-1 operation.
    // This function uses SysCtrlClockGet() to get the system clock
    // frequency.  This could be also be a variable or hard coded value
    // instead of a function call.
    UARTConfigSetExpClk(UART0_BASE, SysCtrlIOClockGet(), 115200,
                  (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                   UART_CONFIG_PAR_NONE));

    // Enable UART hardware
    UARTEnable(UART0_BASE);

    // Disable FIFO as we only one 1byte buffer
    UARTFIFODisable(UART0_BASE);

    //========================== spi init =====================================

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

    //============== get radio device part number and version number ==========
    //==== power on radio

    GPIOPinTypeGPIOOutput(GPIO_C_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(GPIO_D_BASE, GPIO_PIN_1);

    //set radio pwr off
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, 0);
    GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, 0);
    for(delay=0;delay<0xA2C2;delay++);

    //init the radio, pwr up the radio
    GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, GPIO_PIN_0);
    for(delay=0;delay<0xA2C2;delay++);

    //reset the radio
    GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, GPIO_PIN_1);

    at86rf215_spiWriteReg(RG_RF_RST, CMD_RF_RESET);

    //==== get radio device part number and version number
    partNumber    = at86rf215_spiReadReg(RG_RF_PN);
    versionNumber = at86rf215_spiReadReg(RG_RF_VN);

    while(1){
        //==== return the result
        if (partNumber == AT86RF215_PART_NUMBER){
            UARTCharPut(UART0_BASE, 'P');
        } else {
            UARTCharPut(UART0_BASE, 'F');
        }

        if (versionNumber == AT86RF215_VERSION_NUMBER){
            UARTCharPut(UART0_BASE, 'P');
        } else {
            UARTCharPut(UART0_BASE, 'F');
        }
        UARTCharPut(UART0_BASE, ' ');

        for(i=0;i<0xff;i++){
            for(j=0;j<0xfff;j++);
        }
    }
}

void spi_txrx(uint8_t*     bufTx,
              uint16_t     lenbufTx,
              spi_return_t returnType,
              uint8_t*     bufRx,
              uint16_t     maxLenBufRx,
              spi_first_t  isFirst,
              spi_last_t   isLast) {

    uint32_t data,i;

    GPIOPinWrite(GPIO_B_BASE, GPIO_PIN_1, GPIO_PIN_1);

    // lower CS signal to have slave listening
    if (isFirst==SPI_FIRST) {
       GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, 0);
    }

    for (i=0; i<lenbufTx; i++) {
        // Push a byte
        SSIDataPut(SSI0_BASE, bufTx[i]);
        // Wait until it is complete
        while(SSIBusy(SSI0_BASE));
        // Read a byte
        SSIDataGet(SSI0_BASE, &data);
        // Store the result
        bufRx[i] = (uint8_t)(data & 0xFF);
        // one byte less to go
    }

    if (isLast==SPI_LAST) {
        GPIOPinWrite(SPI_GPIO_SSI_BASE, SPI_PIN_SSI_FSS, SPI_PIN_SSI_FSS);
    }

    GPIOPinWrite(GPIO_B_BASE, GPIO_PIN_1, 0);
}

void at86rf215_spiWriteReg(uint16_t reg, uint8_t regValueToWrite) {

    uint8_t spi_rx_buffer[3];
    uint8_t spi_tx_buffer[3];

    spi_tx_buffer[0]     = (FLAG_WRITE | (uint8_t)((reg)/256));
    spi_tx_buffer[1]     = (uint8_t)((reg)%256);
    spi_tx_buffer[2]     = regValueToWrite;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        3,                          // lenbufTx
        SPI_FIRSTBYTE,              // returnType
        spi_rx_buffer,              // bufRx
        3,                          // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
}

uint8_t at86rf215_spiReadReg(uint16_t regAddr16) {

    uint8_t              spi_tx_buffer[3];
    uint8_t              spi_rx_buffer[3];

    spi_tx_buffer[0] = (FLAG_READ | (uint8_t)(regAddr16 >> 8));
    spi_tx_buffer[1] = (uint8_t)(regAddr16 & 0xFF);
    spi_tx_buffer[2] = 0x00;

    spi_txrx(
        spi_tx_buffer,              // bufTx
        sizeof(spi_tx_buffer),      // lenbufTx
        SPI_BUFFER,                 // returnType
        spi_rx_buffer,              // bufRx
        sizeof(spi_rx_buffer),      // maxLenBufRx
        SPI_FIRST,                  // isFirst
        SPI_LAST                    // isLast
    );
    return spi_rx_buffer[2];
}

//=========================== private =========================================

static void SysCtrlRunSetting(void) {
  /* Disable General Purpose Timers 0, 1, 2, 3 when running */
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT3);

  /* Disable SSI 0, 1 when running */
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_SSI1);

  /* Disable UART1 when running */
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_UART1);

  /* Disable I2C, AES and PKA when running */
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_AES);

  /* Enable UART0 and RFC when running */
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_RFC);
}

static void SysCtrlSleepSetting(void) {
  /* Disable General Purpose Timers 0, 1, 2, 3 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT3);

  /* Disable SSI 0, 1 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_SSI1);

  /* Disable UART 0, 1 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_UART1);

  /* Disable I2C, PKA, AES during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_AES);

  /* Enable UART and RFC during sleep */
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_RFC);
}

static void SysCtrlDeepSleepSetting(void) {
  /* Disable General Purpose Timers 0, 1, 2, 3 during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT3);

  /* Disable SSI 0, 1 during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI1);

  /* Disable UART 0, 1 during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_UART1);

  /* Disable I2C, PKA, AES during deep sleep */
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_AES);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_RFC);
}

static void SysCtrlWakeupSetting(void) {
  /* Allow the SMTimer to wake up the processor */
  GPIOIntWakeupEnable(GPIO_IWE_SM_TIMER);
}

//=========================== interrupt handlers ==============================
