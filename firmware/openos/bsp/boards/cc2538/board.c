/**
\brief CC2538-specific definition of the "board" bsp module.

\author Xavier Vilajosana <xvilajosana@eecs.berkeley.edu>, August 2013.
*/


#include "board.h"

// bsp modules
#include "leds.h"
#include "hw_ioc.h"             // Access to IOC register defines
#include "hw_ssi.h"             // Access to SSI register defines
#include "hw_sys_ctrl.h"        // Clocking control
#include "ioc.h"                // Access to driverlib ioc fns
#include "gpio.h"               // Access to driverlib gpio fns
#include "sys_ctrl.h"           // Access to driverlib SysCtrl fns
#include "interrupt.h"          // Access to driverlib interrupt fns
#include "bsp_timer.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "hw_types.h"
#include "hw_memmap.h"

//=========================== variables =======================================

#define BSP_RADIO_BASE              ( GPIO_D_BASE )
#define BSP_BUTTON_BASE             ( GPIO_C_BASE )
#define BSP_RADIO_INT               ( GPIO_PIN_5 )
#define BSP_RADIO_EXT               ( GPIO_PIN_4 )
#define BSP_USER_BUTTON             ( GPIO_PIN_3 )
//=========================== prototypes ======================================

void antenna_init();
void antenna_internal();
void antenna_external();
void button_init();

void GPIO_C_Isr_Handler();

static void clock_init(void);
static void gpio_init(void);

static void SysCtrlDeepSleepSetting(void);
static void SysCtrlSleepSetting(void);
static void SysCtrlRunSetting(void);
static void SysCtrlWakeupSetting(void);

//=========================== main ============================================

extern int mote_main();

int main() {
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

   leds_debug_on();
}

/**
 * Configures the used button as input source
 * Registers gpio_c interrupt..
 */
void button_init(){
	GPIOPinTypeGPIOInput(BSP_BUTTON_BASE, BSP_USER_BUTTON);
	GPIOIntTypeSet(BSP_BUTTON_BASE,BSP_USER_BUTTON,GPIO_FALLING_EDGE);
	GPIOPortIntRegister(BSP_BUTTON_BASE,GPIO_C_Isr_Handler);
	GPIOPinIntClear(BSP_BUTTON_BASE, BSP_USER_BUTTON);
	GPIOPinIntEnable(BSP_BUTTON_BASE, BSP_USER_BUTTON);
}

/**
 * GPIO_C ISR handler. User button is GPIO_C_3
 * Toggle debug led when user button is pressed
 */
void GPIO_C_Isr_Handler(){

	GPIOPinIntClear(GPIO_C_BASE, BSP_USER_BUTTON);
	leds_debug_toggle();//toggle led.
}

/**
 * Configures the antenna using a RF switch
 * INT is the internal antenna (chip) configured through ANT1_SEL (V1)
 * EXT is the external antenna (connector) configured through ANT2_SEL (V2)
 */
void antenna_init(void) {
    // Configure the ANT1 and ANT2 GPIO as output
    GPIOPinTypeGPIOOutput(BSP_RADIO_BASE, BSP_RADIO_INT);
    GPIOPinTypeGPIOOutput(BSP_RADIO_BASE, BSP_RADIO_EXT);

    // By default the chip antenna is selected as the default
    GPIOPinWrite(BSP_RADIO_BASE, BSP_RADIO_INT, BSP_RADIO_INT);
    GPIOPinWrite(BSP_RADIO_BASE, BSP_RADIO_EXT, ~BSP_RADIO_EXT);
}

/**
 * Selects the external (connector) antenna
 */
void antenna_external(void) {
    GPIOPinWrite(BSP_RADIO_BASE, BSP_RADIO_EXT, BSP_RADIO_EXT);
    GPIOPinWrite(BSP_RADIO_BASE, BSP_RADIO_INT, ~BSP_RADIO_INT);
}

/**
 * Selects the internal (chip) antenna
 */
void antenna_internal(void) {
    GPIOPinWrite(BSP_RADIO_BASE, BSP_RADIO_EXT, ~BSP_RADIO_EXT);
    GPIOPinWrite(BSP_RADIO_BASE, BSP_RADIO_INT, BSP_RADIO_INT);
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
