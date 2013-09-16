/******************************************************************************
*  Filename:       ccm.h
*  Revised:        $Date: 2013-03-22 16:13:31 +0100 (Fri, 22 Mar 2013) $
*  Revision:       $Revision: 9513 $
*
*  Description:    Support for Hardware CCM encryption and authentication.
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

#ifndef __CCM_H__
#define __CCM_H__

#include "hw_types.h"

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
    
extern uint8_t CCMAuthEncryptStart (bool bEncrypt,
                                    uint8_t ui8Mval, 
                                    uint8_t *pui8N, 
                                    uint8_t *pui8M, 
                                    uint16_t ui16LenM, 
                                    uint8_t *pui8A, 
                                    uint16_t ui16LenA, 
                                    uint8_t ui8KeyLocation, 
                                    uint8_t *pui8Cstate, 
                                    uint8_t ui8CCMLVal, 
                                    uint8_t ui8IntEnable);
extern uint8_t CCMAuthEncryptCheckResult(void);
extern uint8_t CCMAuthEncryptGetResult(uint8_t ui8Mval,  
                                       uint16_t ui16LenM, 
                                       uint8_t *pui8Cstate);
extern uint8_t CCMInvAuthDecryptStart (bool bDecrypt,
                                       uint8_t ui8Mval, 
                                       uint8_t *pui8N, 
                                       uint8_t *pui8C, 
                                       uint16_t ui16LenC, 
                                       uint8_t *pui8A, 
                                       uint16_t ui16LenA,
                                       uint8_t ui8KeyLocation, 
                                       uint8_t *pui8Cstate, 
                                       uint8_t ui8CCMLVal, 
                                       uint8_t ui8IntEnable);
extern uint8_t CCMInvAuthDecryptCheckResult(void);
extern uint8_t CCMInvAuthDecryptGetResult(uint8_t ui8Mval, 
                                          uint8_t *pui8C, 
                                          uint16_t ui16LenC, 
                                          uint8_t *pui8Cstate);
    
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif  // __CCM_H__
