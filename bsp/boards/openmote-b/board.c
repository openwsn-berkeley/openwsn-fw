
 // Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 //         Pere Tuset (peretuset@openmote.com)
 // Date:   July 2013
 // Description: CC2538-specific definition of the "board" bsp module.


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

#include "board.h"
#include "board_info.h"
#include "debugpins.h"
#include "i2c.h"
#include "leds.h"
#include "radio.h"
#include "sensors.h"
#include "sctimer.h"
#include "uart.h"
#include "cryptoengine.h"

//=========================== defines =======================================

#define BSP_BUTTON_BASE                 ( GPIO_D_BASE )
#define BSP_BUTTON_USER                 ( GPIO_PIN_5 )

//=========================== variables ======================================

//=========================== prototypes ======================================

void board_timer_init(void);
uint32_t board_timer_get(void);
bool board_timer_expired(uint32_t future);

static void clock_init(void);
static void gpio_init(void);
static void button_init(void);

static void antenna_init(void);
void antenna_cc2538(void);
void antenna_at86rf215(void);

static void SysCtrlDeepSleepSetting(void);
static void SysCtrlSleepSetting(void);
static void SysCtrlRunSetting(void);
static void SysCtrlWakeupSetting(void);

//=========================== variables =======================================
slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
slotType_t selected_slot_type;

//=========================== main ============================================
   
extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {

    gpio_init();
    clock_init();
    antenna_init();
    board_timer_init();
    leds_init();
    debugpins_init();
    button_init();
    sctimer_init();
    uart_init();
    radio_init();
    i2c_init();
    board_init_slot_vars();
    // sensors_init();
    cryptoengine_init();
}

void antenna_init(void) {
    // Configure GPIO as output 
    GPIOPinTypeGPIOOutput(BSP_ANTENNA_BASE, BSP_ANTENNA_CC2538_24GHZ);
    GPIOPinTypeGPIOOutput(BSP_ANTENNA_BASE, BSP_ANTENNA_AT215_24GHZ);

#ifdef ATMEL_24GHZ
    // atmel is using 2.4ghz, connect antenna to atrf215
    antenna_at86rf215();
#else
    // atmel is not using 2.4ghz, connect antenna to cc2538
    antenna_cc2538();
#endif
}

void antenna_cc2538(void) {
  GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_CC2538_24GHZ, 0);
  GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_AT215_24GHZ, BSP_ANTENNA_AT215_24GHZ);
}

void antenna_at86rf215(void) {
  GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_AT215_24GHZ, 0);
  GPIOPinWrite(BSP_ANTENNA_BASE, BSP_ANTENNA_CC2538_24GHZ, BSP_ANTENNA_CC2538_24GHZ);
}


 // Puts the board to sleep
 
void board_sleep(void) {
    SysCtrlPowerModeSet(SYS_CTRL_PM_NOACTION);
    SysCtrlSleep();
}


 // Timer runs at 32 MHz and is 32-bit wide
 // The timer is divided by 32, whichs gives a 1 microsecond ticks

void board_timer_init(void) {
    //Configure the timer 
    TimerConfigure(GPTIMER2_BASE, GPTIMER_CFG_PERIODIC_UP);

    // Enable the timer
    TimerEnable(GPTIMER2_BASE, GPTIMER_BOTH);
}


 // Returns the current value of the timer
 // The timer is divided by 32, whichs gives a 1 microsecond ticks

uint32_t board_timer_get(void) {
    uint32_t current;

    //Get the current timer value
    current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;

    return current;
}


 // Returns true if the timer has expired
 // The timer is divided by 32, whichs gives a 1 microsecond ticks

bool board_timer_expired(uint32_t future) {
    uint32_t current;
    int32_t remaining;

    //Get current time
    current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;

    //Calculate remaining time
    remaining = (int32_t) (future - current);

    //Return if timer has expired
    if (remaining > 0) {
        return false;
    } else {
        return true;
    }
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
    slot_board_vars [SLOT_20ms_24GHZ].slotDuration                   =  655  ; // ms  
    slot_board_vars [SLOT_20ms_24GHZ].maxTxDataPrepare               =  15   ;  // 457us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxAckPrepare                =  10   ; // 305us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxRxDataPrepare               =  10   ; // 305us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].maxTxAckPrepare                =  15   ; // 457us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].delayTx                        =  6    ; // 183us (based on measurement)
    slot_board_vars [SLOT_20ms_24GHZ].delayRx                        =  0    ; // 0us (can not measure)
    
    //40ms slot for 24ghz cc2538
    slot_board_vars [SLOT_40ms_24GHZ].slotDuration                   =   1310 ;  // ms 
    slot_board_vars [SLOT_40ms_24GHZ].maxTxDataPrepare               =   15  ;  // 457us (based on measurement) 
    slot_board_vars [SLOT_40ms_24GHZ].maxRxAckPrepare                =   10  ;  // 305us (based on measurement)
    slot_board_vars [SLOT_40ms_24GHZ].maxRxDataPrepare               =   10  ;  // 305us (based on measurement)
    slot_board_vars [SLOT_40ms_24GHZ].maxTxAckPrepare                =   15  ;  // 457us (based on measurement)
    slot_board_vars [SLOT_40ms_24GHZ].delayTx                        =   6   ;  // 183us (measured xxxus)
    slot_board_vars [SLOT_40ms_24GHZ].delayRx                        =   0  ;  // 0us (can not measure)

    //40ms slot for FSK
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].slotDuration              =   1310   ;  // ms  
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].maxTxDataPrepare          =   50   ;  // 1525us  (based on measurement)
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].maxRxAckPrepare           =   10   ;  // 305µs   (based on measurement)
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].maxRxDataPrepare          =   10   ;  // 305µs   (based on measurement)
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].maxTxAckPrepare           =   33   ;  // 1000µs  (based on measurement)
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].delayTx                   =   66   ;  // 2000µs  (based on measurement)
    slot_board_vars [SLOT_40ms_FSK_SUBGHZ].delayRx                   =   16   ;  // 488µs. This parameter is usually set to 0, however for Atmel on openmote-b, it takes at least 1ms for the transmission to occure because of spi delay (or other implementation specific overhead), so reciver is expected to wait a little more before turning on the radio. 
    
    //40ms slot for OFDM1 MCS0-3    
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].slotDuration      =  1310    ;  // ms  
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].maxTxDataPrepare  =  50    ;  // 1525us (based on measurement) 
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].maxRxAckPrepare   =  10    ;  // 305us (based on measurement) 
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].maxRxDataPrepare  =  10    ;  // 305us (based on measurement) 
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].maxTxAckPrepare   =  33    ;  // 1000us (based on measurement) 
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].delayTx           =  41    ;  // 1251us (based on measurement)  
    slot_board_vars [SLOT_40ms_OFDM1MCS0_3_SUBGHZ].delayRx           =  16    ;  // 488µs. This parameter is usually set to 0, however for Atmel on openmote-b, it takes at least 1ms for the transmission to occure because of spi delay (or other implementation specific overhead), so reciver is expected to wait a little more before turning on the radio. 
}

// To get the current slotDuration at any time (in tics)
// if you need the value in MS, divide by PORT_TICS_PER_MS (which varies by board and clock frequency and defined in board_info.h)
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


 // Resets the board

void board_reset(void) {
    SysCtrlReset();
}

//=========================== private =========================================

static void gpio_init(void) {
    // Configure all GPIO as input
    GPIOPinTypeGPIOInput(GPIO_A_BASE, 0xFF);
    GPIOPinTypeGPIOInput(GPIO_B_BASE, 0xFF);
    GPIOPinTypeGPIOInput(GPIO_C_BASE, 0xFF);
    GPIOPinTypeGPIOInput(GPIO_D_BASE, 0xFF);
}

static void clock_init(void) {
    // Disable global interrupts
    bool bIntDisabled = IntMasterDisable();

    // Configure the 32 kHz pins, PD6 and PD7, for crystal operation
    // By default they are configured as GPIOs
    GPIODirModeSet(GPIO_D_BASE, 0x40, GPIO_DIR_MODE_IN);
    GPIODirModeSet(GPIO_D_BASE, 0x80, GPIO_DIR_MODE_IN);
    IOCPadConfigSet(GPIO_D_BASE, 0x40, IOC_OVERRIDE_ANA);
    IOCPadConfigSet(GPIO_D_BASE, 0x80, IOC_OVERRIDE_ANA);

    // Set the real-time clock to use the 32 kHz external crystal
    // Set the system clock to use the external 32 MHz crystal
    // Set the system clock to 32 MHz
    SysCtrlClockSet(true, false, SYS_CTRL_SYSDIV_32MHZ);

    // Set the IO clock to operate at 32 MHz
    // This way peripherals can run while the system clock is gated
    SysCtrlIOClockSet(SYS_CTRL_SYSDIV_32MHZ);

    // Wait until the selected clock configuration is stable
    while (!((HWREG(SYS_CTRL_CLOCK_STA)) & (SYS_CTRL_CLOCK_STA_XOSC_STB)));

    // Define what peripherals run in each mode
    SysCtrlRunSetting();
    SysCtrlSleepSetting();
    SysCtrlDeepSleepSetting();
    SysCtrlWakeupSetting();

    // Re-enable interrupt if initially enabled
    if (!bIntDisabled) {
        IntMasterEnable();
    }
}


 // Configures the user button as input source

static void button_init(void) {
    // The button is an input GPIO on falling edge
    GPIOPinTypeGPIOInput(BSP_BUTTON_BASE, BSP_BUTTON_USER);
    GPIOIntTypeSet(BSP_BUTTON_BASE, BSP_BUTTON_USER, GPIO_FALLING_EDGE);

    // Enable wake-up capability
    GPIOIntWakeupEnable(GPIO_IWE_PORT_D);

    // Clear and enable the interrupt
    GPIOPinIntClear(BSP_BUTTON_BASE, BSP_BUTTON_USER);
    GPIOPinIntEnable(BSP_BUTTON_BASE, BSP_BUTTON_USER);
}

static void SysCtrlRunSetting(void) {
  // Disable General Purpose Timers 0, 1, 2, 3 when running
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_GPT3);

  // Disable SSI 0, 1 when running
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_SSI1);

  // Disable UART1 when running
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_UART1);

  // Disable I2C, AES and PKA when running
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralDisable(SYS_CTRL_PERIPH_AES);

  // Enable UART0 and RFC when running
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_RFC);
}

static void SysCtrlSleepSetting(void) {
  // Disable General Purpose Timers 0, 1, 2, 3 during sleep
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_GPT3);

  // Disable SSI 0, 1 during sleep
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_SSI1);

  // Disable UART 0, 1 during sleep
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_UART1);

  // Disable I2C, PKA, AES during sleep
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralSleepDisable(SYS_CTRL_PERIPH_AES);

  // Enable UART and RFC during sleep
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralSleepEnable(SYS_CTRL_PERIPH_RFC);
}

static void SysCtrlDeepSleepSetting(void) {
  // Disable General Purpose Timers 0, 1, 2, 3 during deep sleep
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT1);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT2);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_GPT3);

  // Disable SSI 0, 1 during deep sleep
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_SSI1);

  // Disable UART 0, 1 during deep sleep
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_UART0);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_UART1);

  // Disable I2C, PKA, AES during deep sleep
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_I2C);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_PKA);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_AES);
  SysCtrlPeripheralDeepSleepDisable(SYS_CTRL_PERIPH_RFC);
}

static void SysCtrlWakeupSetting(void) {
  // Allow the SMTimer to wake up the processor
  GPIOIntWakeupEnable(GPIO_IWE_SM_TIMER);
}

//=========================== interrupt handlers ==============================
