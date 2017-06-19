//*****************************************************************************
//! @file       bsp.c
//! @brief      Board support package for MSP430F5438a on SmartRF06EB.
//!
//! Revised     $Date: 2013-09-18 13:18:48 +0200 (on, 18 sep 2013) $
//! Revision    $Revision: 10587 $
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/


/**************************************************************************//**
* @addtogroup bsp_api
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "bsp.h"


/******************************************************************************
* DEFINES
*/
#define DCO_MULT_1MHZ           30
#define DCO_MULT_4MHZ           122
#define DCO_MULT_8MHZ           244
#define DCO_MULT_12MHZ          366
#define DCO_MULT_16MHZ          488
#define DCO_MULT_20MHZ          610
#define DCO_MULT_25MHZ          763

#define DCORSEL_1MHZ            DCORSEL_2
#define DCORSEL_4MHZ            DCORSEL_4
#define DCORSEL_8MHZ            DCORSEL_4
#define DCORSEL_12MHZ           DCORSEL_5
#define DCORSEL_16MHZ           DCORSEL_5
#define DCORSEL_20MHZ           DCORSEL_6
#define DCORSEL_25MHZ           DCORSEL_7

#define VCORE_1MHZ              PMMCOREV_0
#define VCORE_4MHZ              PMMCOREV_0
#define VCORE_8MHZ              PMMCOREV_0
#define VCORE_12MHZ             PMMCOREV_1
#define VCORE_16MHZ             PMMCOREV_1
#define VCORE_20MHZ             PMMCOREV_2
#define VCORE_25MHZ             PMMCOREV_3

#define VCORE_1_35V             PMMCOREV_0
#define VCORE_1_55V             PMMCOREV_1
#define VCORE_1_75V             PMMCOREV_2
#define VCORE_1_85V             PMMCOREV_3

// Register defines
#define IO_SPI0_BUS_DIR         P9DIR
#define IO_SPI0_BUS_SEL         P9SEL
#define IO_SPI0_BUS_OUT         P9OUT
#define IO_SPI0_BUS_REN         P9REN

#define IO_SPI1_BUS_DIR         IO_SPI0_BUS_DIR
#define IO_SPI1_BUS_SEL         IO_SPI0_BUS_SEL
#define IO_SPI1_BUS_OUT         IO_SPI0_BUS_OUT
#define IO_SPI1_BUS_REN         IO_SPI0_BUS_REN


/******************************************************************************
* FUNCTION PROTOTYPES
*/
//static void bspMcuStartXT1(void);
static void bspMcuSetVCoreUp(uint8_t ui8Level);
static void bspMcuSetVCoreDown(uint8_t ui8Level);
static void bspMcuGetSystemClockSettings(uint32_t ui32SystemClockSpeed,
                                         uint8_t *pui8SetDcoRange,
                                         uint8_t *pui8SetVCore,
                                         uint32_t *pui32SetMultiplier);
static void bspMcuSetVCore(uint8_t ui8Level);


/******************************************************************************
* LOCAL VARIABLES
*/
static uint32_t ui32BspMclkSpeed;
static uint32_t ui32IoSpiClkSpeed[2];


/******************************************************************************
* FUNCTIONS
*/

/**************************************************************************//**
* @brief    Function initializes the MSP430F5438a clocks and I/O for use on
*           SmartRF06EB.
*
*           The function assumes an external crystal oscillator to be available
*           to the MSP430F5438a. The MSP430F5438a main system clock (MCLK) and
*           Sub Main System Clock (SMCLK) are set to the frequency given by
*           input argument \e ui32SysClockSpeed. ACLK is set to 32768 Hz.
*
* @param    ui32SysClockSpeed   is the system clock speed in MHz; it must be
*                               one of the following:
*           \li \b BSP_SYS_CLK_1MHZ
*           \li \b BSP_SYS_CLK_4MHZ
*           \li \b BSP_SYS_CLK_8MHZ
*           \li \b BSP_SYS_CLK_12MHZ
*           \li \b BSP_SYS_CLK_16MHZ
*           \li \b BSP_SYS_CLK_20MHZ
*           \li \b BSP_SYS_CLK_25MHZ
*
* @return   None
******************************************************************************/
void
bspInit(uint32_t ui32SysClockSpeed)
{
    uint16_t ui16IntState;

    //
    // Stop watchdog timer (prevent timeout reset)
    //
    WDTCTL = WDTPW + WDTHOLD;

    //
    // Disable global interrupts
    //
    ui16IntState = __get_interrupt_state();
    __disable_interrupt();

    //
    //  Set capacitor values for XT1, 32768 Hz */
    //
    bspMcuStartXT1();

    bspSysClockSpeedSet(ui32SysClockSpeed);

    //
    // Initialize LEDs as off (pins as GPIO output high)
    //
    P4SEL &= ~BSP_LED_ALL;
    P4OUT |=  BSP_LED_ALL;
    P4DIR |=  BSP_LED_ALL;

    //
    // LCD CS as GPIO output high
    //
    P9SEL &= ~BIT6;
    P9OUT |=  BIT6;
    P9DIR |=  BIT6;

    //
    // Accelerometer + SPI flash CS as GPIO output high
    //
    P8SEL &= ~(BSP_FLASH_CS_N | BIT7);
    P8OUT |= (BSP_FLASH_CS_N | BIT7);
    P8DIR |= (BSP_FLASH_CS_N | BIT7);

    //
    // RF SPI0 CS as GPIO output high
    //
    P3SEL &= ~BIT0;
    P3OUT |=  BIT0;
    P3DIR |=  BIT0;

    //
    // RF SPI1 CS as GPIO output high
    //
    P4SEL &= ~BIT1;
    P4OUT |=  BIT1;
    P4DIR |=  BIT1;

    //
    // Hold SPI flash powered up, but in reset (GPIO output high, full drive
    // strength).
    //
    P7SEL &= ~(BIT2 | BIT6);
    P7DS  |=   BIT6;
    P7OUT |=   BIT6;
    P7OUT &=  ~BIT2;
    P7DIR |= (BIT2 | BIT6);

    //
    // Accelerometer + Light sensor PWR as GPIO input pulldown
    //
    P6SEL &= ~(BIT0 | BSP_ALS_PWR);
    P6OUT &= ~(BIT0 | BSP_ALS_PWR);
    P6REN |= (BIT0 | BSP_ALS_PWR);
    P6DIR &= ~(BIT0 | BSP_ALS_PWR);

    //
    // USB UART as GPIO input pullup
    //
    P5SEL &= ~(BIT6 | BIT7);
    P5OUT |= (BIT6 | BIT7);
    P5REN |= (BIT6 | BIT7);
    P5DIR &= ~(BIT6 | BIT7);

    //
    // Return to previous interrupt state
    //
    __set_interrupt_state(ui16IntState);
}


/**************************************************************************//**
* @brief    Function returns the system clock speed (in MHz) set by bspInit().
*
* @return   Returns the system clock speed (in MHz) set by bspInit().
******************************************************************************/
uint32_t
bspSysClockSpeedGet(void)
{
    return (ui32BspMclkSpeed);
}


/**************************************************************************//**
* @brief    Function initializes the SPI interface specified by \e ui8Spi to
*           the baud rate specified by \e ui32ClockSpeed.
*
*           The actual SPI clock speed may differ from \e ui32ClockSpeed if
*           (SMCLK divided by \e ui32ClockSpeed is not an integer. In this case
*           the SPI clock speed will be the nearest frequency less than
*           \e ui32ClockSpeed.
*
*           If \e ui8Spi equals \b BSP_IO_SPI0, the SPI clock speed will be
*           limited to the smallest of 20 MHz and the SMCLK speed set by
*           bspInit().
*
*           If \e ui8Spi equals \b BSP_IO_SPI1, the SPI clock speed will be
*           limited to 400 kHz.
*
* @param    ui8Spi          is the interface to initialize; it must be one
*                           of the following:
*                           \li \b BSP_IO_SPI0 (LCD and SPI FLASH)
*                           \li \b BSP_IO_SPI1 (Accelerometer)
* @param    ui8ClockSpeed   is the SPI baudrate in hertz. The actual SPI clock
*                           speed configured may be less than the specified.
*
* @return   Returns the actual clock speed set in Hz.
******************************************************************************/
uint32_t
bspIoSpiInit(uint8_t ui8Spi, uint32_t ui32ClockSpeed)
{
    uint16_t ui16Div;
    uint32_t ui32SysClk = bspSysClockSpeedGet();

    //
    // Check arguments
    //
    if(ui32ClockSpeed > ui32SysClk)
    {
        ui32ClockSpeed = ui32SysClk;
    }
    if((ui8Spi == BSP_IO_SPI0) && (ui32ClockSpeed > 20000000))
    {
        //
        // Clock speed too high. Max 20 MHz for signal integrity.
        //
        ui32ClockSpeed = 20000000;
    }
    else if((ui8Spi == BSP_IO_SPI1) && (ui32ClockSpeed > 400000))
    {
        //
        // Clock speed too high. Accelerometer supports max 400 kHz.
        //
        ui32ClockSpeed = 400000;
    }
    else if((ui8Spi != BSP_IO_SPI0) && (ui8Spi != BSP_IO_SPI1))
    {
        //
        // Invalid input argument. No clock set.
        //
        return (0);
    }

    //
    // Calculate divider and store actual clock speed
    //
    ui16Div = ui32SysClk / ui32ClockSpeed;
    if(ui32SysClk % ui32ClockSpeed)
    {
        //
        // Choose the closest, low-side rate
        //
        ui16Div++;
    }
    ui32IoSpiClkSpeed[(ui8Spi >> 1)] = ui32SysClk / (uint32_t)ui16Div;

    if(ui8Spi == BSP_IO_SPI0)
    {
        //
        // Configure USCI B2 for SPI master. Disabling SPI module.
        // Configuration: Synchronous mode, 3-pin PSI, master, 8-bit, MSB
        // first, clock pol=1 (inactive high), clock pha=0 (sampling on second
        // edge), clock source = SMCLK.
        //
        UCB2CTL1 |= UCSWRST;
        UCB2CTL0  = UCSYNC | UCMST | UCCKPL | UCMSB;
        UCB2CTL1 |= UCSSEL1 | UCSSEL0;

        //
        // Apply divider (low nibble, high nibble)
        //
        UCB2BR0  = (ui16Div & 0xFF);
        UCB2BR1  = ((ui16Div >> 8) & 0xFF);

        //
        // Configure ports and pins
        //
        P9SEL |= (BSP_IO_SPI0_MOSI | BSP_IO_SPI0_MISO | BSP_IO_SPI0_SCLK);

        //
        // Enable SPI interface
        //
        UCB2CTL1 &= ~UCSWRST;
    }

    if(ui8Spi == BSP_IO_SPI1)
    {
        //
        // Configure USCI A2 for SPI master. Disabling SPI module.
        // Configuration: Synchronous mode, 3-pin PSI, master, 8-bit, MSB
        // first, clock pol=1 (inactive high), clock pha=0 (sampling on second
        // edge), clock source = SMCLK.
        //
        UCA2CTL1 |= UCSWRST;
        UCA2CTL0 = UCSYNC | UCMST | UCCKPL | UCMSB;
        UCA2CTL1 |= UCSSEL1 | UCSSEL0;

        //
        // Apply divider (low nibble, high nibble)
        //
        UCA2BR0 = (ui16Div & 0xFF);
        UCA2BR1 = (ui16Div >> 8) & 0xFF;

        //
        // Configure ports and pins
        //
        P9SEL |= (BSP_IO_SPI1_MOSI | BSP_IO_SPI1_MISO | BSP_IO_SPI1_SCLK);

        //
        // Enable SPI interface
        //
        UCA2CTL1 &= ~UCSWRST;
    }

    //
    // Return actual SPI clock speed
    //
    return (ui32IoSpiClkSpeed[(ui8Spi >> 1)]);
}


/**************************************************************************//**
* @brief    Returns the baudrate configured by bspSpiInit() for the SPI
*           interface specified by \e ui8Spi.
*
* @param    ui8Spi      The interface. May be one of the following:
*                       \li \b BSP_IO_SPI0 (LCD and SPI FLASH)
*                       \li \b BSP_IO_SPI1 (Accelerometer)
*
* @return   Returns the configured baudrate.
******************************************************************************/
uint32_t
bspIoSpiClockSpeedGet(uint8_t ui8Spi)
{
    return (ui32IoSpiClkSpeed[(ui8Spi >> 1)]);
}


/**************************************************************************//**
* @brief    Function un-initializes the SPI interface specified \e ui8Spi.
*
* @param    ui8Spi      The interface to uninitialize. May be one
*                       of the following:
*                       \li \b BSP_IO_SPI0 (LCD and SPI FLASH)
*                       \li \b BSP_IO_SPI1 (Accelerometer)
*
* @return   None
******************************************************************************/
void
bspIoSpiUninit(uint8_t ui8Spi)
{
    switch(ui8Spi)
    {
    case BSP_IO_SPI0:
        //
        // Reset SPI module and set MISO, MOSI and SCLK as GPIO input tristate.
        //
        UCB2CTL1 |= UCSWRST;
        IO_SPI0_BUS_SEL &= ~(BSP_IO_SPI0_MOSI | BSP_IO_SPI0_MISO |            \
                             BSP_IO_SPI0_SCLK);
        IO_SPI0_BUS_DIR &= ~(BSP_IO_SPI0_MOSI | BSP_IO_SPI0_MISO |            \
                             BSP_IO_SPI0_SCLK);
        IO_SPI0_BUS_REN &= ~(BSP_IO_SPI0_MOSI | BSP_IO_SPI0_MISO |            \
                             BSP_IO_SPI0_SCLK);
        break;
    case BSP_IO_SPI1:
        //
        // Reset SPI module and set MISO, MOSI and SCLK as GPIO input tristate.
        //
        UCA2CTL1 |= UCSWRST;
        IO_SPI1_BUS_SEL &= ~(BSP_IO_SPI1_MOSI | BSP_IO_SPI1_MISO |            \
                             BSP_IO_SPI1_SCLK);
        IO_SPI1_BUS_DIR &= ~(BSP_IO_SPI1_MOSI | BSP_IO_SPI1_MISO |            \
                             BSP_IO_SPI1_SCLK);
        IO_SPI1_BUS_REN &= ~(BSP_IO_SPI1_MOSI | BSP_IO_SPI1_MISO |            \
                             BSP_IO_SPI1_SCLK);
    default:
        break;
    }

    //
    // Clear SPI speed variable
    //
    ui32IoSpiClkSpeed[(ui8Spi >> 1)] = 0;
}


/**************************************************************************//**
* @brief    This is an assert function. It runs an infinite loop that blinks
*           all LEDs quickly. The function assumes LEDs to be initialized by,
*           for example,  bspInit().
*
* @return   None
******************************************************************************/
void
bspAssert(void)
{
    //
    // Set all LEDs to the same state before the infinite loop
    //
    P4OUT |= BSP_LED_ALL;
    while(1)
    {
        //
        // Toggle LEDs
        //
        P4OUT ^= BSP_LED_ALL;

        //
        // Simple wait
        //
        __delay_cycles(900000);
    }
}


/******************************************************************************
* LOCAL FUNCTIONS
*/
/*****************************************************************************
* @brief    This function runs the initialization routine for XT1. It sets the
*           necessary internal capacitor values, and loops until all ocillator
*           fault flags remain cleared. XT1 is in Low Power Mode.
*
* @return      none
******************************************************************************/
void
bspMcuStartXT1(void)
{
    //
    // Set up XT1 Pins to analog function, and to lowest drive
    //
    P7SEL |= 0x03;

    //
    // Set internal cap values
    //
    UCSCTL6 |= XCAP_3 ;

    //
    // Check OFIFG fault flag
    //
    while(SFRIFG1 & OFIFG)
    {
        //
        // Check OFIFG fault flag
        //
        while(SFRIFG1 & OFIFG)
        {
            //
            // Clear OSC fault flags and OFIFG fault flag
            //
            UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT1HFOFFG + XT2OFFG);
            SFRIFG1 &= ~OFIFG;
        }

        //
        // Reduce the drive strength
        //
        UCSCTL6 &= ~(XT1DRIVE1_L + XT1DRIVE0);
    }
}


/**************************************************************************//**
* @brief    This function sets the MCLK frequency. It also selects XT1 as ACLK
*           source with no divison in Low Power mode.
*
* @param    ui32SystemClockSpeed    is the intended frequency of operation.
*
* @return   None
******************************************************************************/
void
bspSysClockSpeedSet(uint32_t ui32SystemClockSpeed)
{
    uint8_t ui8SetDcoRange, ui8SetVCore;
    uint32_t ui32SetMultiplier;
    
    //
    // Set clocks (doing sanity check)
    // MCLK     = ui32SysClockSpeed;
    // SMCLK    = ui32SysClockSpeed;
    // ACLK     = 32 768 Hz
    //
    if((ui32SystemClockSpeed > 25000000UL) || (ui32SystemClockSpeed < 1000000UL))
    {
        bspAssert();
    }
    
    ui32BspMclkSpeed = ui32SystemClockSpeed;
    
    //
    // Get DCO, VCore and multiplier settings for the given clock speed
    //
    bspMcuGetSystemClockSettings(ui32SystemClockSpeed, &ui8SetDcoRange,
                                 &ui8SetVCore, &ui32SetMultiplier);

    //
    // Set VCore setting
    //
    bspMcuSetVCore(ui8SetVCore);

    //
    // Disable FLL control loop, set lowest possible DCOx, MODx and select
    // a suitable range
    //
    __bis_SR_register(SCG0);
    UCSCTL0 = 0x00;
    UCSCTL1 = ui8SetDcoRange;

    //
    // Set DCO multiplier and reenable the FLL control loop
    //
    UCSCTL2 = ui32SetMultiplier + FLLD_1;
    UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV  |  SELM__DCOCLKDIV ;
    __bic_SR_register(SCG0);

    //
    // Loop until osciallator fault falgs (XT1, XT2 & DCO fault flags)
    // are cleared
    //
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);

        //
        // Clear XT2, XT1, DCO fault flags
        //
        SFRIFG1 &= ~OFIFG;
    }
    while(SFRIFG1 & OFIFG);

    //
    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_FLL_reference. See UCS chapter in 5xx UG
    // for optimization.
    // 32 x 32 x / f_FLL_reference (32,768 Hz) = .03125 = t_DCO_settle
    // t_DCO_settle / (1 / 25 MHz) = 781250 = counts_DCO_settle
    //
    __delay_cycles(781250);
}


/**************************************************************************//**
* @brief    Increments the VCore setting.
*
* @param    ui8Level        is the target VCore setting.
*
* @return   None
******************************************************************************/
static void
bspMcuSetVCoreUp(uint8_t ui8Level)
{
    //
    // Open PMM module registers for write access
    //
    PMMCTL0_H = 0xA5;

    //
    // Set SVS/M high side to new ui8Level
    //
    SVSMHCTL = (SVSMHCTL & ~(SVSHRVL0 * 3 + SVSMHRRL0)) |
               (SVSHE + SVSHRVL0 * ui8Level + SVMHE + SVSMHRRL0 * ui8Level);

    //
    // Set SVM new Level
    //
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * ui8Level;

    //
    // Set SVS/M low side to new level
    //
    SVSMLCTL = (SVSMLCTL & ~(SVSMLRRL_3)) | (SVMLE + SVSMLRRL0 * ui8Level);

    //
    // Wait until SVM is settled
    //
    while((PMMIFG & SVSMLDLYIFG) == 0)
    {
    }

    //
    // Set VCore to 'level' and clear flags
    //
    PMMCTL0_L = PMMCOREV0 * ui8Level;
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);

    if((PMMIFG & SVMLIFG))
    {
        //
        // Wait until level is reached
        //
        while((PMMIFG & SVMLVLRIFG) == 0);
    }

    //
    // Set SVS/M Low side to new level
    //
    SVSMLCTL = (SVSMLCTL & ~(SVSLRVL0 * 3 + SVSMLRRL_3)) |                      \
               (SVSLE + SVSLRVL0 * ui8Level + SVMLE + SVSMLRRL0 * ui8Level);

    //
    // Lock PMM module registers from write access
    //
    PMMCTL0_H = 0x00;
}

/**************************************************************************//**
* @brief    This function decrements the VCore setting.
*
* @param    ui8Level    is the target VCore setting.
*
* @return   None
******************************************************************************/
static void
bspMcuSetVCoreDown(uint8_t ui8Level)
{
    //
    // Open PMM module registers for write access
    //
    PMMCTL0_H = 0xA5;

    //
    // Set SVS/M low side to new level
    //
    SVSMLCTL = (SVSMLCTL & ~(SVSLRVL0 * 3 + SVSMLRRL_3)) |                      \
               (SVSLRVL0 * ui8Level + SVMLE + SVSMLRRL0 * ui8Level);

    //
    // Wait until SVM is settled
    //
    while((PMMIFG & SVSMLDLYIFG) == 0)
    {
    }

    //
    // Set VCore to new level
    //
    PMMCTL0_L = (ui8Level * PMMCOREV0);


    //
    // Lock PMM module registers for write access
    //
    PMMCTL0_H = 0x00;
}

/*****************************************************************************
* @brief    Get function for the DCORSEL, VCORE, and DCO multiplier
*           settings that map to a given clock speed.
*
* @param    ui32SystemClockSpeed    is the target DCO frequency; can be one of
*           the following values:
*           \li \b BSP_SYS_CLK_1MHZ
*           \li \b BSP_SYS_CLK_4MHZ
*           \li \b BSP_SYS_CLK_8MHZ
*           \li \b BSP_SYS_CLK_12MHZ
*           \li \b BSP_SYS_CLK_16MHZ
*           \li \b BSP_SYS_CLK_20MHZ
*           \li \b BSP_SYS_CLK_25MHZ
* @param    pui8SetDcoRange         is a pointer to the DCO range select bits.
* @param    pui8SetVCore            is a pointer to the VCore level bits.
* @param    pui32SetMultiplier      is a pointer to the DCO multiplier bits.
*
* @return      None
******************************************************************************/
static void
bspMcuGetSystemClockSettings(uint32_t ui32SystemClockSpeed,
                             uint8_t *pui8SetDcoRange, uint8_t *pui8SetVCore,
                             uint32_t  *pui32SetMultiplier)
{
    switch(ui32SystemClockSpeed)
    {
    case BSP_SYS_CLK_1MHZ:
        *pui8SetDcoRange = DCORSEL_1MHZ;
        *pui8SetVCore = VCORE_1MHZ;
        *pui32SetMultiplier = DCO_MULT_1MHZ;
        break;
    case BSP_SYS_CLK_4MHZ:
        *pui8SetDcoRange = DCORSEL_4MHZ;
        *pui8SetVCore = VCORE_4MHZ;
        *pui32SetMultiplier = DCO_MULT_4MHZ;
        break;
    case BSP_SYS_CLK_8MHZ:
        *pui8SetDcoRange = DCORSEL_8MHZ;
        *pui8SetVCore = VCORE_8MHZ;
        *pui32SetMultiplier = DCO_MULT_8MHZ;
        break;
    case BSP_SYS_CLK_12MHZ:
        *pui8SetDcoRange = DCORSEL_12MHZ;
        *pui8SetVCore = VCORE_12MHZ;
        *pui32SetMultiplier = DCO_MULT_12MHZ;
        break;
    case BSP_SYS_CLK_16MHZ:
        *pui8SetDcoRange = DCORSEL_16MHZ;
        *pui8SetVCore = VCORE_16MHZ;
        *pui32SetMultiplier = DCO_MULT_16MHZ;
        break;
    case BSP_SYS_CLK_20MHZ:
        *pui8SetDcoRange = DCORSEL_20MHZ;
        *pui8SetVCore = VCORE_20MHZ;
        *pui32SetMultiplier = DCO_MULT_20MHZ;
        break;
    case BSP_SYS_CLK_25MHZ:
        *pui8SetDcoRange = DCORSEL_25MHZ;
        *pui8SetVCore = VCORE_25MHZ;
        *pui32SetMultiplier = DCO_MULT_25MHZ;
        break;
    }
}


/*****************************************************************************
* @brief    This function sets the PMM core voltage (PMMCOREV) setting.
*
* @param    ui8Level        is the target VCore setting.
*
* @return   None
******************************************************************************/
static void
bspMcuSetVCore(uint8_t ui8Level)
{
    uint8_t ui8ActLevel;
    do
    {
        ui8ActLevel = PMMCTL0_L & PMMCOREV_3;
        if(ui8ActLevel < ui8Level)
        {
            //
            // Set Vcore (step by step)
            //
            bspMcuSetVCoreUp(++ui8ActLevel);
        }
        if(ui8ActLevel > ui8Level)
        {
            //
            // Set VCore step by step
            //
            bspMcuSetVCoreDown(--ui8ActLevel);
        }
    }
    while(ui8ActLevel != ui8Level);
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
