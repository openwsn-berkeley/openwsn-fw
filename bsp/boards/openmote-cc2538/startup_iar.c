//*****************************************************************************
//! @file       startup_iar.c
//! @brief      Startup code for CC2538 for use with IAR EWARM.
//!
//! Revised     $Date: 2013-04-29 14:48:18 +0200 (Mon, 29 Apr 2013) $
//! Revision    $Revision: 9929 $
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

#include <stdint.h>

#define FLASH_START_ADDR                0x00200000
#define BOOTLOADER_BACKDOOR_DISABLE     0xEFFFFFFF
#define SYS_CTRL_EMUOVR                 0x400D20B4
#define SYS_CTRL_I_MAP                  0x400D2098


//*****************************************************************************
//
// Check if compiler is IAR
//
//*****************************************************************************
#if !(defined(__IAR_SYSTEMS_ICC__))
#error "startup_iar.c: Unsupported compiler!"
#endif


//*****************************************************************************
//
// Macro for hardware access, both direct and via the bit-band region.
//
//*****************************************************************************
#ifndef HWREG
#define HWREG(x)                                                              \
        (*((volatile uint32_t *)(x)))
#endif


//*****************************************************************************
//
// IAR: Enable the IAR extensions for this source file.
//
//*****************************************************************************
#pragma language=extended


//*****************************************************************************
//
// IAR: The entry point for the application startup code.
//
//*****************************************************************************
extern void __iar_program_start(void);


//*****************************************************************************
//
// IAR: Get stack start (highest address) symbol from linker file.
//
//*****************************************************************************
extern const void* STACK_TOP;

// It is required to place something in the CSTACK segment to get the stack
// check feature in IAR to work as expected
__root static void* dummy_stack @ ".stack";

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
static void ResetISR(void);
static void NmiISR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);

// Handlers that can potentially be registered directly by application
extern void PendSVIntHandler(void);
extern void SysTickIntHandler(void);
extern void GPIOAIntHandler(void);
extern void GPIOBIntHandler(void);
extern void GPIOCIntHandler(void);
extern void GPIODIntHandler(void);
extern void UART0IntHandler(void);
extern void UART1IntHandler(void);
extern void SSI0IntHandler(void);
extern void SSI1IntHandler(void);
extern void I2CIntHandler(void);
extern void ADCIntHandler(void);
extern void WatchdogIntHandler(void);
extern void Timer0AIntHandler(void);
extern void Timer0BIntHandler(void);
extern void Timer1AIntHandler(void);
extern void Timer1BIntHandler(void);
extern void Timer2AIntHandler(void);
extern void Timer2BIntHandler(void);
extern void Timer3AIntHandler(void);
extern void Timer3BIntHandler(void);
extern void CompIntHandler(void);
extern void RFCoreTxIntHandler(void);
extern void RFCoreErrIntHandler(void);
extern void IcePickIntHandler(void);
extern void FlashIntHandler(void);
extern void AESIntHandler(void);
extern void PKAIntHandler(void);
extern void SleepModeIntHandler(void);
extern void MacTimerIntHandler(void);
extern void USBIntHandler(void);
extern void uDMAIntHandler(void);
extern void uDMAErrIntHandler(void);

// Default interrupt handlers, these can be overwritten if defined elsewhere
#pragma weak PendSVIntHandler = IntDefaultHandler
#pragma weak SysTickIntHandler = IntDefaultHandler
#pragma weak GPIOAIntHandler = IntDefaultHandler
#pragma weak GPIOBIntHandler = IntDefaultHandler
#pragma weak GPIOCIntHandler = IntDefaultHandler
#pragma weak GPIODIntHandler = IntDefaultHandler
#pragma weak UART0IntHandler = IntDefaultHandler
#pragma weak UART1IntHandler = IntDefaultHandler
#pragma weak SSI0IntHandler = IntDefaultHandler
#pragma weak SSI1IntHandler = IntDefaultHandler
#pragma weak I2CIntHandler = IntDefaultHandler
#pragma weak ADCIntHandler = IntDefaultHandler
#pragma weak WatchdogIntHandler = IntDefaultHandler
#pragma weak Timer0AIntHandler = IntDefaultHandler
#pragma weak Timer0BIntHandler = IntDefaultHandler
#pragma weak Timer1AIntHandler = IntDefaultHandler
#pragma weak Timer1BIntHandler = IntDefaultHandler
#pragma weak Timer2AIntHandler = IntDefaultHandler
#pragma weak Timer2BIntHandler = IntDefaultHandler
#pragma weak Timer3AIntHandler = IntDefaultHandler
#pragma weak Timer3BIntHandler = IntDefaultHandler
#pragma weak CompIntHandler = IntDefaultHandler
#pragma weak RFCoreTxIntHandler = IntDefaultHandler
#pragma weak RFCoreErrIntHandler = IntDefaultHandler
#pragma weak IcePickIntHandler = IntDefaultHandler
#pragma weak FlashIntHandler = IntDefaultHandler
#pragma weak AESIntHandler = IntDefaultHandler
#pragma weak PKAIntHandler = IntDefaultHandler
#pragma weak SleepModeIntHandler = IntDefaultHandler
#pragma weak MacTimerIntHandler = IntDefaultHandler
#pragma weak USBIntHandler = IntDefaultHandler
#pragma weak uDMAIntHandler = IntDefaultHandler
#pragma weak uDMAErrIntHandler = IntDefaultHandler


//*****************************************************************************
//
// Customer Configuration Area in Lock Page
// Holds Image Vector table address (bytes 2013 - 2015) and
// Image Valid bytes (bytes 2008 -2011)
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32BootldrCfg;
    uint32_t ui32ImageValid;
    uint32_t ui32ImageVectorAddr;
}
lockPageCCA_t;

__root const lockPageCCA_t __cca @ ".flashcca" =
{
  BOOTLOADER_BACKDOOR_DISABLE,  // Bootloader backdoor disabled
  0,                            // Image valid bytes
  FLASH_START_ADDR              // Vector table located at flash start address
};


//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x200000 (start of the flash).
//
//*****************************************************************************
__root void (* const __vector_table[])(void) @ ".intvec" =
{
    (void (*)(void))&STACK_TOP,             // 0 The initial stack pointer
    ResetISR,                               // 1 The reset handler
    NmiISR,                                 // The NMI handler
    FaultISR,                               // The hard fault handler
    IntDefaultHandler,                      // 4 The MPU fault handler
    IntDefaultHandler,                      // 5 The bus fault handler
    IntDefaultHandler,                      // 6 The usage fault handler
    0,                                      // 7 Reserved
    0,                                      // 8 Reserved
    0,                                      // 9 Reserved
    0,                                      // 10 Reserved
    IntDefaultHandler,                      // 11 SVCall handler
    IntDefaultHandler,                      // 12 Debug monitor handler
    0,                                      // 13 Reserved
    PendSVIntHandler,                       // 14 The PendSV handler
    SysTickIntHandler,                      // 15 The SysTick handler
    GPIOAIntHandler,                        // 16 GPIO Port A
    GPIOBIntHandler,                        // 17 GPIO Port B
    GPIOCIntHandler,                        // 18 GPIO Port C
    GPIODIntHandler,                        // 19 GPIO Port D
    0,                                      // 20 none
    UART0IntHandler,                        // 21 UART0 Rx and Tx
    UART1IntHandler,                        // 22 UART1 Rx and Tx
    SSI0IntHandler,                         // 23 SSI0 Rx and Tx
    I2CIntHandler,                          // 24 I2C Master and Slave
    0,                                      // 25 Reserved
    0,                                      // 26 Reserved
    0,                                      // 27 Reserved
    0,                                      // 28 Reserved
    0,                                      // 29 Reserved
    ADCIntHandler,                          // 30 ADC Sequence 0
    0,                                      // 31 Reserved
    0,                                      // 32 Reserved
    0,                                      // 33 Reserved
    WatchdogIntHandler,                     // 34 Watchdog timer, timer 0
    Timer0AIntHandler,                      // 35 Timer 0 subtimer A
    Timer0BIntHandler,                      // 36 Timer 0 subtimer B
    Timer1AIntHandler,                      // 37 Timer 1 subtimer A
    Timer1BIntHandler,                      // 38 Timer 1 subtimer B
    Timer2AIntHandler,                      // 39 Timer 2 subtimer A
    Timer2BIntHandler,                      // 40 Timer 2 subtimer B
    CompIntHandler,                         // 41 Analog Comparator 0
    RFCoreTxIntHandler,                     // 42 RFCore Rx/Tx
    RFCoreErrIntHandler,                    // 43 RFCore Error
    IcePickIntHandler,                      // 44 IcePick
    FlashIntHandler,                        // 45 FLASH Control
    AESIntHandler,                          // 46 AES
    PKAIntHandler,                          // 47 PKA
    SleepModeIntHandler,                    // 48 Sleep Timer
    MacTimerIntHandler,                     // 49 MacTimer
    SSI1IntHandler,                         // 50 SSI1 Rx and Tx
    Timer3AIntHandler,                      // 51 Timer 3 subtimer A
    Timer3BIntHandler,                      // 52 Timer 3 subtimer B
    0,                                      // 53 Reserved
    0,                                      // 54 Reserved
    0,                                      // 55 Reserved
    0,                                      // 56 Reserved
    0,                                      // 57 Reserved
    0,                                      // 58 Reserved
    0,                                      // 59 Reserved
    USBIntHandler,                          // 60 USB 2538
    0,                                      // 61 Reserved
    uDMAIntHandler,                         // 62 uDMA
    uDMAErrIntHandler,                      // 63 uDMA Error
#ifndef CC2538_USE_ALTERNATE_INTERRUPT_MAP
    0,                                      // 64 64-155 are not in use
    0,                                      // 65
    0,                                      // 66
    0,                                      // 67
    0,                                      // 68
    0,                                      // 69
    0,                                      // 70
    0,                                      // 71
    0,                                      // 72
    0,                                      // 73
    0,                                      // 74
    0,                                      // 75
    0,                                      // 76
    0,                                      // 77
    0,                                      // 78
    0,                                      // 79
    0,                                      // 80
    0,                                      // 81
    0,                                      // 82
    0,                                      // 83
    0,                                      // 84
    0,                                      // 85
    0,                                      // 86
    0,                                      // 87
    0,                                      // 88
    0,                                      // 89
    0,                                      // 90
    0,                                      // 91
    0,                                      // 92
    0,                                      // 93
    0,                                      // 94
    0,                                      // 95
    0,                                      // 96
    0,                                      // 97
    0,                                      // 98
    0,                                      // 99
    0,                                      // 100
    0,                                      // 101
    0,                                      // 102
    0,                                      // 103
    0,                                      // 104
    0,                                      // 105
    0,                                      // 106
    0,                                      // 107
    0,                                      // 108
    0,                                      // 109
    0,                                      // 110
    0,                                      // 111
    0,                                      // 112
    0,                                      // 113
    0,                                      // 114
    0,                                      // 115
    0,                                      // 116
    0,                                      // 117
    0,                                      // 118
    0,                                      // 119
    0,                                      // 120
    0,                                      // 121
    0,                                      // 122
    0,                                      // 123
    0,                                      // 124
    0,                                      // 125
    0,                                      // 126
    0,                                      // 127
    0,                                      // 128
    0,                                      // 129
    0,                                      // 130
    0,                                      // 131
    0,                                      // 132
    0,                                      // 133
    0,                                      // 134
    0,                                      // 135
    0,                                      // 136
    0,                                      // 137
    0,                                      // 138
    0,                                      // 139
    0,                                      // 140
    0,                                      // 141
    0,                                      // 142
    0,                                      // 143
    0,                                      // 144
    0,                                      // 145
    0,                                      // 146
    0,                                      // 147
    0,                                      // 148
    0,                                      // 149
    0,                                      // 150
    0,                                      // 151
    0,                                      // 152
    0,                                      // 153
    0,                                      // 154
    0,                                      // 155
    USBIntHandler,                          // 156 USB
    RFCoreTxIntHandler,                     // 157 RFCORE RX/TX
    RFCoreErrIntHandler,                    // 158 RFCORE Error
    AESIntHandler,                          // 159 AES
    PKAIntHandler,                          // 160 PKA
    SleepModeIntHandler,                    // 161 SMTimer
    MacTimerIntHandler,                     // 162 MACTimer
#endif
};

//*****************************************************************************
//
// This is the code that gets called when the processor is reset.
//
//*****************************************************************************
static void
ResetISR(void)
{
#ifdef DEBUG
    //
    // Workaround for System Reset debug issue
    //
    uint32_t ui32Timeout = 2000000;
    volatile uint32_t* pui32StopAtResetIsr = (uint32_t*)0x20003000;
    while((*pui32StopAtResetIsr == 0xA5F01248) && (ui32Timeout--));
#endif

#ifdef CC2538_USE_ALTERNATE_INTERRUPT_MAP
    //
    // Enable alternate interrupt mapping
    //
    HWREG(SYS_CTRL_I_MAP) |= 1;
#endif

    //
    // Jump to IAR initialization routine
    //
    __iar_program_start();
}


//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
static void
NmiISR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}


//*****************************************************************************
//
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
FaultISR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}


//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}
