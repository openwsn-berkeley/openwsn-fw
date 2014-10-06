/******************************************************************************
*  Filename:       adc.c
*  Revised:        $Date: 2013-03-24 11:41:19 +0100 (Sun, 24 Mar 2013) $
*  Revision:       $Revision: 9521 $
*
*  Description:    Driver for the SOC ADC Module.
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

//*****************************************************************************
//
//! \addtogroup adc_api
//! @{
//
//*****************************************************************************

#include <headers/hw_ints.h>
#include <headers/hw_memmap.h>
#include <headers/hw_soc_adc.h>
#include "debug.h"
#include "interrupt.h"
#include "adc.h"

//*****************************************************************************
//
//! Registers an interrupt handler for ADC interrupt
//!
//! \param pfnHandler is a pointer to the function called when the
//! SOC ADC interrupt occurs.
//!
//! This function does the actual registering of the interrupt handler, which
//! enables the global interrupt in the interrupt controller.
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
SOCADCIntRegister(void (*pfnHandler)(void))
{
    //
    // Register the interrupt handler.
    //
    IntRegister(INT_ADC0, pfnHandler);

    //
    // Enable the ADC interrupt.
    //
    IntEnable(INT_ADC0);
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the ADC interrupt
//!
//! This function does the actual unregistering of the interrupt handler. This
//! function clears the handler to be called when an ADC interrupt occurs
//! and masks off the interrupt in the interrupt controller so that the
//! interrupt handler no longer is called.
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \return None
//
//*****************************************************************************
void
SOCADCIntUnregister(void)
{
    //
    // Disable the interrupt.
    //
    IntDisable(INT_ADC0);

    //
    // Unregister the interrupt handler.
    //
    IntUnregister(INT_ADC0);
}

//*****************************************************************************
//
//! Configure ADC conversion for a single channel
//!
//! \param ui32Resolution is the resolution of the conversion.
//! \param ui32Reference is the reference voltage to be used for the conversion.
//!
//! This function configures the ADC for a single channel conversion.
//! The \e ui32Resolution parameter must be one of:
//! \b SOCADC_7_BIT, \b SOCADC_9_BIT, \b SOCADC_10_BIT or \b SOCADC_12_BIT.
//! The reference voltage is set using the \e ui32Reference parameter, which
//! must be configured as one of the following:
//! \b SOCADC_REF_INTERNAL  for internal reference,
//! \b SOCADC_REF_EXT_AIN7  for external reference on pin AIN7 (pad PA7),
//! \b SOCADC_REF_AVDD5     for external AVDD5 pin,
//! \b SOCADC_REF_EXT_AIN67 for external reference on differential input pins
//!  AIN6-AIN7 (Pads PA6-PA7).
//!
//! \note A single conversion triggers an interrupt if this has been registered
//! using SOCADCIntRegister().
//!
//! \sa SOCADCSingleStart() and SOCADCIntRegister().
//!
//! \return None
//
//*****************************************************************************
void
SOCADCSingleConfigure(uint32_t ui32Resolution, uint32_t ui32Reference)
{
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT((ui32Resolution == SOCADC_7_BIT)  ||
           (ui32Resolution == SOCADC_9_BIT)  ||
           (ui32Resolution == SOCADC_10_BIT) ||
           (ui32Resolution == SOCADC_12_BIT));
    ASSERT((ui32Reference == SOCADC_REF_INTERNAL) ||
           (ui32Reference == SOCADC_REF_EXT_AIN7) ||
           (ui32Reference == SOCADC_REF_AVDD5)    ||
           (ui32Reference == SOCADC_REF_EXT_AIN67));

    //
    // Stop random generator
    //
    HWREG(SOC_ADC_ADCCON1) = 0x3c;

    ui32Reg = HWREG(SOC_ADC_ADCCON3) & ~(SOC_ADC_ADCCON3_EREF_M |
                                         SOC_ADC_ADCCON3_EDIV_M);
    HWREG(SOC_ADC_ADCCON3) = ui32Reg | ui32Resolution | ui32Reference;
}

//*****************************************************************************
//
//! Start a configured single conversion
//!
//! \param ui32Channel is the input channel to use for the conversion.
//!
//! This function initiates a configured single channel conversion.
//! The input channel is set using the \e ui32Channel parameter.
//! This parameter must be configured as one of the following values:
//! \b SOCADC_AIN0       for single ended input Pad PA0
//! \b SOCADC_AIN1       for single ended input Pad PA1
//! \b SOCADC_AIN2       for single ended input Pad PA2
//! \b SOCADC_AIN3       for single ended input Pad PA3
//! \b SOCADC_AIN4       for single ended input Pad PA4
//! \b SOCADC_AIN5       for single ended input Pad PA5
//! \b SOCADC_AIN6       for single ended input Pad PA6
//! \b SOCADC_AIN7       for single ended input Pad PA7
//! \b SOCADC_AIN01      for differential Pads PA0-PA1
//! \b SOCADC_AIN23      for differential Pads PA2-PA3
//! \b SOCADC_AIN45      for differential Pads PA4-PA5
//! \b SOCADC_AIN67      for differential Pads PA6-PA7
//! \b SOCADC_GND        for Ground as input
//! \b SOCADC_TEMP_SENS  for on-chip temperature sensor
//! \b SOCADC_VDD        for Vdd/3
//!
//! \note A single conversion triggers an interrupt if this has been registered
//! using SOCADCIntRegister().
//!
//! \sa SOCADCSingleConfigure() and SOCADCIntRegister().
//!
//! \return None
//
//*****************************************************************************
void
SOCADCSingleStart(uint32_t ui32Channel)
{
    uint32_t ui32Reg;

    //
    // Check the arguments.
    //
    ASSERT((ui32Channel == SOCADC_AIN0)     ||
           (ui32Channel == SOCADC_AIN1)      ||
           (ui32Channel == SOCADC_AIN2)      ||
           (ui32Channel == SOCADC_AIN3)      ||
           (ui32Channel == SOCADC_AIN4)      ||
           (ui32Channel == SOCADC_AIN5)      ||
           (ui32Channel == SOCADC_AIN6)      ||
           (ui32Channel == SOCADC_AIN7)      ||
           (ui32Channel == SOCADC_AIN01)     ||
           (ui32Channel == SOCADC_AIN23)     ||
           (ui32Channel == SOCADC_AIN45)     ||
           (ui32Channel == SOCADC_AIN67)     ||
           (ui32Channel == SOCADC_GND)       ||
           (ui32Channel == SOCADC_TEMP_SENS) ||
           (ui32Channel == SOCADC_VDD));

    //
    // Program selected channel, this indirectly starts the conversion
    //
    ui32Reg = HWREG(SOC_ADC_ADCCON3) & ~(SOC_ADC_ADCCON3_ECH_M);
    HWREG(SOC_ADC_ADCCON3) = ui32Reg | ui32Channel;
}

//*****************************************************************************
//
//! Get data value from conversion
//!
//! This function gets the latest conversion data result of the programmed
//! conversion. The function returns 16 bits of data, but depending on the
//! programmed precision, only part of the data is significant.
//! The following defined bit masks can be used to extract the significant data
//! depending on the decimation rate:
//!   \b SOCADC_7_BIT_MASK, \b SOCADC_9_BIT_MASK,
//!   \b SOCADC_10_BIT_MASK and \b SOCADC_12_BIT_MASK
//!
//! \sa SOCADCEndOfCOnversionGet().
//!
//! \return Data conversion value
//
//*****************************************************************************
uint16_t
SOCADCDataGet(void)
{
    uint32_t ui32Reg;

    ui32Reg = HWREG(SOC_ADC_ADCL) & SOC_ADC_ADCL_ADC_M;
    ui32Reg |= ((HWREG(SOC_ADC_ADCH) & SOC_ADC_ADCH_ADC_M) << 8);

    return ((uint16_t) ui32Reg);
}

//*****************************************************************************
//
//! Check if conversion is done
//!
//! This function can be used to query the status of the conversion.
//!
//! \return true if conversion is done, otherwise false.
//
//*****************************************************************************
bool
SOCADCEndOfCOnversionGet(void)
{
    return((HWREG(SOC_ADC_ADCCON1) & SOC_ADC_ADCCON1_EOC) ? true : false);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

