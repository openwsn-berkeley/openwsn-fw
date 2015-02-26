/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "board" bsp module.
 */

#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include "board.h"
#include "leds.h"
#include "ioc.h"
#include "gpio.h"
#include "sys_ctrl.h"
#include "interrupt.h"
#include "bsp_timer.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "flash.h"
#include "i2c.h"
#include "sensors.h"

//=========================== variables =======================================

#define BSP_ANTENNA_BASE                ( GPIO_D_BASE )
#define BSP_ANTENNA_INT                 ( GPIO_PIN_5 )
#define BSP_ANTENNA_EXT                 ( GPIO_PIN_4 )

#define BSP_BUTTON_BASE                 ( GPIO_C_BASE )
#define BSP_BUTTON_USER                 ( GPIO_PIN_3 )

#define CC2538_FLASH_ADDRESS            ( 0x0027F800 )

//=========================== prototypes ======================================

void antenna_init(void);
void antenna_internal(void);
void antenna_external(void);

void button_init(void);

void GPIO_C_Isr_Handler(void);

static void clock_init(void);
static void gpio_init(void);

static void SysCtrlDeepSleepSetting(void);
static void SysCtrlSleepSetting(void);
static void SysCtrlRunSetting(void);
static void SysCtrlWakeupSetting(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init() {
   gpio_init();
   clock_init();

   antenna_init();
   antenna_external();

   leds_init();
   debugpins_init();
   button_init();
   bsp_timer_init();
   radiotimer_init();
   uart_init();
   radio_init();
   i2c_init();
   sensors_init();
}

/**
 * Configures the user button as input source
 */
void button_init(){
	GPIOPinTypeGPIOInput(BSP_BUTTON_BASE, BSP_BUTTON_USER);
	GPIOIntTypeSet(BSP_BUTTON_BASE, BSP_BUTTON_USER,GPIO_FALLING_EDGE);
	GPIOPortIntRegister(BSP_BUTTON_BASE, GPIO_C_Isr_Handler);
	GPIOPinIntClear(BSP_BUTTON_BASE, BSP_BUTTON_USER);
	GPIOPinIntEnable(BSP_BUTTON_BASE, BSP_BUTTON_USER);
}

/**
 * GPIO_C ISR handler. User button is GPIO_C_3
 * Erases a Flash sector to trigger the bootloader backdoor
 */
void GPIO_C_Isr_Handler(){
    IntMasterDisable();
    FlashMainPageErase(CC2538_FLASH_ADDRESS);
    SysCtrlReset();
}

/**
 * Configures the antenna using a RF switch
 * INT is the internal antenna (chip) configured through ANT1_SEL (V1)
 * EXT is the external antenna (connector) configured through ANT2_SEL (V2)
 */
void antenna_init(void) {
    // Configure the ANT1 and ANT2 GPIO as output
    GPIOPinTypeGPIOOutput(BSP_ANTENNA_BASE, BSP_ANTENNA_INT);
    GPIOPinTypeGPIOOutput(BSP_ANTENNA_BASE, BSP_ANTENNA_EXT);

    // By default the chip antenna is selected as the default
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_INT, BSP_ANTENNA_INT);
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_EXT, ~BSP_ANTENNA_EXT);
}

/**
 * Selects the external (connector) antenna
 */
void antenna_external(void) {
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_EXT, BSP_ANTENNA_EXT);
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_INT, ~BSP_ANTENNA_INT);
}

/**
 * Selects the internal (chip) antenna
 */
void antenna_internal(void) {
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_EXT, ~BSP_ANTENNA_EXT);
    GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_INT, BSP_ANTENNA_INT);
}

/**
 * Puts the board to sleep
 */
void board_sleep() {
    SysCtrlPowerModeSet(SYS_CTRL_PM_NOACTION);
    SysCtrlSleep();
}

/**
 * Resets the board
 */
void board_reset() {
	SysCtrlReset();
}

//=========================== private =========================================

static void gpio_init(void)
{
    /* Set GPIOs as output */
    GPIOPinTypeGPIOOutput(GPIO_A_BASE, 0xFF);
    GPIOPinTypeGPIOOutput(GPIO_B_BASE, 0xFF);
    GPIOPinTypeGPIOOutput(GPIO_C_BASE, 0xFF);
    GPIOPinTypeGPIOOutput(GPIO_D_BASE, 0xFF);

    /* Initialize GPIOs to low */
    GPIOPinWrite(GPIO_A_BASE, 0xFF, 0x00);
    GPIOPinWrite(GPIO_B_BASE, 0xFF, 0x00);
    GPIOPinWrite(GPIO_C_BASE, 0xFF, 0x00);
    GPIOPinWrite(GPIO_D_BASE, 0xFF, 0x00);
}

static void clock_init(void)
{
    /**
     * Disable global interrupts
     */
    bool bIntDisabled = IntMasterDisable();

    /**
     * Configure the 32 kHz pins, PD6 and PD7, for crystal operation
     * By default they are configured as GPIOs
     */
    GPIODirModeSet(GPIO_D_BASE, 0x40, GPIO_DIR_MODE_IN);
    GPIODirModeSet(GPIO_D_BASE, 0x80, GPIO_DIR_MODE_IN);
    IOCPadConfigSet(GPIO_D_BASE, 0x40, IOC_OVERRIDE_ANA);
    IOCPadConfigSet(GPIO_D_BASE, 0x80, IOC_OVERRIDE_ANA);

    /**
     * Set the real-time clock to use the 32khz internal crystal
     * Set the system clock to use the external 32 MHz crystal
     * Set the system clock to 32 MHz
     */
    SysCtrlClockSet(true, false, SYS_CTRL_SYSDIV_32MHZ);

    /**
     * Set the IO clock to operate at 16 MHz
     * This way peripherals can run while the system clock is gated
     */
    SysCtrlIOClockSet(SYS_CTRL_SYSDIV_16MHZ);

    /**
     * Wait until the selected clock configuration is stable
     */
    while (!((HWREG(SYS_CTRL_CLOCK_STA)) & (SYS_CTRL_CLOCK_STA_XOSC_STB)));

    /**
     * Define what peripherals run in each mode
     */
    SysCtrlDeepSleepSetting();
    SysCtrlSleepSetting();
    SysCtrlRunSetting();
    SysCtrlWakeupSetting();

    /**
     * Re-enable interrupt if initially enabled.
     */
    if(!bIntDisabled)
    {
        IntMasterEnable();
    }
}


static void SysCtrlDeepSleepSetting(void)
{
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

static void SysCtrlSleepSetting(void)
{
  /* Disable General Purpose Timers 0, 1, 2, 3 during sleep */
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT2);
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
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_RFC);
}


void SysCtrlRunSetting(void)
{
  /* Disable General Purpose Timers 0, 1, 2, 3 when running */
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT2);
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
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_RFC);
}


void SysCtrlWakeupSetting(void)
{
  /* SM Timer can wake up the processor */
  GPIOIntWakeupEnable(GPIO_IWE_SM_TIMER);
}
