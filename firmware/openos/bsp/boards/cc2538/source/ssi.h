/******************************************************************************
*  Filename:       ssi.h
*  Revised:        $Date: 2013-01-25 10:58:16 +0100 (Fri, 25 Jan 2013) $
*  Revision:       $Revision: 9248 $
*
*  Description:    Prototypes for the Synchronous Serial Interface Driver.
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

#ifndef __SSI_H__
#define __SSI_H__

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
// Values that can be passed to SSIIntEnable, SSIIntDisable, and SSIIntClear
// as the ui32IntFlags parameter, and returned by SSIIntStatus.
//
//*****************************************************************************
#define SSI_TXFF                0x00000008  // TX FIFO half full or less
#define SSI_RXFF                0x00000004  // RX FIFO half full or more
#define SSI_RXTO                0x00000002  // RX timeout
#define SSI_RXOR                0x00000001  // RX overrun

//*****************************************************************************
//
// Values that can be passed to SSIConfigSetExpClk.
//
//*****************************************************************************
#define SSI_FRF_MOTO_MODE_0     0x00000000  // Moto fmt, polarity 0, phase 0
#define SSI_FRF_MOTO_MODE_1     0x00000002  // Moto fmt, polarity 0, phase 1
#define SSI_FRF_MOTO_MODE_2     0x00000001  // Moto fmt, polarity 1, phase 0
#define SSI_FRF_MOTO_MODE_3     0x00000003  // Moto fmt, polarity 1, phase 1
#define SSI_FRF_TI              0x00000010  // TI frame format
#define SSI_FRF_NMW             0x00000020  // National MicroWire frame format

#define SSI_MODE_MASTER         0x00000000  // SSI master
#define SSI_MODE_SLAVE          0x00000001  // SSI slave
#define SSI_MODE_SLAVE_OD       0x00000002  // SSI slave with output disabled

//*****************************************************************************
//
// Values that can be passed to SSIDMAEnable() and SSIDMADisable().
//
//*****************************************************************************
#define SSI_DMA_TX              0x00000002  // Enable DMA for transmit
#define SSI_DMA_RX              0x00000001  // Enable DMA for receive

//*****************************************************************************
//
// Values that can be passed to SSIClockSourceSet() or returned from
// SSIClockSourceGet().
//
//*****************************************************************************
#define SSI_CLOCK_SYSTEM        0x00000000
#define SSI_CLOCK_PIOSC         0x00000001

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void SSIConfigSetExpClk(uint32_t ui32Base, uint32_t ui32SSIClk,
                               uint32_t ui32Protocol, uint32_t ui32Mode,
                               uint32_t ui32BitRate,
                               uint32_t ui32DataWidth);
extern void SSIDataGet(uint32_t ui32Base, uint32_t *pui32Data);
extern int32_t SSIDataGetNonBlocking(uint32_t ui32Base,
                                  uint32_t *pui32Data);
extern void SSIDataPut(uint32_t ui32Base, uint32_t ui32Data);
extern int32_t SSIDataPutNonBlocking(uint32_t ui32Base, uint32_t ui32Data);
extern void SSIDisable(uint32_t ui32Base);
extern void SSIEnable(uint32_t ui32Base);
extern void SSIIntClear(uint32_t ui32Base, uint32_t ui32IntFlags);
extern void SSIIntDisable(uint32_t ui32Base, uint32_t ui32IntFlags);
extern void SSIIntEnable(uint32_t ui32Base, uint32_t ui32IntFlags);
extern void SSIIntRegister(uint32_t ui32Base, void (*pfnHandler)(void));
extern uint32_t SSIIntStatus(uint32_t ui32Base, bool bMasked);
extern void SSIIntUnregister(uint32_t ui32Base);
extern void SSIDMAEnable(uint32_t ui32Base, uint32_t ui32DMAFlags);
extern void SSIDMADisable(uint32_t ui32Base, uint32_t ui32DMAFlags);
extern bool SSIBusy(uint32_t ui32Base);
extern void SSIClockSourceSet(uint32_t ui32Base, uint32_t ui32Source);
extern uint32_t SSIClockSourceGet(uint32_t ui32Base);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __SSI_H__
