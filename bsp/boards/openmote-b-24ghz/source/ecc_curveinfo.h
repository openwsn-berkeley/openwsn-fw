/******************************************************************************
*  Filename:       ecc_curveinfo.h
*  Revised:        $Date: 2012-10-01 11:15:04 -0700 (Mon, 01 Oct 2012) $
*  Revision:       $Revision: 31660 $
*
*  Description:    Typedef for the ecc curve information.
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

#ifndef __ECC_CURVES_H__
#define __ECC_CURVES_H__

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
// A structure which contains the necessary elements of the
// Elliptical curve cryptography's (ECC) prime curve.
//
//*****************************************************************************
typedef struct _curveInfo
{
  //
  // Name of the curve.
  //
  char*       name;

  //
  // Size of the curve in 32-bit word.
  //
  uint8_t     ui8Size;

  //
  // The prime that defines the field of the curve.
  //
  uint32_t*   pui32Prime;

  //
  // Order of the curve.
  //
  uint32_t*   pui32N;

  //
  // Co-efficient a of the equation.
  //
  uint32_t*   pui32A;

  //
  // co-efficient b of the equation.
  //
  uint32_t*   pui32B;

  //
  // x co-ordinate value of the generator point.
  //
  uint32_t*   pui32Gx;

  //
  // y co-ordinate value of the generator point.
  //
  uint32_t*   pui32Gy;
}
tECCCurveInfo;

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __ECC_CURVES_H__
