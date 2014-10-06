/******************************************************************************
*  Filename:       watchdog.h
*  Revised:        $Date: 2013-04-04 15:31:10 +0200 (Thu, 04 Apr 2013) $
*  Revision:       $Revision: 9634 $
*
*  Description:    Prototypes for the Watchdog Timer API.
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

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

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
// The following are values that can be passed to the WatchdogEnable()
//
//*****************************************************************************
#define WATCHDOG_INTERVAL_32768 0x00000000 // Timer select: Twdt x 32768
#define WATCHDOG_INTERVAL_8192  0x00000001 // Timer select: Twdt x 8192
#define WATCHDOG_INTERVAL_512   0x00000002 // Timer select: Twdt x 512
#define WATCHDOG_INTERVAL_64    0x00000003 // Timer select: Twdt x 64

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void WatchdogEnable(uint32_t ui32Interval);
extern void WatchdogClear(void);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __WATCHDOG_H__
