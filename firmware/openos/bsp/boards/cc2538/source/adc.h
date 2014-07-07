/******************************************************************************
*  Filename:       adc.h
*  Revised:        $Date: 2013-01-21 15:25:21 +0100 (Mon, 21 Jan 2013) $
*  Revision:       $Revision: 9178 $
*
*  Description:    Prototypes for the SOC ADC API
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

#ifndef __ADC_H__
#define __ADC_H__

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
// Values that can be passed to SOCADCSingleConfigure  as the ui32Resolution
// parameter (Resolution or decimation rate).
//
//*****************************************************************************
#define SOCADC_7_BIT      0x00000000 //  64 decimation rate ( 7 bits ENOB)
#define SOCADC_9_BIT      0x00000010 // 128 decimation rate ( 9 bits ENOB)
#define SOCADC_10_BIT     0x00000020 // 256 decimation rate (10 bits ENOB)
#define SOCADC_12_BIT     0x00000030 // 512 decimation rate (12 bits ENOB)

//*****************************************************************************
//
// The following defines can be used to extract the significant data
// depending on the chosen decimation rate
//
//*****************************************************************************
#define SOCADC_7_BIT_MASK     0xfe00 // Mask for getting data( 7 bits ENOB)
#define SOCADC_9_BIT_MASK     0xff80 // Mask for getting data( 9 bits ENOB)
#define SOCADC_10_BIT_MASK    0xffc0 // Mask for getting data(10 bits ENOB)
#define SOCADC_12_BIT_MASK    0xfff0 // Mask for getting data(12 bits ENOB)

#define SOCADC_7_BIT_RSHIFT        9 // Mask for getting data( 7 bits ENOB)
#define SOCADC_9_BIT_RSHIFT        7 // Mask for getting data( 9 bits ENOB)
#define SOCADC_10_BIT_RSHIFT       6 // Mask for getting data(10 bits ENOB)
#define SOCADC_12_BIT_RSHIFT       4 // Mask for getting data(12 bits ENOB)

//*****************************************************************************
//
// Values that can be passed to SOCADCSingleConfigure as the 
// ui32Reference parameter (reference voltage).
//
//*****************************************************************************
#define SOCADC_REF_INTERNAL    0x00000000 // Internal reference
#define SOCADC_REF_EXT_AIN7    0x00000040 // External reference on AIN7 pin
#define SOCADC_REF_AVDD5       0x00000080 // AVDD5 pin
#define SOCADC_REF_EXT_AIN67   0x000000c0 // External reference on AIN6-AIN7
                                          // differential input pins
//*****************************************************************************
//
// Values that can be passed to SOCADCSingleStart as the ui32Channel
// parameter (input channel).
//
//*****************************************************************************
#define SOCADC_AIN0       0x00000000  // Single ended Pad PA0
#define SOCADC_AIN1       0x00000001  // Single ended Pad PA1
#define SOCADC_AIN2       0x00000002  // Single ended Pad PA2
#define SOCADC_AIN3       0x00000003  // Single ended Pad PA3
#define SOCADC_AIN4       0x00000004  // Single ended Pad PA4
#define SOCADC_AIN5       0x00000005  // Single ended Pad PA5
#define SOCADC_AIN6       0x00000006  // Single ended Pad PA6
#define SOCADC_AIN7       0x00000007  // Single ended Pad PA7
#define SOCADC_AIN01      0x00000008  // Differential Pads PA0-PA1
#define SOCADC_AIN23      0x00000008  // Differential Pads PA2-PA3
#define SOCADC_AIN45      0x00000008  // Differential Pads PA4-PA5
#define SOCADC_AIN67      0x00000008  // Differential Pads PA6-PA7
#define SOCADC_GND        0x0000000c  // Ground
#define SOCADC_TEMP_SENS  0x0000000e  // On-chip temperature sensor
#define SOCADC_VDD        0x0000000f  // Vdd/3

//*****************************************************************************
//
// Values that can be passed to SOCADCStart as the ui32StartSelect
// parameter (Start Selection Criteria).
//
//*****************************************************************************
#define SOCADC_FULLSPEED  0x00000010  // Full speed, do not wait for triggers
#define SOCADC_TIMER_COMP 0x00000020  // GP Timer 0, Timber A compare event
#define SOCADC_ONE_SHOT   0x00000030  // Do a single sample
  
//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void SOCADCIntRegister(void (*pfnHandler)(void));
extern void SOCADCIntUnregister(void);

extern void SOCADCSingleConfigure(uint32_t ui32Resolution, 
                                  uint32_t ui32Reference);
extern void SOCADCSingleStart(uint32_t ui32Channel);

extern uint16_t SOCADCDataGet(void);
extern bool SOCADCEndOfCOnversionGet(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __ADC_H__
