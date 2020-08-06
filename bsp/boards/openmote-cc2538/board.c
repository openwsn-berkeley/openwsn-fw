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

#include <source/flash.h>
#include <source/interrupt.h>
#include <source/ioc.h>
#include <source/gpio.h>
#include <source/gptimer.h>
#include <source/sys_ctrl.h>

#include "config.h"
#include "board.h"
#include "debugpins.h"
#include "i2c.h"
#include "leds.h"
#include "radio.h"
#include "sensors.h"
#include "sctimer.h"
#include "uart.h"
#include "cryptoengine.h"
#include "pwm.h"

//=========================== variables =======================================

#define BSP_BUTTON_BASE                 ( GPIO_C_BASE )
#define BSP_BUTTON_USER                 ( GPIO_PIN_3 )

#ifdef REVA1 //Rev.A1 uses SF23 cc2538 which start at diffferent location
#define CC2538_FLASH_ADDRESS            ( 0x0023F800 )
#else
#define CC2538_FLASH_ADDRESS            ( 0x0027F800 )
#endif
//=========================== prototypes ======================================

void board_timer_init(void);

uint32_t board_timer_get(void);

bool board_timer_expired(uint32_t future);

static void clock_init(void);

static void gpio_init(void);

static void button_init(void);

static void SysCtrlDeepSleepSetting(void);

static void SysCtrlSleepSetting(void);

static void SysCtrlRunSetting(void);

static void SysCtrlWakeupSetting(void);

static void GPIO_C_Handler(void);

bool user_button_initialized;

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
    user_button_initialized = FALSE;

    gpio_init();
    clock_init();
    board_timer_init();
    leds_init();
    debugpins_init();
    button_init();
    sctimer_init();
    uart_init();
    radio_init();

    i2c_init();

#if defined(BOARD_CRYPTOENGINE_ENABLED)
    cryptoengine_init();
#endif


#if defined(BOARD_SENSORS_ENABLED)
    sensors_init();
#endif

   pwm_init();
   board_init_slot_vars();
}

//==== bootstrapping slot info lookup table
void board_init_slot_vars(void){
    //10ms slot
    slot_board_vars [SLOT_10ms].slotDuration                         = 328  ;  // ms 
    slot_board_vars [SLOT_10ms].maxTxDataPrepare                     = 10  ;  // 305us (measured  82us)
    slot_board_vars [SLOT_10ms].maxRxAckPrepare                      = 10  ;  // 305us (measured  83us)
    slot_board_vars [SLOT_10ms].maxRxDataPrepare                     =  4  ;  // 122us (measured  22us)
    slot_board_vars [SLOT_10ms].maxTxAckPrepare                      = 10  ;  // 122us (measured  94us)
    #ifdef L2_SECURITY_ACTIVE                                             
    slot_board_vars [SLOT_10ms].delayTx                              = 14  ;  // 366us (measured xxxus)
    #else                                                                 
    slot_board_vars [SLOT_10ms].delayTx                              = 12  ;  // 366us (measured xxxus)
    #endif                                                                
    slot_board_vars [SLOT_10ms].delayRx                              =  0  ;  // 0us (can not measure)

    // 20ms slot
    slot_board_vars [SLOT_20ms_24GHZ].slotDuration                   =  655   ; // ms  
    slot_board_vars [SLOT_20ms_24GHZ].maxTxDataPrepare               =  15   ;  // 457us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxAckPrepare                =  10   ; // 305us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxDataPrepare               =  10   ; // 305us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxTxAckPrepare                =  15   ; // 457us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].delayTx                        =  13   ; // 396us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].delayRx                        =  0    ; // 0us (can not measure)
}

// To get the current slotDuration at any time
// used during initialization by sixtop to fire the first sixtop EB
uint16_t board_getSlotDuration (void)
{
    return slot_board_vars [selected_slot_type].slotDuration;
}

// Setter/Getter function for slot_board_vars
slot_board_vars_t board_selectSlotTemplate (slotType_t slot_type)
{
  selected_slot_type = slot_type;
  return slot_board_vars [selected_slot_type];
}
/**
 * Puts the board to sleep
 */
void board_sleep(void) {
    SysCtrlPowerModeSet(SYS_CTRL_PM_NOACTION);
    SysCtrlSleep();
}

/**
 * Timer runs at 32 MHz and is 32-bit wide
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
void board_timer_init(void) {
    // Configure the timer
    TimerConfigure(GPTIMER2_BASE, GPTIMER_CFG_PERIODIC_UP);

    // Enable the timer
    TimerEnable(GPTIMER2_BASE, GPTIMER_BOTH);
}

/**
 * Returns the current value of the timer
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
uint32_t board_timer_get(void) {
    uint32_t current;

    current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;

    return current;
}

/**
 * Returns true if the timer has expired
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
bool board_timer_expired(uint32_t future) {
    uint32_t current;
    int32_t remaining;

    current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;

    remaining = (int32_t)(future - current);

    if (remaining > 0) {
        return false;
    } else {
        return true;
    }
}

/**
 * Resets the board
 */
void board_reset(void) {
    SysCtrlReset();
}

//=========================== private =========================================

static void gpio_init(void) {
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

static void clock_init(void) {
    /* Disable global interrupts */
    bool bIntDisabled = IntMasterDisable();

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

    /* Set the IO clock to operate at 16 MHz */
    /* This way peripherals can run while the system clock is gated */
    SysCtrlIOClockSet(SYS_CTRL_SYSDIV_16MHZ);

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
}

/**
 * Configures the user button as input source
 */
static void button_init(void) {
    volatile uint32_t i;

    /* Delay to avoid pin floating problems */
    for (i = 0xFFFF; i != 0; i--);

    GPIOPinIntDisable(BSP_BUTTON_BASE, BSP_BUTTON_USER);
    GPIOPinIntClear(BSP_BUTTON_BASE, BSP_BUTTON_USER);

    /* The button is an input GPIO on falling edge */
    GPIOPinTypeGPIOInput(BSP_BUTTON_BASE, BSP_BUTTON_USER);
    GPIOIntTypeSet(BSP_BUTTON_BASE, BSP_BUTTON_USER, GPIO_FALLING_EDGE);

    /* Register the interrupt */
    GPIOPortIntRegister(BSP_BUTTON_BASE, GPIO_C_Handler);

    /* Clear and enable the interrupt */
    GPIOPinIntClear(BSP_BUTTON_BASE, BSP_BUTTON_USER);
    GPIOPinIntEnable(BSP_BUTTON_BASE, BSP_BUTTON_USER);
    user_button_initialized = TRUE;
}

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
    SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT3);
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
    SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_GPT3);
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

/**
 * GPIO_C interrupt handler. User button is GPIO_C_3
 * Erases a Flash sector to trigger the bootloader backdoor
 */
static void GPIO_C_Handler(void) {
    if (!user_button_initialized) return;
    /* Disable the interrupts */
    IntMasterDisable();
    leds_all_off();

    /* Eras the CCA flash page */
    FlashMainPageErase(CC2538_FLASH_ADDRESS);

    leds_circular_shift();

    /* Reset the board */
    SysCtrlReset();
}
