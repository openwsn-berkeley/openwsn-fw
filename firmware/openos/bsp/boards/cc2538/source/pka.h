/******************************************************************************
*  Filename:       pka.h
*  Revised:        $Date: 2012-10-01 11:15:04 -0700 (Mon, 01 Oct 2012) $
*  Revision:       $Revision: 31660 $
*
*  Description:    Type definition and function prototypes needed to
*                  interface with the public key accelerator hardware
*                  engine on CC2538.
*
*  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
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

#ifndef __PKA_H__
#define __PKA_H__

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
#include "ecc_curveinfo.h"

//*****************************************************************************
//
// Function return values
//
//*****************************************************************************
#define PKA_STATUS_SUCCESS             0 // Success
#define PKA_STATUS_FAILURE             1 // Failure
#define PKA_STATUS_INVALID_PARAM       2 // Invalid parameter
#define PKA_STATUS_BUF_UNDERFLOW       3 // Buffer underflow
#define PKA_STATUS_RESULT_0            4 // Result is all zeros
#define PKA_STATUS_A_GR_B              5 // Big number compare return status if
                                         // the first big num is greater than
                                         // the second.
#define PKA_STATUS_A_LT_B              6 // Big number compare return status if
                                         // the first big num is less than the
                                         // second.
#define PKA_STATUS_OPERATION_INPRG     7 // PKA operation is in progress.
#define PKA_STATUS_OPERATION_NOT_INPRG 8 // No PKA operation is in progress.

//*****************************************************************************
//
// A structure containing the pointers to the values of x and y co-ordinates of
// the Elliptical Curve point.
//
//*****************************************************************************
typedef struct _ECPt
{
  //
  // Pointer to value of the x co-ordinate of the ec point.
  //
  uint32_t* pui32X;

  //
  // Pointer to value of the y co-ordinate of the ec point.
  //
  uint32_t* pui32Y;
}
tECPt;

//*****************************************************************************
//
// PKA function return type.
//
//*****************************************************************************
typedef uint8_t tPKAStatus;

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void PKAEnableInt(void);
extern void PKADisableInt(void);
extern void PKAClearInt(void);
extern void PKARegInt(void(*pfnHandler)(void));
extern void PKAUnRegInt(void);
extern tPKAStatus PKAGetOpsStatus(void);
extern tPKAStatus PKABigNumModStart(uint32_t* pui32BNum, uint8_t ui8BNSize,
                                    uint32_t* pui32Modulus, uint8_t ui8ModSize,
                                    uint32_t* pui32ResultVector);
extern tPKAStatus PKABigNumModGetResult(uint32_t* pui32ResultBuf,
                                        uint8_t ui8Size,
                                        uint32_t ui32ResVectorLoc);
extern tPKAStatus PKABigNumCmpStart(uint32_t* pui32BNum1, uint32_t* pui32BNum2,
                                    uint8_t ui8Size);
extern tPKAStatus PKABigNumCmpGetResult(void);
extern tPKAStatus PKABigNumInvModStart(uint32_t* pui32BNum, uint8_t ui8BNSize,
                                       uint32_t* pui32Modulus, uint8_t ui8Size,
                                       uint32_t* pui32ResultVector);
extern tPKAStatus PKABigNumInvModGetResult(uint32_t* pui32ResultBuf,
                                           uint8_t ui8Size,
                                           uint32_t ui32ResVectorLoc);
extern tPKAStatus PKABigNumMultiplyStart(uint32_t* pui32Xplicand,
                                         uint8_t ui8XplicandSize,
                                         uint32_t* pui32Xplier,
                                         uint8_t ui8XplierSize,
                                         uint32_t* pui32ResultVector);
extern tPKAStatus PKABigNumMultGetResult(uint32_t* pui32ResultBuf,
                                         uint32_t* pui32Len,
                                         uint32_t ui32ResVectorLoc);
extern tPKAStatus PKABigNumAddStart(uint32_t* pui32BN1, uint8_t ui8BN1Size,
                                    uint32_t* pui32BN2, uint8_t ui8BN2Size,
                                    uint32_t* pui32ResultVector);
extern tPKAStatus PKABigNumAddGetResult(uint32_t* pui32ResultBuf,
                                        uint32_t* pui32Len,
                                        uint32_t ui32resVectorLoc);
extern tPKAStatus PKAECCMultiplyStart(uint32_t* pui32Scalar,
                                      tECPt* ptEcPt,
                                      tECCCurveInfo* ptCurve,
                                      uint32_t* pui32ResultVector);
extern tPKAStatus PKAECCMultiplyGetResult(tECPt* ptOutEcPt,
                                          uint32_t ui32ResVectorLoc);
extern tPKAStatus PKAECCMultGenPtStart(uint32_t* pui32Scalar,
                                       tECCCurveInfo* ptCurve,
                                       uint32_t* pui32ResultVector);
extern tPKAStatus PKAECCMultGenPtGetResult(tECPt* ptOutEcPt,
                                           uint32_t pui32ResVectorLoc);
extern tPKAStatus PKAECCAddStart(tECPt* ptEcPt1, tECPt* ptEcPt2,
                                 tECCCurveInfo* ptCurve,
                                 uint32_t* pui32ResultVector);
extern tPKAStatus PKAECCAddGetResult(tECPt* ptOutEcPt, uint32_t ui32ResultLoc);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __PKA_H__
