//*****************************************************************************
//! @file       startup_gcc.c
//! @brief      Startup code for CC2538 for use with GCC.
//!
//! Revised     $Date: 2013-04-11 20:13:13 +0200 (Thu, 11 Apr 2013) $
//! Revision    $Revision: 9714 $
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

#include <headers/hw_nvic.h>

#include <stdint.h>

#define FLASH_START_ADDR                0x00200000
#define BOOTLOADER_BACKDOOR_ENABLE      0xF6FFFFFF // ENABLED: PORT A, PIN 6, LOW
#define BOOTLOADER_BACKDOOR_DISABLE     0xEFFFFFFF // DISABLED
#define SYS_CTRL_EMUOVR                 0x400D20B4
#define SYS_CTRL_I_MAP                  0x400D2098

//*****************************************************************************
//
// Macro for hardware access, both direct and via the bit-band region.
//
//*****************************************************************************
#ifndef HWREG
#define HWREG(x)                                                              \
        (*((volatile unsigned long *)(x)))
#endif

extern int main (void);

void ResetISR(void);
void NmiSR(void);
void FaultISR(void);
void IntDefaultHandler(void);

//*****************************************************************************
//
// Reserve space for the system stack.
//
//*****************************************************************************
static uint32_t pui32Stack[512];

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

__attribute__ ((section(".flashcca"), used))
const lockPageCCA_t __cca =
{
  BOOTLOADER_BACKDOOR_ENABLE,   // Bootloader backdoor enabled
  0,               				// Image valid bytes
  FLASH_START_ADDR 				// Vector table located at flash start address
};

__attribute__ ((section(".vectors"), used))
void (* const gVectors[])(void) =
{
   (void (*)(void))((uint32_t)pui32Stack + sizeof(pui32Stack)), // Stack pointer
   ResetISR,							   // Reset handler
   NmiSR,                                  // The NMI handler
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
   IntDefaultHandler,                      // 14 The PendSV handler
   IntDefaultHandler,                      // 15 The SysTick handler
   IntDefaultHandler,                      // 16 GPIO Port A
   IntDefaultHandler,                      // 17 GPIO Port B
   IntDefaultHandler,                      // 18 GPIO Port C
   IntDefaultHandler,                      // 19 GPIO Port D
   0,                                      // 20 none
   IntDefaultHandler,                      // 21 UART0 Rx and Tx
   IntDefaultHandler,                      // 22 UART1 Rx and Tx
   IntDefaultHandler,                      // 23 SSI0 Rx and Tx
   IntDefaultHandler,                      // 24 I2C Master and Slave
   0,                                      // 25 Reserved
   0,                                      // 26 Reserved
   0,                                      // 27 Reserved
   0,                                      // 28 Reserved
   0,                                      // 29 Reserved
   IntDefaultHandler,                      // 30 ADC Sequence 0
   0,                                      // 31 Reserved
   0,                                      // 32 Reserved
   0,                                      // 33 Reserved
   IntDefaultHandler,                      // 34 Watchdog timer, timer 0
   IntDefaultHandler,                      // 35 Timer 0 subtimer A
   IntDefaultHandler,                      // 36 Timer 0 subtimer B
   IntDefaultHandler,                      // 37 Timer 1 subtimer A
   IntDefaultHandler,                      // 38 Timer 1 subtimer B
   IntDefaultHandler,                      // 39 Timer 2 subtimer A
   IntDefaultHandler,                      // 40 Timer 2 subtimer B
   IntDefaultHandler,                      // 41 Analog Comparator 0
   IntDefaultHandler,                      // 42 RFCore Rx/Tx
   IntDefaultHandler,                      // 43 RFCore Error
   IntDefaultHandler,                      // 44 IcePick
   IntDefaultHandler,                      // 45 FLASH Control
   IntDefaultHandler,                      // 46 AES
   IntDefaultHandler,                      // 47 PKA
   IntDefaultHandler,                      // 48 Sleep Timer
   IntDefaultHandler,                      // 49 MacTimer
   IntDefaultHandler,                      // 50 SSI1 Rx and Tx
   IntDefaultHandler,                      // 51 Timer 3 subtimer A
   IntDefaultHandler,                      // 52 Timer 3 subtimer B
   0,                                      // 53 Reserved
   0,                                      // 54 Reserved
   0,                                      // 55 Reserved
   0,                                      // 56 Reserved
   0,                                      // 57 Reserved
   0,                                      // 58 Reserved
   0,                                      // 59 Reserved
   IntDefaultHandler,                      // 60 USB 2538
   0,                                      // 61 Reserved
   IntDefaultHandler,                      // 62 uDMA
   IntDefaultHandler,                      // 63 uDMA Error
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
   IntDefaultHandler,                      // 156 USB
   IntDefaultHandler,                      // 157 RFCORE RX/TX
   IntDefaultHandler,                      // 158 RFCORE Error
   IntDefaultHandler,                      // 159 AES
   IntDefaultHandler,                      // 160 PKA
   IntDefaultHandler,                      // 161 SMTimer
   IntDefaultHandler,                      // 162 MACTimer
#endif
};

//*****************************************************************************
//
// The following are constructs created by the linker, indicating where the
// the "data" and "bss" segments reside in memory.  The initializers for the
// for the "data" segment resides immediately following the "text" segment.
//
//*****************************************************************************
extern uint32_t _etext;
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _bss;
extern uint32_t _ebss;

//
// And here are the weak interrupt handlers.
//
void 
NmiSR (void) 
{ 
    ResetISR(); 
    while(1)
    {
    } 
}


void 
FaultISR (void) 
{ 
    while(1)
    {
    } 
}


void 
IntDefaultHandler (void) 
{ 
    while(1)
    {
    }
}

void 
ResetISR (void)
{
	uint32_t *pui32Src, *pui32Dest;

    //
	// Workaround for PM debug issue
    //
    HWREG(SYS_CTRL_EMUOVR) = 0xFF;


    /* Workaround for J-Link debug issue */
    HWREG(NVIC_VTABLE) = (uint32_t)gVectors;

    //
	// Copy the data segment initializers from flash to SRAM.
    //
	pui32Src = &_etext;
	for(pui32Dest = &_data; pui32Dest < &_edata; )
	{
		*pui32Dest++ = *pui32Src++;
	}

    //
	// Zero fill the bss segment.
    //
	__asm("    ldr     r0, =_bss\n"
		  "    ldr     r1, =_ebss\n"
		  "    mov     r2, #0\n"
		  "    .thumb_func\n"
		  "zero_loop:\n"
		  "        cmp     r0, r1\n"
		  "        it      lt\n"
		  "        strlt   r2, [r0], #4\n"
		  "        blt     zero_loop");

#ifdef CC2538_USE_ALTERNATE_INTERRUPT_MAP
    //
    // Enable alternate interrupt mapping
    //
    HWREG(SYS_CTRL_I_MAP) |= 1;
#endif
   //
   // Call the application's entry point.
   //
   main();

   //
   // End here if return from main()
   //
   while(1)
   {
   }
}

