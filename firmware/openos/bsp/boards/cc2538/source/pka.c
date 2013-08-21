/******************************************************************************
*  Filename:       pka.c
*  Revised:        $Date: 2012-10-01 11:15:04 -0700 (Mon, 01 Oct 2012) $
*  Revision:       $Revision: 31660 $
*
*  Description:    Driver for the PKA HW module.
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

//*****************************************************************************
//
//! \addtogroup pka_driver
//! @{
//
//*****************************************************************************

#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_pka.h"
#include "hw_types.h"
#include "interrupt.h"
#include "pka.h"
#include "sys_ctrl.h"
#include "debug.h"

//*****************************************************************************
//
// Macro definition for NULL
//
//*****************************************************************************

#ifndef NULL
#define NULL                    ((void*)0)
#endif

//*****************************************************************************
//
// Define for the maximum curve size supported by the PKA module in 32 bit
// word.
// \note PKA hardware module can support upto 384 bit curve size due to the
//       2K of PKA RAM.
//
//*****************************************************************************
#define PKA_MAX_CURVE_SIZE_32_BIT_WORD \
                                12

//*****************************************************************************
//
// Define for the maximum length of the big number supported by the PKA module
// in 32 bit word.
//
//*****************************************************************************
#define PKA_MAX_LEN_IN_32_BIT_WORD \
                                PKA_MAX_CURVE_SIZE_32_BIT_WORD

//*****************************************************************************
//
// Define for the PKA RAM size.
//
//****************************************************************************
#define PKA_RAM_SIZE            2000


//*****************************************************************************
//
//! Enables the PKA interrupt.
//!
//! This function enables the PKA interrupt.
//!
//! \return None.
//
//*****************************************************************************
void
PKAEnableInt(void)
{
    //
    // Enable the PKA interrupt.
    //
    IntEnable(INT_PKA);
}

//*****************************************************************************
//
//! Disables the PKA interrupt.
//!
//! This function disables the PKA interrupt.
//!
//! \return None.
//
//*****************************************************************************
void
PKADisableInt( void )
{
    //
    // Disables the PKA interrupt.
    //
    IntDisable(INT_PKA);
}

//*****************************************************************************
//
//! Clears the PKA interrupt.
//!
//! This function unpends PKA interrupt.  This will cause any previously
//! generated PKA interrupts that have not been handled yet to be discarded.
//!
//! \return None.
//
//*****************************************************************************
void
PKAClearInt(void)
{
    //
    // UnPends the PKA interrupt.
    //
    IntPendClear(INT_PKA);
}

//*****************************************************************************
//
//! Registers an interrupt handler for PKA interrupt.
//!
//! \param pfnHandler is a pointer to the function to be called when the
//! PKA interrupt occurs.
//!
//! This function does the actual registering of the interrupt handler.  This
//! will not enable the PKA interrupt in the interrupt controller, a call to
//! the function \sa PKAEnableInt() is needed to enable the PKA interrupt.
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None.
//
//*****************************************************************************
void
PKARegInt(void (*pfnHandler)(void))
{
    //
    // Register the interrupt handler.
    //
    IntRegister(INT_PKA, pfnHandler);
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the PKA interrupt.
//!
//! This function deregisters the interrupt service routine.  This function
//! will not disable the interrupt and an explicit call to \sa PKADisableInt()
//! is needed.
//!
//! \return None.
//
//*****************************************************************************
void
PKAUnRegInt(void)
{
    //
    // Unregister the interrupt handler.
    //
    IntUnregister(INT_PKA);
}

//*****************************************************************************
//
//! Provides the PKA operation status.
//!
//! This function provides information on whether any PKA operation is in
//! progress or not. This function allows to check the PKA operation status
//! before starting any new PKA operation.
//!
//! \return Returns: 
//! - \b PKA_STATUS_INPRG if the PKA operation is in progress.
//! - \b PKA_STATUS_OPERATION_NOT_INPRG if the PKA operation is not in progress.
//
//*****************************************************************************
tPKAStatus
PKAGetOpsStatus(void)
{
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }
    else
    {
        return (PKA_STATUS_OPERATION_NOT_INPRG);
    }
}

//*****************************************************************************
//
//! Starts the big number modulus operation.
//!
//! \param pui32BNum is the pointer to the big number on which modulo operation
//! needs to be carried out.
//! \param ui8BNSize is the size of the big number \sa pui32BNum in 32-bit
//! word.
//! \param pui32Modulus is the pointer to the divisor.
//! \param ui8ModSize is the size of the divisor \sa pui32Modulus.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the modulo operation on the big num \sa pui32BNum
//! using the divisor \sa pui32Modulus.  The PKA RAM location where the result
//! will be available is stored in \sa pui32ResultVector.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKABigNumModStart(uint32_t* pui32BNum, uint8_t ui8BNSize,
                  uint32_t* pui32Modulus, uint8_t ui8ModSize,
                  uint32_t* pui32ResultVector)
{
    uint8_t extraBuf;
    uint32_t offset;
    int i;

    //
    // Check the arguments.
    //
    ASSERT(NULL != pui32BNum);
    ASSERT(NULL != pui32Modulus);
    ASSERT(NULL != pui32ResultVector);

    //
    // make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // calculate the extra buffer requirement.
    //
    extraBuf = 2 + ui8ModSize % 2;

    offset = 0;

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the number will be stored.
    //
    HWREG( (PKA_APTR) ) = offset >>2;

    //
    // Load the number in PKA RAM
    //
    for(i = 0; i < ui8BNSize; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = pui32BNum[i];
    }

    //
    // determine the offset for the next data input.
    //
    offset += 4 * (i + ui8BNSize % 2);

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the divisor will be stored.
    //
    HWREG( (PKA_BPTR) ) = offset >> 2;

    //
    // Load the divisor in PKA RAM.
    //
    for(i = 0; i < ui8ModSize;  i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = pui32Modulus[i];
    }

    //
    // determine the offset for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Copy the result vector address location.
    //
    *pui32ResultVector = PKA_RAM_BASE + offset;

    //
    // Load C ptr with the result location in PKA RAM
    //
    HWREG( (PKA_CPTR) ) = offset >> 2;

    //
    // Load A length registers with Big number length in 32 bit words.
    //
    HWREG( (PKA_ALENGTH) ) = ui8BNSize;

    //
    // Load B length registers  Divisor length in 32-bit words.
    //
    HWREG( (PKA_BLENGTH) ) = ui8ModSize;

    //
    // Start the PKCP modulo operation by setting the PKA Function register.
    //
    HWREG( (PKA_FUNCTION) ) = (PKA_FUNCTION_RUN | PKA_FUNCTION_MODULO);

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of the big number modulus operation.
//!
//! \param pui32ResultBuf is the pointer to buffer where the result needs to
//! be stored.
//! \param ui8Size is the size of the provided buffer in 32 bit size word.
//! \param ui32ResVectorLoc is the address of the result location which
//! was provided by the start function \sa PKABigNumModStart().
//!
//! This function gets the result of the big number modulus operation which was
//! previously started using the function \sa PKABigNumModStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if successful.
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_BUF_UNDERFLOW, if the \e ui8Size is less than the length
//! of the result.
//
//*****************************************************************************
tPKAStatus
PKABigNumModGetResult(uint32_t* pui32ResultBuf,uint8_t ui8Size,
                      uint32_t ui32ResVectorLoc)
{
    uint32_t regMSWVal;
    uint32_t len;
    int i;

    //
    // Check the arguments.
    //
    ASSERT(NULL != pui32ResultBuf);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    //
    // verify that the operation is complete.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    //  Get the MSW register value.
    //
    regMSWVal = HWREG(PKA_DIVMSW);

    //
    // Check to make sure that the result vector is not all zeroes.
    //
    if(regMSWVal & PKA_DIVMSW_RESULT_IS_ZERO)
    {
        return (PKA_STATUS_RESULT_0);
    }

    //
    // Get the length of the result.
    //
    len = ((regMSWVal & PKA_DIVMSW_MSW_ADDRESS_M) + 1) -
          ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

    //
    // If the size of the buffer provided is less than the result length than
    // return error.
    //
    if(ui8Size < len)
    {
        return (PKA_STATUS_BUF_UNDERFLOW);
    }

    //
    // copy the result from vector C into the pResult.
    //
    for(i = 0; i < len; i++)
    {
        pui32ResultBuf[i]= HWREG( (ui32ResVectorLoc + 4*i) );
    }

    return (PKA_STATUS_SUCCESS);
} // PKABigNumModGetResult()

//*****************************************************************************
//
//! Starts the comparison of two big numbers.
//!
//! \param pui32BNum1 is the pointer to the first big number.
//! \param pui32BNum2 is the pointer to the second big number.
//! \param ui8Size is the size of the big number in 32 bit size word.
//!
//! This function starts the comparison of two big numbers pointed by
//! \e pui32BNum1 and \e pui32BNum2.
//! Note this function expects the size of the two big numbers equal.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKABigNumCmpStart(uint32_t* pui32BNum1, uint32_t* pui32BNum2, uint8_t ui8Size)
{
    uint32_t offset;
    int i;

    //
    // Check the arguments.
    //
    ASSERT(NULL != pui32BNum1);
    ASSERT(NULL != pui32BNum2);

    offset = 0;

    //
    // Make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the first big number will be stored.
    //
    HWREG( (PKA_APTR) ) = offset >> 2;

    //
    // Load the first big number in PKA RAM.
    //
    for(i = 0; i < ui8Size; i++)
    {
        HWREG( (PKA_RAM_BASE + offset + 4*i) ) = pui32BNum1[i];
    }

    //
    // Determine the offset in PKA RAM for the next pointer.
    //
    offset += 4 * (i + ui8Size % 2);

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the second big number will be stored.
    //
    HWREG( (PKA_BPTR) ) = offset >> 2;

    //
    // Load the second big number in PKA RAM.
    //
    for(i = 0; i < ui8Size;  i++)
    {
        HWREG( (PKA_RAM_BASE + offset + 4*i) ) = pui32BNum2[i];
    }

    //
    // Load length registers in 32 bit word size.
    //
    HWREG( (PKA_ALENGTH) ) = ui8Size;

    //
    // Set the PKA Function register for the Compare operation
    // and start the operation.
    //
    HWREG( (PKA_FUNCTION) ) = (PKA_FUNCTION_RUN | PKA_FUNCTION_COMPARE);

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of the comparison operation of two big numbers.
//!
//! This function provides the results of the comparison of two big numbers
//! which was started using the \sa PKABigNumCmpStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_OPERATION_INPRG if the operation is in progress.
//! - \b PKA_STATUS_SUCCESS if the two big numbers are equal.
//! - \b PKA_STATUS_A_GR_B  if the first number is greater than the second.
//! - \b PKA_STATUS_A_LT_B if the first number is less than the second.
//
//*****************************************************************************
tPKAStatus
PKABigNumCmpGetResult(void)
{
    tPKAStatus status;

    //
    // verify that the operation is complete.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        status = PKA_STATUS_OPERATION_INPRG;
        return (status);
    }

    //
    // Check the COMPARE register.
    //
    switch(HWREG(PKA_COMPARE))
    {
        case PKA_COMPARE_A_EQUALS_B:
            status = PKA_STATUS_SUCCESS;
            break;

        case PKA_COMPARE_A_GREATER_THAN_B:
            status = PKA_STATUS_A_GR_B;
            break;

        case PKA_COMPARE_A_LESS_THAN_B:
            status = PKA_STATUS_A_LT_B;
            break;

        default:
            status = PKA_STATUS_FAILURE;
            break;
    }

    return (status);
}

//*****************************************************************************
//
//! Starts the big number inverse modulo operation.
//!
//! \param pui32BNum is the pointer to the buffer containing the big number
//! (dividend).
//! \param ui8BNSize is the size of the \e pui32BNum in 32 bit word.
//! \param pui32Modulus is the pointer to the buffer containing the divisor.
//! \param ui8Size is the size of the divisor in 32 bit word.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the the inverse modulo operation on \e pui32BNum
//! using the divisor \e pui32Modulus.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKABigNumInvModStart(uint32_t* pui32BNum, uint8_t ui8BNSize,
                     uint32_t* pui32Modulus, uint8_t ui8Size,
                     uint32_t* pui32ResultVector)
{
    uint32_t offset;
    int i;

    //
    // Check the arguments.
    //
    ASSERT(NULL != pui32BNum);
    ASSERT(NULL != pui32Modulus);
    ASSERT(NULL != pui32ResultVector);

    offset = 0;

    //
    // Make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the number will be stored.
    //
    HWREG( (PKA_APTR) ) = offset >>2;

    //
    // Load the \e pui32BNum number in PKA RAM.
    //
    for(i = 0; i < ui8BNSize; i++)
    {
        HWREG( (PKA_RAM_BASE + offset + 4*i) ) = pui32BNum[i];
    }

    //
    // Determine the offset for next data.
    //
    offset += 4 * (i + ui8BNSize % 2);

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the modulus will be stored.
    //
    HWREG( (PKA_BPTR) ) = offset >> 2;

    //
    // Load the \e pui32Modulus divisor in PKA RAM.
    //
    for(i = 0; i < ui8Size;  i++)
    {
        HWREG( (PKA_RAM_BASE + offset + 4*i) ) = pui32Modulus[i];
    }

    //
    // Determine the offset for result data.
    //
    offset += 4 * (i + ui8Size % 2);

    //
    // Copy the result vector address location.
    //
    *pui32ResultVector = PKA_RAM_BASE + offset;

    //
    // Load D ptr with the result location in PKA RAM.
    //
    HWREG( (PKA_DPTR) ) = offset >> 2;

    //
    // Load the respective length registers.
    //
    HWREG( (PKA_ALENGTH) ) = ui8BNSize;
    HWREG( (PKA_BLENGTH) ) = ui8Size;

    //
    // set the PKA function to InvMod operation and the start the operation.
    //
    HWREG( (PKA_FUNCTION) ) = 0x0000F000;

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of the big number inverse modulo operation.
//!
//! \param pui32ResultBuf is the pointer to buffer where the result needs to be
//! stored.
//! \param ui8Size is the size of the provided buffer in 32 bit ui8Size
//! word.
//! \param ui32ResVectorLoc is the address of the result location which
//! was provided by the start function \sa PKABigNumInvModStart().
//!
//! This function gets the result of the big number inverse modulo operation
//! previously started using the function \sa PKABigNumInvModStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if the operation is successful. 
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy performing 
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
//! then the result. 
//
//*****************************************************************************
tPKAStatus
PKABigNumInvModGetResult(uint32_t* pui32ResultBuf, uint8_t ui8Size,
                         uint32_t ui32ResVectorLoc)
{
    uint32_t regMSWVal;
    uint32_t len;
    int i;

    //
    // Check the arguments.
    //
    ASSERT(NULL != pui32ResultBuf);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    //
    // Verify that the operation is complete.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Get the MSW register value.
    //
    regMSWVal = HWREG(PKA_MSW);

    //
    // Check to make sure that the result vector is not all zeroes.
    //
    if(regMSWVal & PKA_MSW_RESULT_IS_ZERO)
    {
        return (PKA_STATUS_RESULT_0);
    }

    //
    // Get the length of the result
    //
    len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1) -
          ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

    //
    // Check if the provided buffer length is adequate to store the result
    // data.
    //
    if(ui8Size < len)
    {
        return (PKA_STATUS_BUF_UNDERFLOW);
    }

    //
    // Copy the result from vector C into the \e pui32ResultBuf.
    for(i = 0; i < len; i++)
    {
        pui32ResultBuf[i]= HWREG( (ui32ResVectorLoc + 4*i) );
    }

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Starts the big number multiplication.
//!
//! \param pui32Xplicand is the pointer to the buffer containing the big
//! number multiplicand.
//! \param ui8XplicandSize is the size of the multiplicand in 32-bit word.
//! \param pui32Xplier is the pointer to the buffer containing the big
//! number multiplier.
//! \param ui8XplierSize is the size of the multiplier in 32-bit word.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the multiplication of the two big numbers.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKABigNumMultiplyStart(uint32_t* pui32Xplicand, uint8_t ui8XplicandSize,
                       uint32_t* pui32Xplier, uint8_t ui8XplierSize,
                       uint32_t* pui32ResultVector)
{
    uint32_t offset;
    int i;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != pui32Xplicand);
    ASSERT(NULL != pui32Xplier);
    ASSERT(NULL != pui32ResultVector);

    offset = 0;

    //
    // Make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the multiplicand will be stored.
    //
    HWREG( (PKA_APTR) ) = offset >> 2;

    //
    // Load the multiplicand in PKA RAM.
    //
    for(i = 0; i < ui8XplicandSize; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = *pui32Xplicand;
        pui32Xplicand++;
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + (ui8XplicandSize % 2));

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the multiplier will be stored.
    //
    HWREG( (PKA_BPTR) ) = offset >> 2;

    //
    // Load the multiplier in PKA RAM.
    //
    for(i = 0; i < ui8XplierSize; i++)
    {
        HWREG( (PKA_RAM_BASE + offset + 4*i) ) = *pui32Xplier;
        pui32Xplier++;
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + (ui8XplierSize % 2));

    //
    // Copy the result vector address location.
    //
    *pui32ResultVector = PKA_RAM_BASE + offset;

    //
    // Load C ptr with the result location in PKA RAM.
    //
    HWREG( (PKA_CPTR) ) = offset >> 2;

    //
    // Load the respective length registers.
    //
    HWREG( (PKA_ALENGTH) ) = ui8XplicandSize;
    HWREG( (PKA_BLENGTH) ) = ui8XplierSize;

    //
    // Set the PKA function to the multiplication and start it.
    //
    HWREG( (PKA_FUNCTION) ) = (PKA_FUNCTION_RUN | PKA_FUNCTION_MULTIPLY);

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the results of the big number multiplication.
//!
//! \param pui32ResultBuf is the pointer to buffer where the result needs to be
//! stored.
//! \param pui32Len is the address of the variable containing the length of the
//! buffer.  After the operation, the actual length of the resultant is stored
//! at this address.
//! \param ui32ResVectorLoc is the address of the result location which
//! was provided by the start function \sa PKABigNumMultiplyStart().
//!
//! This function gets the result of the multiplication of two big numbers
//! operation previously started using the function \sa
//! PKABigNumMultiplyStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if the operation is successful. 
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy performing 
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_FAILURE if the operation is not successful.
//! - \b PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
//! then the length of the result. 
//
//*****************************************************************************
tPKAStatus
PKABigNumMultGetResult(uint32_t* pui32ResultBuf, uint32_t* pui32Len,
                       uint32_t ui32ResVectorLoc)
{
    uint32_t regMSWVal;
    uint32_t len;
    int i;

    //
    // Check for arguments.
    //
    ASSERT(NULL != pui32ResultBuf);
    ASSERT(NULL != pui32Len);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    //
    // Verify that the operation is complete.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Get the MSW register value.
    //
    regMSWVal = HWREG(PKA_MSW);

    //
    // Check to make sure that the result vector is not all zeroes.
    //
    if(regMSWVal & PKA_MSW_RESULT_IS_ZERO)
    {
        return (PKA_STATUS_RESULT_0);
    }

    //
    // Get the length of the result.
    //
    len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1) -
          ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

    //
    // Make sure that the length of the supplied result buffer is adequate
    // to store the resultant.
    //
    if(*pui32Len < len)
    {
        return (PKA_STATUS_BUF_UNDERFLOW);
    }

    //
    // Copy the resultant length.
    //
    *pui32Len = len;

    //
    // Copy the result from vector C into the pResult.
    //
    for(i = 0; i < *pui32Len; i++)
    {
        pui32ResultBuf[i]= HWREG( (ui32ResVectorLoc + 4*i) );
    }

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Starts the addition of two big number.
//!
//! \param pui32BN1 is the pointer to the buffer containing the first
//! big mumber.
//! \param ui8BN1Size is the size of the first big number in 32-bit word.
//! \param pui32BN2 is the pointer to the buffer containing the second
//! big number.
//! \param ui8BN2Size is the size of the second big number in 32-bit word.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the addition of the two big numbers.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKABigNumAddStart(uint32_t* pui32BN1, uint8_t ui8BN1Size,
                  uint32_t* pui32BN2, uint8_t ui8BN2Size,
                  uint32_t* pui32ResultVector)
{
    uint32_t offset;
    int i;

    //
    // Check for arguments.
    //
    ASSERT(NULL != pui32BN1);
    ASSERT(NULL != pui32BN2);
    ASSERT(NULL != pui32ResultVector);

    offset = 0;

    //
    // Make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the big number 1 will be stored.
    //
    HWREG( (PKA_APTR) ) = offset >> 2;

    //
    // Load the big number 1 in PKA RAM.
    //
    for(i = 0; i < ui8BN1Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = pui32BN1[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + (ui8BN1Size % 2));

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the big number 2 will be stored.
    //
    HWREG( (PKA_BPTR) ) = offset >> 2;

    //
    // Load the big number 2 in PKA RAM.
    //
    for(i = 0; i < ui8BN2Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = pui32BN2[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + (ui8BN2Size % 2));

    //
    // Copy the result vector address location.
    //
    *pui32ResultVector = PKA_RAM_BASE + offset;

    //
    // Load C ptr with the result location in PKA RAM.
    //
    HWREG( (PKA_CPTR) ) = offset >> 2;

    //
    // Load respective length registers.
    //
    HWREG( (PKA_ALENGTH) ) = ui8BN1Size;
    HWREG( (PKA_BLENGTH) ) = ui8BN2Size;

    //
    // Set the function for the add operation and start the operation.
    //
    HWREG( (PKA_FUNCTION) ) = (PKA_FUNCTION_RUN | PKA_FUNCTION_ADD);

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of the addition operation on two big number.
//!
//! \param pui32ResultBuf is the pointer to buffer where the result
//! needs to be stored.
//! \param pui32Len is the address of the variable containing the length of
//! the buffer.  After the operation the actual length of the resultant is
//! stored at this address.
//! \param ui32ResVectorLoc is the address of the result location which
//! was provided by the start function \sa PKABigNumAddStart().
//!
//! This function gets the result of the addition operation on two big numbers,
//! previously started using the function \sa PKABigNumAddStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if the operation is successful. 
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy performing 
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_FAILURE if the operation is not successful.
//! - \b PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
//! then the length of the result. 
//
//*****************************************************************************
tPKAStatus
PKABigNumAddGetResult(uint32_t* pui32ResultBuf, uint32_t* pui32Len,
                      uint32_t ui32ResVectorLoc)
{
    uint32_t regMSWVal;
    uint32_t len;
    int i;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != pui32ResultBuf);
    ASSERT(NULL != pui32Len);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    //
    // Verify that the operation is complete.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Get the MSW register value.
    //
    regMSWVal = HWREG(PKA_MSW);

    //
    // Check to make sure that the result vector is not all zeroes.
    //
    if(regMSWVal & PKA_MSW_RESULT_IS_ZERO)
    {
        return (PKA_STATUS_RESULT_0);
    }

    //
    // Get the length of the result.
    //
    len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1) -
          ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

    //
    // Make sure that the supplied result buffer is adequate to store the
    // resultant data.
    //
    if(*pui32Len < len)
    {
        return (PKA_STATUS_BUF_UNDERFLOW);
    }

    //
    // Copy the length.
    //
    *pui32Len = len;

    //
    // Copy the result from vector C into the provided buffer.
    for(i = 0; i < *pui32Len; i++)
    {
        pui32ResultBuf[i] = HWREG( (ui32ResVectorLoc +  4*i) );
    }

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Starts ECC Multiplication.
//!
//! \param pui32Scalar is pointer to the buffer containing the scalar
//! value to be multiplied.
//! \param ptEcPt is the pointer to the structure containing the
//! elliptic curve point to be multiplied.  The point should be on the given
//! curve.
//! \param ptCurve is the pointer to the structure containing the curve
//! info.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the Elliptical curve cryptography (ECC) point
//! multiplication operation on the EC point and the scalar value.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKAECCMultiplyStart(uint32_t* pui32Scalar, tECPt* ptEcPt,
                    tECCCurveInfo* ptCurve, uint32_t* pui32ResultVector)
{
    uint8_t extraBuf;
    uint32_t offset;
    int i;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != pui32Scalar);
    ASSERT(NULL != ptEcPt);
    ASSERT(NULL != ptEcPt->pui32X);
    ASSERT(NULL != ptEcPt->pui32Y);
    ASSERT(NULL != ptCurve);
    ASSERT(ptCurve->ui8Size <= PKA_MAX_CURVE_SIZE_32_BIT_WORD);
    ASSERT(NULL != pui32ResultVector);

    offset = 0;

    //
    // Make sure no PKA operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Calculate the extra buffer requirement.
    //
    extraBuf = 2 + ptCurve->ui8Size % 2;

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the scalar will be stored.
    //
    HWREG((PKA_APTR)) = offset >> 2;

    //
    // Load the scalar in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = *pui32Scalar++;
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + (ptCurve->ui8Size % 2));

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the curve parameters will be stored.
    //
    HWREG((PKA_BPTR)) = offset >> 2;

    //
    // Write curve parameter 'p' as 1st part of vector B immediately
    // following vector A at PKA RAM
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) =
            (uint32_t)ptCurve->pui32Prime[i];
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Copy curve parameter 'a' in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32A[i];
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Copy curve parameter 'b' in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32B[i];
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Update the C ptr with the offset address of the PKA RAM location
    // where the Gx, Gy will be stored.
    //
    HWREG((PKA_CPTR)) = offset >> 2;

    //
    // Write elliptic curve point x co-ordinate value.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = ptEcPt->pui32X[i];
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Write elliptic curve point y co-ordinate value.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = ptEcPt->pui32Y[i];
    }

    //
    // Determine the offset for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Update the result location.
    //
    *pui32ResultVector =  PKA_RAM_BASE + offset;

    //
    // Load D ptr with the result location in PKA RAM.
    //
    HWREG(PKA_DPTR) = offset >> 2;

    //
    // Load length registers.
    //
    HWREG(PKA_ALENGTH) = ptCurve->ui8Size;
    HWREG(PKA_BLENGTH) = ptCurve->ui8Size;

    //
    // set the PKA function to ECC-MULT and start the operation.
    //
    HWREG(PKA_FUNCTION) = 0x0000D000;

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of ECC Multiplication
//!
//! \param ptOutEcPt is the pointer to the structure where the resultant EC
//! point will be stored.  The callee is responsible to allocate the space for
//! the ec point structure and the x and y co-ordinate as well.
//! \param ui32ResVectorLoc is the address of the result location which
//! was provided by the start function \sa PKAECCMultiplyStart().
//!
//! This function gets the result of ecc point multiplication operation on the
//! ec point and the scalar value, previously started using the function
//! \sa PKAECCMultiplyStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if the operation is successful. 
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy performing 
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_FAILURE if the operation is not successful.
//
//*****************************************************************************
tPKAStatus
PKAECCMultiplyGetResult(tECPt* ptOutEcPt, uint32_t ui32ResVectorLoc)
{
    int i;
    uint32_t addr;
    uint32_t regMSWVal;
    uint32_t len;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != ptOutEcPt);
    ASSERT(NULL != ptOutEcPt->pui32X);
    ASSERT(NULL != ptOutEcPt->pui32Y);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    //
    // Verify that the operation is completed.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    if(HWREG(PKA_SHIFT) == 0x00000000)
    {
        //
        // Get the MSW register value.
        //
        regMSWVal = HWREG(PKA_MSW);

        //
        // Check to make sure that the result vector is not all zeroes.
        //
        if(regMSWVal & PKA_MSW_RESULT_IS_ZERO)
        {
            return (PKA_STATUS_RESULT_0);
        }

        //
        // Get the length of the result
        //
        len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1) -
              ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

        addr = ui32ResVectorLoc;

        //
        // copy the x co-ordinate value of the result from vector D into
        // the \e ptOutEcPt.
        //
        for(i = 0; i < len; i++)
        {
            ptOutEcPt->pui32X[i] = HWREG(addr + 4*i);
        }

        addr += 4 * (i + 2 + len % 2);

        //
        // copy the y co-ordinate value of the result from vector D into
        // the \e ptOutEcPt.
        //
        for(i = 0; i < len; i++)
        {
            ptOutEcPt->pui32Y[i] = HWREG(addr + 4*i);
        }

        return (PKA_STATUS_SUCCESS);
    }
    else
    {
        return (PKA_STATUS_FAILURE);
    }
}

//*****************************************************************************
//
//! Starts the ECC Multiplication with Generator point.
//!
//! \param pui32Scalar is the to pointer to the buffer containing the scalar
//! value.
//! \param ptCurve is the pointer to the structure containing the curve
//! info.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the ecc point multiplication operation of the
//! scalar value with the well known generator point of the given curve.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKAECCMultGenPtStart(uint32_t* pui32Scalar, tECCCurveInfo* ptCurve,
                     uint32_t* pui32ResultVector)
{
    uint8_t extraBuf;
    uint32_t offset;
    int i;

    //
    // check for the arguments.
    //
    ASSERT(NULL != pui32Scalar);
    ASSERT(NULL != ptCurve);
    ASSERT(ptCurve->ui8Size <= PKA_MAX_CURVE_SIZE_32_BIT_WORD);
    ASSERT(NULL != pui32ResultVector);

    offset = 0;

    //
    // Make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Calculate the extra buffer requirement.
    //
    extraBuf = 2 + ptCurve->ui8Size % 2;

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the scalar will be stored.
    //
    HWREG(PKA_APTR) = offset >> 2;

    //
    // Load the scalar in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = *pui32Scalar++;
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + (ptCurve->ui8Size % 2));

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the curve parameters will be stored.
    //
    HWREG(PKA_BPTR) = offset >> 2;

    //
    // Write curve parameter 'p' as 1st part of vector B.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) =
            (uint32_t)ptCurve->pui32Prime[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Write curve parameter 'a' in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32A[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // write curve parameter 'b' in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32B[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Update the C ptr with the offset address of the PKA RAM location
    // where the Gx, Gy will be stored.
    //
    HWREG(PKA_CPTR) = offset >> 2;

    //
    // Write x co-ordinate value of the Generator point in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32Gx[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Write y co-ordinate value of the Generator point in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32Gy[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Update the result location.
    //
    *pui32ResultVector =  PKA_RAM_BASE + offset;

    //
    // Load D ptr with the result location in PKA RAM.
    //
    HWREG(PKA_DPTR) = offset >> 2;

    //
    // Load length registers.
    //
    HWREG(PKA_ALENGTH) = ptCurve->ui8Size;
    HWREG(PKA_BLENGTH) = ptCurve->ui8Size;

    //
    // Set the PKA function to ECC-MULT and start the operation.
    //
    HWREG( (PKA_FUNCTION) ) = 0x0000D000;

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of ECC Multiplication with Generator point.
//!
//! \param ptOutEcPt is the pointer to the structure where the resultant EC
//! point will be stored.  The callee is responsible to allocate the space for
//! the ec point structure and the x and y co-ordinate as well.
//! \param ui32ResVectorLoc is the address of the result location which
//! was provided by the start function \sa PKAECCMultGenPtStart().
//!
//! This function gets the result of ecc point multiplication operation on the
//! scalar point and the known generator point on the curve, previously started
//! using the function \sa PKAECCMultGenPtStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if the operation is successful. 
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy performing 
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_FAILURE if the operation is not successful.
//
//*****************************************************************************
tPKAStatus
PKAECCMultGenPtGetResult(tECPt* ptOutEcPt, uint32_t ui32ResVectorLoc)
{
    int i;
    uint32_t regMSWVal;
    uint32_t addr;
    uint32_t len;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != ptOutEcPt);
    ASSERT(NULL != ptOutEcPt->pui32X);
    ASSERT(NULL != ptOutEcPt->pui32Y);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    //
    // Verify that the operation is completed.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    if(HWREG(PKA_SHIFT) == 0x00000000)
    {
        //
        // Get the MSW register value.
        //
        regMSWVal = HWREG(PKA_MSW);

        //
        // Check to make sure that the result vector is not all zeroes.
        //
        if(regMSWVal & PKA_MSW_RESULT_IS_ZERO)
        {
            return (PKA_STATUS_RESULT_0);
        }

        //
        // Get the length of the result.
        //
        len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1) -
              ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

        addr = ui32ResVectorLoc;

        //
        // Copy the x co-ordinate value of the result from vector D into the
        // EC point.
        //
        for(i = 0; i < len; i++)
        {
            ptOutEcPt->pui32X[i] = HWREG( (addr + 4*i) );
        }

        addr += 4 * (i + 2 + len % 2);

        //
        // Copy the y co-ordinate value of the result from vector D into the
        // EC point.
        //
        for(i = 0; i < len; i++)
        {
            ptOutEcPt->pui32Y[i] = HWREG( (addr + 4*i) );
        }

        return (PKA_STATUS_SUCCESS);
    }
    else
    {
        return (PKA_STATUS_FAILURE);
    }
}

//*****************************************************************************
//
//! Starts the ECC Addition.
//!
//! \param ptEcPt1 is the pointer to the structure containing the first
//! ecc point.
//! \param ptEcPt2 is the pointer to the structure containing the
//! second ecc point.
//! \param ptCurve is the pointer to the structure containing the curve
//! info.
//! \param pui32ResultVector is the pointer to the result vector location
//! which will be set by this function.
//!
//! This function starts the ecc point addition operation on the
//! two given ec points and generates the resultant ecc point.
//!
//!\return Returns: 
//! - \b PKA_STATUS_SUCCESS if successful in starting the operation.  
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy doing
//! some other operation.
//
//*****************************************************************************
tPKAStatus
PKAECCAddStart(tECPt* ptEcPt1, tECPt* ptEcPt2,tECCCurveInfo* ptCurve,
               uint32_t* pui32ResultVector)
{
    uint8_t extraBuf;
    uint32_t offset;
    int i;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != ptEcPt1);
    ASSERT(NULL != ptEcPt1->pui32X);
    ASSERT(NULL != ptEcPt1->pui32Y);
    ASSERT(NULL != ptEcPt2);
    ASSERT(NULL != ptEcPt2->pui32X);
    ASSERT(NULL != ptEcPt2->pui32Y);
    ASSERT(NULL != ptCurve);
    ASSERT(NULL != pui32ResultVector);

    offset = 0;

    //
    // Make sure no operation is in progress.
    //
    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    //
    // Calculate the extra buffer requirement.
    //
    extraBuf = 2 + ptCurve->ui8Size % 2;

    //
    // Update the A ptr with the offset address of the PKA RAM location
    // where the first ecPt will be stored.
    //
    HWREG(PKA_APTR) = offset >> 2;

    //
    // Load the x co-ordinate value of the first EC point in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = ptEcPt1->pui32X[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Load the y co-ordinate value of the first EC point in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = ptEcPt1->pui32Y[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Update the B ptr with the offset address of the PKA RAM location
    // where the curve parameters will be stored.
    //
    HWREG(PKA_BPTR) = offset >> 2;

    //
    // Write curve parameter 'p' as 1st part of vector B
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) =
            (uint32_t)ptCurve->pui32Prime[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Write curve parameter 'a'.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = (uint32_t)ptCurve->pui32A[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Update the C ptr with the offset address of the PKA RAM location
    // where the ecPt2 will be stored.
    //
    HWREG(PKA_CPTR) = offset >> 2;

    //
    // Load the x co-ordinate value of the second EC point in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = ptEcPt2->pui32X[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Load the y co-ordinate value of the second EC point in PKA RAM.
    //
    for(i = 0; i < ptCurve->ui8Size; i++)
    {
        HWREG((PKA_RAM_BASE + offset + 4*i)) = ptEcPt2->pui32Y[i];
    }

    //
    // Determine the offset in PKA RAM for the next data.
    //
    offset += 4 * (i + extraBuf);

    //
    // Copy the result vector location.
    //
    *pui32ResultVector = PKA_RAM_BASE + offset;

    //
    // Load D ptr with the result location in PKA RAM.
    //
    HWREG(PKA_DPTR) = offset >> 2;

    //
    // Load length registers.
    //
    HWREG(PKA_BLENGTH) = ptCurve->ui8Size;

    //
    // Set the PKA Function to ECC-ADD and start the operation.
    //
    HWREG( (PKA_FUNCTION) ) = 0x0000B000;

    return (PKA_STATUS_SUCCESS);
}

//*****************************************************************************
//
//! Gets the result of the ECC Addition
//!
//! \param ptOutEcPt is the pointer to the structure where the resultant
//!        point will be stored. The callee is responsible to allocate memory,
//!        for the ec point structure including the memory for x and y
//!        co-ordinate values.
//! \param ui32ResVectorLoc is the address of the result location which
//!        was provided by the function \sa PKAECCAddStart().
//!
//! This function gets the result of ecc point addition operation on the
//! on the two given ec points, previously started using the function \sa
//! PKAECCAddStart().
//!
//! \return Returns:
//! - \b PKA_STATUS_SUCCESS if the operation is successful. 
//! - \b PKA_STATUS_OPERATION_INPRG, if the PKA hw module is busy performing 
//! the operation.
//! - \b PKA_STATUS_RESULT_0 if the result is all zeroes.
//! - \b PKA_STATUS_FAILURE if the operation is not successful.
//
//*****************************************************************************
tPKAStatus
PKAECCAddGetResult(tECPt* ptOutEcPt, uint32_t ui32ResVectorLoc)
{
    uint32_t regMSWVal;
    uint32_t addr;
    int i;
    uint32_t len;

    //
    // Check for the arguments.
    //
    ASSERT(NULL != ptOutEcPt);
    ASSERT(NULL != ptOutEcPt->pui32X);
    ASSERT(NULL != ptOutEcPt->pui32Y);
    ASSERT((ui32ResVectorLoc > PKA_RAM_BASE) &&
           (ui32ResVectorLoc < (PKA_RAM_BASE + PKA_RAM_SIZE)));

    if((HWREG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0)
    {
        return (PKA_STATUS_OPERATION_INPRG);
    }

    if(HWREG(PKA_SHIFT) == 0x00000000)
    {
        //
        // Get the MSW register value.
        //
        regMSWVal = HWREG(PKA_MSW);

        //
        // Check to make sure that the result vector is not all zeroes.
        //
        if(regMSWVal & PKA_MSW_RESULT_IS_ZERO)
        {
            return (PKA_STATUS_RESULT_0);
        }

        //
        // Get the length of the result.
        //
        len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1) -
              ((ui32ResVectorLoc - PKA_RAM_BASE) >> 2);

        addr = ui32ResVectorLoc;

        //
        // Copy the x co-ordinate value of result from vector D into the
        // the output EC Point.
        //
        for(i = 0; i < len; i++)
        {
            ptOutEcPt->pui32X[i] = HWREG((addr + 4*i));
        }

        addr += 4 * (i + 2 + len % 2);

        //
        // Copy the y co-ordinate value of result from vector D into the
        // the output EC Point.
        //
        for(i = 0; i < len; i++)
        {
            ptOutEcPt->pui32Y[i] = HWREG((addr + 4*i));
        }

        return (PKA_STATUS_SUCCESS);
    }
    else
    {
        return (PKA_STATUS_FAILURE);
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

