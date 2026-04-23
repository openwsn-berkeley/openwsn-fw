/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2022 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File      : nrf52833_Vectors.s
Purpose   : Exception and interrupt vectors for nrf52833 devices.

Additional information:
  Preprocessor Definitions
    __NO_EXTERNAL_INTERRUPTS
      If defined,
        the vector table will contain only the internal exceptions
        and interrupts.
    __VECTORS_IN_RAM
      If defined,
        an area of RAM, large enough to store the vector table,
        will be reserved.

    __OPTIMIZATION_SMALL
      If defined,
        all weak definitions of interrupt handlers will share the
        same implementation.
      If not defined,
        all weak definitions of interrupt handlers will be defined
        with their own implementation.
*/
        .syntax unified

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/

//
// Directly place a vector (word) in the vector table
//
.macro VECTOR Name=
        .section .vectors, "ax"
        .code 16
        .word \Name
.endm

//
// Declare an exception handler with a weak definition
//
.macro EXC_HANDLER Name=
        //
        // Insert vector in vector table
        //
        .section .vectors, "ax"
        .word \Name
        //
        // Insert dummy handler in init section
        //
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b   // Endless loop
.endm

//
// Declare an interrupt handler with a weak definition
//
.macro ISR_HANDLER Name=
        //
        // Insert vector in vector table
        //
        .section .vectors, "ax"
        .word \Name
        //
        // Insert dummy handler in init section
        //
#if defined(__OPTIMIZATION_SMALL)
        .section .init, "ax"
        .weak \Name
        .thumb_set \Name,Dummy_Handler
#else
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b   // Endless loop
#endif
.endm

//
// Place a reserved vector in vector table
//
.macro ISR_RESERVED
        .section .vectors, "ax"
        .word 0
.endm

//
// Place a reserved vector in vector table
//
.macro ISR_RESERVED_DUMMY
        .section .vectors, "ax"
        .word Dummy_Handler
.endm

/*********************************************************************
*
*       Externals
*
**********************************************************************
*/
        .extern __stack_end__
        .extern Reset_Handler
        .extern HardFault_Handler

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*  Setup of the vector table and weak definition of interrupt handlers
*
*/
        .section .vectors, "ax"
        .code 16
        .balign 256
        .global _vectors
_vectors:
        //
        // Internal exceptions and interrupts
        //
        VECTOR __stack_end__
        VECTOR Reset_Handler
        EXC_HANDLER NMI_Handler
        VECTOR HardFault_Handler
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        EXC_HANDLER SVC_Handler
        ISR_RESERVED
        ISR_RESERVED
        EXC_HANDLER PendSV_Handler
        EXC_HANDLER SysTick_Handler
        //
        // External interrupts
        //
#ifndef __NO_EXTERNAL_INTERRUPTS
        ISR_HANDLER POWER_CLOCK_IRQHandler
        ISR_HANDLER RADIO_IRQHandler
        ISR_HANDLER UARTE0_UART0_IRQHandler
        ISR_HANDLER SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
        ISR_HANDLER SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
        ISR_HANDLER NFCT_IRQHandler
        ISR_HANDLER GPIOTE_IRQHandler
        ISR_HANDLER SAADC_IRQHandler
        ISR_HANDLER TIMER0_IRQHandler
        ISR_HANDLER TIMER1_IRQHandler
        ISR_HANDLER TIMER2_IRQHandler
        ISR_HANDLER RTC0_IRQHandler
        ISR_HANDLER TEMP_IRQHandler
        ISR_HANDLER RNG_IRQHandler
        ISR_HANDLER ECB_IRQHandler
        ISR_HANDLER CCM_AAR_IRQHandler
        ISR_HANDLER WDT_IRQHandler
        ISR_HANDLER RTC1_IRQHandler
        ISR_HANDLER QDEC_IRQHandler
        ISR_HANDLER COMP_LPCOMP_IRQHandler
        ISR_HANDLER SWI0_EGU0_IRQHandler
        ISR_HANDLER SWI1_EGU1_IRQHandler
        ISR_HANDLER SWI2_EGU2_IRQHandler
        ISR_HANDLER SWI3_EGU3_IRQHandler
        ISR_HANDLER SWI4_EGU4_IRQHandler
        ISR_HANDLER SWI5_EGU5_IRQHandler
        ISR_HANDLER TIMER3_IRQHandler
        ISR_HANDLER TIMER4_IRQHandler
        ISR_HANDLER PWM0_IRQHandler
        ISR_HANDLER PDM_IRQHandler
        ISR_RESERVED_DUMMY
        ISR_RESERVED_DUMMY
        ISR_HANDLER MWU_IRQHandler
        ISR_HANDLER PWM1_IRQHandler
        ISR_HANDLER PWM2_IRQHandler
        ISR_HANDLER SPIM2_SPIS2_SPI2_IRQHandler
        ISR_HANDLER RTC2_IRQHandler
        ISR_HANDLER I2S_IRQHandler
        ISR_HANDLER FPU_IRQHandler
        ISR_HANDLER USBD_IRQHandler
        ISR_HANDLER UARTE1_IRQHandler
        ISR_RESERVED_DUMMY
        ISR_RESERVED_DUMMY
        ISR_RESERVED_DUMMY
        ISR_RESERVED_DUMMY
        ISR_HANDLER PWM3_IRQHandler
        ISR_RESERVED_DUMMY
        ISR_HANDLER SPIM3_IRQHandler
#endif
        //
        .section .vectors, "ax"
_vectors_end:

#ifdef __VECTORS_IN_RAM
        //
        // Reserve space with the size of the vector table
        // in the designated RAM section.
        //
        .section .vectors_ram, "ax"
        .balign 256
        .global _vectors_ram

_vectors_ram:
        .space _vectors_end - _vectors, 0
#endif

/*********************************************************************
*
*  Dummy handler to be used for reserved interrupt vectors
*  and weak implementation of interrupts.
*
*/
        .section .init.Dummy_Handler, "ax"
        .thumb_func
        .weak Dummy_Handler
        .balign 2
Dummy_Handler:
        1: b 1b   // Endless loop


/*************************** End of file ****************************/
