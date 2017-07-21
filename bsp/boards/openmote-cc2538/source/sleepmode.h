/******************************************************************************
*  Filename:       sleepmode.h
*  Revised:        $Date: 2013-01-21 15:25:21 +0100 (Mon, 21 Jan 2013) $
*  Revision:       $Revision: 9178 $
*
*  Description:    Prototypes for the Sleep Mode Timer API.
*
*  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __SLEEPMODE_H__
#define __SLEEPMODE_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <headers/hw_types.h>

//*****************************************************************************
//
// The following are values that can be passed to the SleepModeCaptureConfig
// function.
//
//*****************************************************************************
#define SLEEPMODE_PORT_A    0x00000000  // Capture Port A 
#define SLEEPMODE_PORT_B    0x00000008  // Capture Port B
#define SLEEPMODE_PORT_C    0x00000010  // Capture Port C
#define SLEEPMODE_PORT_D    0x00000018  // Capture Port D
#define SLEEPMODE_PORT_USB  0x00000020  // Capture Port USB

#define SLEEPMODE_PIN_0     0x00000000  // Capture Pin 0
#define SLEEPMODE_PIN_1     0x00000001  // Capture Pin 1 (not valid with USB)
#define SLEEPMODE_PIN_2     0x00000002  // Capture Pin 2 (not valid with USB)
#define SLEEPMODE_PIN_3     0x00000003  // Capture Pin 3 (not valid with USB)
#define SLEEPMODE_PIN_4     0x00000004  // Capture Pin 4 (not valid with USB)
#define SLEEPMODE_PIN_5     0x00000005  // Capture Pin 5 (not valid with USB)
#define SLEEPMODE_PIN_6     0x00000006  // Capture Pin 6 (not valid with USB)
#define SLEEPMODE_PIN_7     0x00000007  // Capture Pin 7 (not valid with USB)

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

extern void SleepModeIntRegister(void (*pfnHandler)(void));
extern void SleepModeIntUnregister(void);
extern uint32_t SleepModeTimerCountGet(void);
extern void SleepModeTimerCompareSet(uint32_t ui32Compare);
extern void SleepModeCaptureConfig(uint32_t ui32Port, uint32_t ui32Pin);
extern void SleepModeCaptureNew(void);
extern uint32_t SleepModeCaptureGet(void);
extern bool SleepModeCaptureIsValid(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __SLEEPMODE_H__
