/******************************************************************************
*  Filename:       aes.c
*  Revised:        $Date: 2013-03-22 16:13:31 +0100 (Fri, 22 Mar 2013) $
*  Revision:       $Revision: 9513 $
*
*  Description:    Support for Hardware AES encryption.
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
//! \addtogroup aes_api
//! @{
//
//*****************************************************************************

#include "aes.h"

//*****************************************************************************
//
// Length of AES ECB block in bytes
//
//*****************************************************************************
#define AES_ECB_LENGTH  16

//*****************************************************************************
//
// Current AES operation initialized to None
//
//*****************************************************************************
volatile uint8_t g_ui8CurrentAESOp = AES_NONE;


//*****************************************************************************
//
//! AESLoadKey writes the key into the Key Ram. Key Ram location must be
//! specified.
//!
//! \param   pui8Key is pointer to AES Key.
//! \param   ui8KeyLocation is location in Key RAM.
//!
//! The \e ui8KeyLocation parameter is an enumerated type which specifies
//! the Key Ram locationin which the key is stored.
//! This parameter can have any of the following values:
//!
//! - \b KEY_AREA_0
//! - \b KEY_AREA_1
//! - \b KEY_AREA_2,
//! - \b KEY_AREA_3,
//! - \b KEY_AREA_4,
//! - \b KEY_AREA_5,
//! - \b KEY_AREA_6,
//! - \b KEY_AREA_7
//!
//! The pointer \e pui8Key has the address where the Key is stored.
//!
//! \return  AES_SUCCESS if successful.
//
//*****************************************************************************
uint8_t AESLoadKey(uint8_t *pui8Key , uint8_t ui8KeyLocation)
{
    static uint32_t ui32temp[4];
    uint8_t * pui8temp = (uint8_t *)ui32temp;
    uint8_t i;

    g_ui8CurrentAESOp = AES_KEYL0AD;
    // The key address needs  to be 4 byte aligned
    for(i = 0; i < KEY_BLENGTH; i++)
    {
        pui8temp[i] = pui8Key[i];
    }
    IntDisable(INT_AES);

    // workaround for AES registers not retained after PM2
    HWREG(AES_CTRL_INT_CFG) |= AES_CTRL_INT_CFG_LEVEL;
    HWREG(AES_CTRL_INT_EN) |= (AES_CTRL_INT_EN_DMA_IN_DONE |
                               AES_CTRL_INT_EN_RESULT_AV);

    // configure master control module
    HWREG(AES_CTRL_ALG_SEL) &= (~AES_CTRL_ALG_SEL_KEYSTORE);
    HWREG(AES_CTRL_ALG_SEL) |= AES_CTRL_ALG_SEL_KEYSTORE;

    // clear any outstanding events
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    //configure key store module (area, size)
    HWREG(AES_KEY_STORE_SIZE) &= KEY_STORE_SIZE_BITS;

    // 128-bit key size
    HWREG(AES_KEY_STORE_SIZE) |= KEY_STORE_SIZE_128;

    // enable keys to write (e.g. Key 0)
    HWREG(AES_KEY_STORE_WRITE_AREA) = (0x00000001 << ui8KeyLocation);


    // configure DMAC
    // enable DMA channel 0
    HWREG(AES_DMAC_CH0_CTRL) |= 0x000000001;

    // base address of the key in ext. memory
    HWREG(AES_DMAC_CH0_EXTADDR) = (uint32_t)pui8temp;

    // total key length in bytes (e.g. 16 for 1 x 128-bit key)
    HWREG(AES_DMAC_CH0_DMALENGTH) = 0x10;

    // wait for operation completed
    do
    {
        ASM_NOP;
    }
    while((!(HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV)));

    // check for absence of errors in DMA and key store
    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
    {
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_DMA_BUS_ERR;
        return (AES_DMA_BUS_ERROR);
    }
    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_WR_ERR))
    {
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_KEY_ST_WR_ERR;
        return (AES_KEYSTORE_WRITE_ERROR);
    }

    // acknowledge the interrupt
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    // disable master control/DMA clock
    HWREG(AES_CTRL_ALG_SEL) = 0x00000000;

    // check status, if error return error code
    if((HWREG(AES_KEY_STORE_WRITTEN_AREA) & 0x7) != 0x1)
    {
        g_ui8CurrentAESOp = AES_NONE;
        return (AES_KEYSTORE_WRITE_ERROR);
    }

    for(i = 0; i < KEY_BLENGTH; i++)
    {
        pui8temp[i] = 0;
    }

    g_ui8CurrentAESOp = AES_NONE;
    return (AES_SUCCESS);
}

//*****************************************************************************
//
//! AESECBStart starts an AES-ECB operation.
//!
//! \param pui8MsgIn is pointer to input data.
//! \param pui8MsgOut is pointer to output data.
//! \param ui8KeyLocation is the location in Key RAM.
//! \param ui8Encrypt is set 'true' to ui8Encrypt or set 'false' to decrypt.
//! \param ui8IntEnable is set 'true' to enable AES interrupts or 'false' to
//! disable AES interrupt.
//!
//! The \e ui8KeyLocation parameter is an enumerated type which specifies
//! the Key Ram location in which the key is stored.
//! This parameter can have any of the following values:
//!
//! - \b KEY_AREA_0
//! - \b KEY_AREA_1
//! - \b KEY_AREA_2,
//! - \b KEY_AREA_3,
//! - \b KEY_AREA_4,
//! - \b KEY_AREA_5,
//! - \b KEY_AREA_6,
//! - \b KEY_AREA_7
//!
//! \return  AES_SUCCESS if successful.
//
//*****************************************************************************
uint8_t AESECBStart(uint8_t *pui8MsgIn,
                    uint8_t *pui8MsgOut,
                    uint8_t ui8KeyLocation,
                    uint8_t ui8Encrypt,
                    uint8_t ui8IntEnable)
{
    // workaround for AES registers not retained after PM2
    g_ui8CurrentAESOp = AES_ECB;
    HWREG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
    HWREG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_RESULT_AV;
    if(ui8IntEnable)
    {
        IntPendClear(INT_AES);
        IntEnable(INT_AES);
    }

    // configure the master control module
    // enable the DMA path to the AES engine
    HWREG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_AES;
    // clear any outstanding events
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    HWREG(AES_KEY_STORE_READ_AREA) = (uint32_t)ui8KeyLocation;

    //wait until key is loaded to the AES module
    do
    {
        ASM_NOP;
    }
    while((HWREG(AES_KEY_STORE_READ_AREA) & AES_KEY_STORE_READ_AREA_BUSY));

    // check for Key Store read error
    if((HWREG(AES_CTRL_INT_STAT)& AES_CTRL_INT_STAT_KEY_ST_RD_ERR))
    {
        // Clear Key Store Read error
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_KEY_ST_RD_ERR;
        return (AES_KEYSTORE_READ_ERROR);
    }

    // configure AES engine
    // program AES-ECB-128 encryption and no IV
    if(ui8Encrypt)
    {
        HWREG(AES_AES_CTRL) = 0x0000000C;
    }
    else
    {
        HWREG(AES_AES_CTRL) = 0x00000008;
    }

    // write length of the message (lo)
    HWREG(AES_AES_C_LENGTH_0) = (uint32_t) AES_ECB_LENGTH;
    // write length of the message (hi)
    HWREG(AES_AES_C_LENGTH_1) = 0;

    // configure DMAC
    // enable DMA channel 0
    HWREG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH0_CTRL_EN;

    // base address of the input data in ext. memory
    HWREG(AES_DMAC_CH0_EXTADDR) = (uint32_t)pui8MsgIn;

    // input data length in bytes, equal to the message
    HWREG(AES_DMAC_CH0_DMALENGTH) = AES_ECB_LENGTH;

    // length (may be non-block size aligned)
    HWREG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH1_CTRL_EN; // enable DMA channel 1

    // base address of the output data buffer
    HWREG(AES_DMAC_CH1_EXTADDR) = (uint32_t)pui8MsgOut;

    // output data length in bytes, equal to the result
    HWREG(AES_DMAC_CH1_DMALENGTH) = AES_ECB_LENGTH;

    return (AES_SUCCESS);
}

//*****************************************************************************
//
//! AESECBCheckResult is called to check the result of AES-ECB AESECBStart
//! operation.
//!
//! \return  if result is available or error occurs returns true.  If result
//! is not yet available or no error occurs returns false
//
//*****************************************************************************
uint8_t AESECBCheckResult(void)
{
    return (((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV)) ||
            ((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR)) ||
            ((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_WR_ERR)) ||
            ((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_RD_ERR)));
}

//*****************************************************************************
//
//! AESECBGetResult gets the result of the AES ECB operation.  This function
//! must only be called after AESECBStart function is called.
//!
//! \return  AES_SUCCESS if successful.
//
//*****************************************************************************
uint8_t AESECBGetResult(void)
{
    //check for errors
    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
    {
        // clear the DMA error bit
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_DMA_BUS_ERR;
        return (AES_DMA_BUS_ERROR);
    }
    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_WR_ERR))
    {
        // clear the Key Store Write error bit
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_KEY_ST_WR_ERR;
        return (AES_KEYSTORE_WRITE_ERROR);
    }
    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_RD_ERR))
    {
        // clear the Key Store Read error bit
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_KEY_ST_RD_ERR;
        return (AES_KEYSTORE_READ_ERROR);
    }

    // if no errors then AES ECB operation was successful, disable AES
    // interrupt
    IntDisable(INT_AES);

    //clear DMA done and result available bits
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    // result has already been copied to the output buffer by DMA
    HWREG(AES_CTRL_ALG_SEL) = 0x00000000; // disable master control/DMA clock
    HWREG(AES_AES_CTRL) = 0x00000000; // clear mode
    g_ui8CurrentAESOp = AES_NONE;
    return (AES_SUCCESS);
}

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************

