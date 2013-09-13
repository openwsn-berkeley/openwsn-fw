/******************************************************************************
*  Filename:       gptimer.h
*  Revised:        $Date: 2013-02-19 10:35:37 +0100 (Tue, 19 Feb 2013) $
*  Revision:       $Revision: 9322 $
*
*  Description:    Prototypes for the general purpose timer module
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

#ifndef __GPTIMER_H__
#define __GPTIMER_H__

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

#include "hw_types.h"
  
//*****************************************************************************
//
// Values that can be passed to TimerConfigure as the ui32Config parameter.
//
//*****************************************************************************
#define GPTIMER_CFG_ONE_SHOT       0x00000021  // Full-width one-shot timer
#define GPTIMER_CFG_ONE_SHOT_UP    0x00000031  // Full-width one-shot up-count
                                               // timer
#define GPTIMER_CFG_PERIODIC       0x00000022  // Full-width periodic timer
#define GPTIMER_CFG_PERIODIC_UP    0x00000032  // Full-width periodic up-count
                                               // timer
#define GPTIMER_CFG_SPLIT_PAIR     0x04000000  // Two half-width timers
#define GPTIMER_CFG_A_ONE_SHOT     0x00000021  // Timer A one-shot timer
#define GPTIMER_CFG_A_ONE_SHOT_UP  0x00000031  // Timer A one-shot up-count timer
#define GPTIMER_CFG_A_PERIODIC     0x00000022  // Timer A periodic timer
#define GPTIMER_CFG_A_PERIODIC_UP  0x00000032  // Timer A periodic up-count timer
#define GPTIMER_CFG_A_CAP_COUNT    0x00000003  // Timer A event counter
#define GPTIMER_CFG_A_CAP_COUNT_UP 0x00000013  // Timer A event up-counter
#define GPTIMER_CFG_A_CAP_TIME     0x00000007  // Timer A event timer
#define GPTIMER_CFG_A_CAP_TIME_UP  0x00000017  // Timer A event up-count timer
#define GPTIMER_CFG_A_PWM          0x0000000A  // Timer A PWM output
#define GPTIMER_CFG_B_ONE_SHOT     0x00002100  // Timer B one-shot timer
#define GPTIMER_CFG_B_ONE_SHOT_UP  0x00003100  // Timer B one-shot up-count timer
#define GPTIMER_CFG_B_PERIODIC     0x00002200  // Timer B periodic timer
#define GPTIMER_CFG_B_PERIODIC_UP  0x00003200  // Timer B periodic up-count timer
#define GPTIMER_CFG_B_CAP_COUNT    0x00000300  // Timer B event counter
#define GPTIMER_CFG_B_CAP_COUNT_UP 0x00001300  // Timer B event up-counter
#define GPTIMER_CFG_B_CAP_TIME     0x00000700  // Timer B event timer
#define GPTIMER_CFG_B_CAP_TIME_UP  0x00001700  // Timer B event up-count timer
#define GPTIMER_CFG_B_PWM          0x00000A00  // Timer B PWM output

//*****************************************************************************
//
// Values that can be passed to TimerIntEnable, TimerIntDisable, and
// TimerIntClear as the ui32IntFlags parameter, and returned from TimerIntStatus.
//
//*****************************************************************************
#define GPTIMER_TIMB_MATCH        0x00000800  // TimerB match interrupt
#define GPTIMER_CAPB_EVENT        0x00000400  // CaptureB event interrupt
#define GPTIMER_CAPB_MATCH        0x00000200  // CaptureB match interrupt
#define GPTIMER_TIMB_TIMEOUT      0x00000100  // TimerB time out interrupt
#define GPTIMER_TIMA_MATCH        0x00000010  // TimerA match interrupt
#define GPTIMER_RTC_MATCH         0x00000008  // RTC interrupt mask
#define GPTIMER_CAPA_EVENT        0x00000004  // CaptureA event interrupt
#define GPTIMER_CAPA_MATCH        0x00000002  // CaptureA match interrupt
#define GPTIMER_TIMA_TIMEOUT      0x00000001  // TimerA time out interrupt

//*****************************************************************************
//
// Values that can be passed to TimerControlEvent as the ui32Event parameter.
//
//*****************************************************************************
#define GPTIMER_EVENT_POS_EDGE    0x00000000  // Count positive edges
#define GPTIMER_EVENT_NEG_EDGE    0x00000404  // Count negative edges
#define GPTIMER_EVENT_BOTH_EDGES  0x00000C0C  // Count both edges

//*****************************************************************************
//
// Values that can be passed to most of the timer APIs as the ui32Timer
// parameter.
//
//*****************************************************************************
#define GPTIMER_A                 0x000000ff  // Timer A
#define GPTIMER_B                 0x0000ff00  // Timer B
#define GPTIMER_BOTH              0x0000ffff  // Timer Both

//*****************************************************************************
//
// Values that can be passed to TimerSynchronize as the ui32Timers parameter.
//
//*****************************************************************************
#define GPTIMER_0A_SYNC           0x00000001  // Synchronize Timer 0A
#define GPTIMER_0B_SYNC           0x00000002  // Synchronize Timer 0B
#define GPTIMER_1A_SYNC           0x00000004  // Synchronize Timer 1A
#define GPTIMER_1B_SYNC           0x00000008  // Synchronize Timer 1B
#define GPTIMER_2A_SYNC           0x00000010  // Synchronize Timer 2A
#define GPTIMER_2B_SYNC           0x00000020  // Synchronize Timer 2B
#define GPTIMER_3A_SYNC           0x00000040  // Synchronize Timer 3A
#define GPTIMER_3B_SYNC           0x00000080  // Synchronize Timer 3B

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void TimerEnable(uint32_t ui32Base, uint32_t ui32Timer);
extern void TimerDisable(uint32_t ui32Base, uint32_t ui32Timer);
extern void TimerConfigure(uint32_t ui32Base, uint32_t ui32Config);
extern void TimerControlLevel(uint32_t ui32Base, uint32_t ui32Timer,
      bool bInvert);
extern void TimerControlTrigger(uint32_t ui32Base, uint32_t ui32Timer,
bool bEnable);
extern void TimerControlEvent(uint32_t ui32Base, uint32_t ui32Timer,
      uint32_t ui32Event);
extern void TimerControlStall(uint32_t ui32Base, uint32_t ui32Timer,
      bool bStall);
extern void TimerControlWaitOnTrigger(uint32_t ui32Base,
      uint32_t ui32Timer,
      bool bWait);
extern void TimerPrescaleSet(uint32_t ui32Base, uint32_t ui32Timer,
     uint32_t ui32Value);
extern uint32_t TimerPrescaleGet(uint32_t ui32Base,
      uint32_t ui32Timer);
extern void TimerPrescaleMatchSet(uint32_t ui32Base,
  uint32_t ui32Timer,
                                  uint32_t ui32Value);
extern uint32_t TimerPrescaleMatchGet(uint32_t ui32Base,
   uint32_t ui32Timer);
extern void TimerLoadSet(uint32_t ui32Base, uint32_t ui32Timer,
 uint32_t ui32Value);
extern uint32_t TimerLoadGet(uint32_t ui32Base,
  uint32_t ui32Timer);
extern uint32_t TimerValueGet(uint32_t ui32Base,
   uint32_t ui32Timer);
extern void TimerMatchSet(uint32_t ui32Base, uint32_t ui32Timer,
  uint32_t ui32Value);
extern uint32_t TimerMatchGet(uint32_t ui32Base,
   uint32_t ui32Timer);
extern void TimerIntRegister(uint32_t ui32Base, uint32_t ui32Timer,
     void (*pfnHandler)(void));
extern void TimerIntUnregister(uint32_t ui32Base, uint32_t ui32Timer);
extern void TimerIntEnable(uint32_t ui32Base, uint32_t ui32IntFlags);
extern void TimerIntDisable(uint32_t ui32Base, uint32_t ui32IntFlags);
extern uint32_t TimerIntStatus(uint32_t ui32Base, bool bMasked);
extern void TimerIntClear(uint32_t ui32Base, uint32_t ui32IntFlags);
extern void TimerSynchronize(uint32_t ui32Base, uint32_t ui32Timers);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __GPTIMER_H__
