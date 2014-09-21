/******************************************************************************
*  Filename:       ccm.c
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

//*****************************************************************************
//
//! \addtogroup ccm_api
//! @{
//
//*****************************************************************************

#include "aes.h"
#include "ccm.h"


//*****************************************************************************
//
//! CCMAuthEncryptStart starts the CCM operation
//!
//! \param bEncrypt if set to 'true' then run encryption and set to 'false' for
//! authentication only.
//! \param ui8Mval is the length of authentication field in octets [0,2,4,6,8,10,
//! 12, 14 or 16].
//! \param  pui8N is the pointer to 13-byte or 12-byte Nonce.
//! \param  pui8M  is the pointer to octet string 'm'/input message.
//! \param  ui16LenM is the length of pui8M[] in octets.
//! \param  pui8A is the pointer to octet string 'a'.
//! \param  ui16LenA is the Length of pui8A[] in octets.
//! \param  ui8KeyLocation is the location where the Key is stored in Key RAM.
//! \param  pui8Cstate is the pointer to output buffer.
//! \param  ui8CCMLVal is the ccm L Value to be used.
//! \param  ui8IntEnable if set to 'true' to enable interrupts or 'false' to
//! disable interrupts.  Should be 'false' if \e bEncrypt is set to 'false'.
//!
//!
//! The function will place in \e pui8Cstate the first ui8Mval bytes
//! containing the Authentication Tag.
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
uint8_t CCMAuthEncryptStart(bool bEncrypt,
                            uint8_t ui8Mval,
                            uint8_t *pui8N,
                            uint8_t *pui8M,
                            uint16_t ui16LenM,
                            uint8_t *pui8A,
                            uint16_t ui16LenA,
                            uint8_t ui8KeyLocation,
                            uint8_t *pui8Cstate,
                            uint8_t ui8CCMLVal,
                            uint8_t ui8IntEnable)
{
    uint8_t  ui8A0[16];
    uint32_t ui32CtrlVal;
    uint8_t  ui8I;
    g_ui8CurrentAESOp = AES_CCM;

    IntDisable(INT_AES);

    // workaround for AES registers not retained after PM2
    HWREG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
    HWREG(AES_CTRL_INT_EN) = (AES_CTRL_INT_EN_RESULT_AV |
                              AES_CTRL_INT_EN_DMA_IN_DONE);

    HWREG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_AES;
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    HWREG(AES_KEY_STORE_READ_AREA) = (uint32_t)ui8KeyLocation;

    //wait until key is loaded to the AES module
    do
    {
        ASM_NOP;
    }
    while((HWREG(AES_KEY_STORE_READ_AREA) & AES_KEY_STORE_READ_AREA_BUSY));

    //check for Key Store read error
    if((HWREG(AES_CTRL_INT_STAT)& AES_CTRL_INT_STAT_KEY_ST_RD_ERR))
    {
        // clear the Keystore Read error bit
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_KEY_ST_RD_ERR;
        return (AES_KEYSTORE_READ_ERROR);
    }

    // Prepare the initialization vector
    ui8A0[0] = ui8CCMLVal - 1;  // Lval

    for(ui8I = 0; ui8I < 13; ui8I++)
    {
        ui8A0[ui8I + 1] = pui8N[ui8I];
    }
    if(3 == ui8CCMLVal)
    {
        ui8A0[13] = 0;
    }
    ui8A0[14] = 0;  // initialize counter to 0
    ui8A0[15] = 0;  // initialize counter to 0

    // write initialization vector
    HWREG(AES_AES_IV_0) = ((uint32_t  *)&ui8A0)[0];
    HWREG(AES_AES_IV_1) = ((uint32_t  *)&ui8A0)[1];
    HWREG(AES_AES_IV_2) = ((uint32_t  *)&ui8A0)[2];
    HWREG(AES_AES_IV_3) = ((uint32_t  *)&ui8A0)[3];

    // configure AES engine
    ui32CtrlVal = ((ui8CCMLVal - 1) <<
                   AES_AES_CTRL_CCM_L_S);            // CCM_L

    if(ui8Mval >= 2)
    {
        ui32CtrlVal |= (((ui8Mval - 2) >> 1) <<
                        AES_AES_CTRL_CCM_M_S);           // CCM_M
    }
    else
    {
        ui32CtrlVal |= (0 <<
                        AES_AES_CTRL_CCM_M_S);           // CCM_M
    }
    ui32CtrlVal |= (AES_AES_CTRL_CCM);               // CCM
    ui32CtrlVal |= (1 << AES_AES_CTRL_key_size_S);   // key = 128
    ui32CtrlVal |= (1 << AES_AES_CTRL_input_ready);  // encryption
    ui32CtrlVal |= AES_AES_CTRL_CTR;                 // CTR
    ui32CtrlVal |= AES_AES_CTRL_save_context;        // save context
    ui32CtrlVal |= (0x3 << AES_AES_CTRL_ctr_width_S);// CTR width 128
    // program AES-CCM-128 encryption
    HWREG(AES_AES_CTRL) = ui32CtrlVal;

    // write the length of the crypto block (lo)
    HWREG(AES_AES_C_LENGTH_0) = (uint16_t)(ui16LenM) ;
    // write the length of the crypto block (hi)
    HWREG(AES_AES_C_LENGTH_1)  =  0;

    // write the length of the AAD data block may be non-block size aligned
    HWREG(AES_AES_AUTH_LENGTH) = ui16LenA;

    if(ui16LenA != 0)
    {
        // configure DMAC to fetch the AAD data
        // enable DMA channel 0
        HWREG(AES_DMAC_CH0_CTRL)     = AES_DMAC_CH0_CTRL_EN;
        // base address of the AAD input data in ext. memory
        HWREG(AES_DMAC_CH0_EXTADDR)  = (uint32_t)pui8A;
        // AAD data length in bytes, equal to the AAD length len
        //({aad data}) (may be non-block size aligned)

        HWREG(AES_DMAC_CH0_DMALENGTH) = ui16LenA;

        // wait for completion of the AAD data transfer, DMA_IN_DONE
        do
        {
            ASM_NOP;
        }
        while(!(HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_IN_DONE));

        // check for the absence of error
        if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
        {
            //clear the DMA error
            HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_DMA_BUS_ERR;
            return (AES_DMA_BUS_ERROR);
        }
    }

    // clear interrupt status
    HWREG(AES_CTRL_INT_CLR) = (AES_CTRL_INT_CLR_DMA_IN_DONE |
                               AES_CTRL_INT_CLR_RESULT_AV);
    if(ui8IntEnable)
    {
        IntPendClear(INT_AES);
        IntEnable(INT_AES);
    }

    // enable result available bit in interrupt enable
    HWREG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_RESULT_AV;

    if(bEncrypt)
    {
        // configure DMAC
        // enable DMA channel 0
        HWREG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH0_CTRL_EN;
        // base address of the payload data in ext. memory
        HWREG(AES_DMAC_CH0_EXTADDR) = (uint32_t)pui8M;
        // payload data length in bytes, equal to the message length
        //len({crypto_data})
        HWREG(AES_DMAC_CH0_DMALENGTH) = (ui16LenM);

        // enable DMA channel 1
        HWREG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH1_CTRL_EN;
        // base address of the output data buffer
        HWREG(AES_DMAC_CH1_EXTADDR) = (uint32_t)pui8M;
        // output data length in bytes, equal to the result data length
        // len({crypto data})
        HWREG(AES_DMAC_CH1_DMALENGTH) = ui16LenM;
    }
    return (AES_SUCCESS);

}

//*****************************************************************************
//
//! CCMAuthEncryptCheckResult checks the status of CCM encrypt operation.
//!
//! \return  if result is available or error occurs, function returns true.  
//! If result is not yet available or no error occurs, returns false
//!
//
//*****************************************************************************
uint8_t CCMAuthEncryptCheckResult(void)
{
    return (((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV)) ||
            ((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR)) ||
            ((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_WR_ERR)) ||
            ((HWREGB(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_KEY_ST_RD_ERR)));
}

//*****************************************************************************
//
//! CCMAuthEncryptGetResult gets the result of CCM operation.  This
//! function should be called after CCCMAuthEncryptStart is called.
//!
//! \param ui8Mval is length of authentication field in octets [0,2,4,6,8,10,12,
//!  14 or 16].
//! \param ui16LenM  is length of message pui8M[] in octets.
//! \param pui8Cstate is pointer to AES state buffer.
//!
//! \return  AES_SUCCESS if successful.
//
//*****************************************************************************
uint8_t CCMAuthEncryptGetResult(uint8_t ui8Mval,
                                uint16_t ui16LenM,
                                uint8_t *pui8Cstate)

{
    uint8_t volatile ui8MIC[16];
    uint8_t ui8I;

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

    IntDisable(INT_AES);

    // disable the master control/DMA clock
    HWREG(AES_CTRL_ALG_SEL) = 0x00000000;

    // read tag
    // wait for the context ready bit [30]
    do
    {
        ASM_NOP;
    }
    while((HWREG(AES_AES_CTRL) & AES_AES_CTRL_saved_context_ready) !=
            AES_AES_CTRL_saved_context_ready);

    // Read the tag registers
    ((uint32_t  *)&ui8MIC)[0] = HWREG(AES_AES_TAG_OUT_0);
    ((uint32_t  *)&ui8MIC)[1] = HWREG(AES_AES_TAG_OUT_1);
    ((uint32_t  *)&ui8MIC)[2] = HWREG(AES_AES_TAG_OUT_2);
    ((uint32_t  *)&ui8MIC)[3] = HWREG(AES_AES_TAG_OUT_3);

    // clear the interrupt status
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    // copy tag to pui8Cstate
    for(ui8I = 0; ui8I < ui8Mval; ui8I++)
    {
        pui8Cstate[ui8I] = ui8MIC[ui8I];
    }
    g_ui8CurrentAESOp = AES_NONE;
    return (AES_SUCCESS);
}

//*****************************************************************************
//
//! CCMInvAuthDecryptStart starts the CCM Decryption and Inverse
//! Authentication operation.
//!
//! \param bDecrypt if set to 'true' then run decryption, set to 'false' if
//! authentication only
//! \param ui8Mval is the length of authentication field in octets [0,2,4,6,8,
//! 10,12,14 or 16].
//! \param  pui8N is the pointer to 13-byte or 12-byte Nonce.
//! \param  pui8C  is the pointer to octet string 'c' = 'm' || auth tag T.
//! \param  ui16LenC is the length of pui8C[] in octets.
//! \param  pui8A is the pointer to octet string 'a'.
//! \param  ui16LenA is the Length of pui8A[] in octets.
//! \param  ui8KeyLocation is the location where the Key is stored in Key RAM.
//! \param  pui8Cstate is the pointer to output buffer.  (cannot be part
//! of pui8C[]).
//! \param  ui8CCMLVal is the ccm L Value to be used.
//! \param  ui8IntEnable if set to 'true' to enable interrupts or 'false' to
//! disable interuupts.  Set to 'false' if \e bDecrypt is set to 'false'.
//!
//!
//! The function will place in \e pui8Cstate the first ui8Mval bytes of 
//! containing the Authentication Tag.
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
//! \return  AES_SUCCESS if successful.
//
//*****************************************************************************
uint8_t CCMInvAuthDecryptStart(bool bDecrypt,
                               uint8_t ui8Mval,
                               uint8_t *pui8N,
                               uint8_t *pui8C,
                               uint16_t ui16LenC,
                               uint8_t *pui8A,
                               uint16_t ui16LenA,
                               uint8_t ui8KeyLocation,
                               uint8_t *pui8Cstate,
                               uint8_t ui8CCMLVal,
                               uint8_t ui8IntEnable)
{
    uint16_t ui16LenM = ui16LenC - ui8Mval;
    uint8_t  ui8A0[16];
    uint32_t ui32CtrlVal;
    uint8_t  ui8I;
    g_ui8CurrentAESOp = AES_CCM;

    // workaround for AES registers not retained after PM2
    HWREG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
    HWREG(AES_CTRL_INT_EN) = (AES_CTRL_INT_EN_RESULT_AV |
                              AES_CTRL_INT_EN_DMA_IN_DONE);

    HWREG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_AES;
    HWREG(AES_CTRL_INT_CLR) = (AES_CTRL_INT_CLR_DMA_IN_DONE |
                               AES_CTRL_INT_CLR_RESULT_AV);

    HWREG(AES_KEY_STORE_READ_AREA) = (uint32_t)ui8KeyLocation;

    //wait until key is loaded to the AES module
    do
    {
        ASM_NOP;
    }
    while((HWREG(AES_KEY_STORE_READ_AREA) & AES_KEY_STORE_READ_AREA_BUSY));

    //check for Key Store read error
    if((HWREG(AES_CTRL_INT_STAT)& AES_CTRL_INT_STAT_KEY_ST_RD_ERR))
    {
        // clear the Keystore Read error bit
        HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_KEY_ST_RD_ERR;
        return (AES_KEYSTORE_READ_ERROR);
    }

    // Prepare the initialization vector
    ui8A0[0] = ui8CCMLVal - 1;          // Lval
    for(ui8I = 0; ui8I < 13; ui8I++)
    {
        ui8A0[ui8I + 1] = pui8N[ui8I];
    }
    if(3 == ui8CCMLVal)
    {
        ui8A0[13] = 0;
    }
    ui8A0[14] = 0;                   // initialize counter to 0
    ui8A0[15] = 0;                   // initialize counter to 0

    // write initialization vector
    HWREG(AES_AES_IV_0) = ((uint32_t  *)&ui8A0)[0];
    HWREG(AES_AES_IV_1) = ((uint32_t  *)&ui8A0)[1];
    HWREG(AES_AES_IV_2) = ((uint32_t  *)&ui8A0)[2];
    HWREG(AES_AES_IV_3) = ((uint32_t  *)&ui8A0)[3];

    // configure AES engine
    ui32CtrlVal = ((ui8CCMLVal - 1) <<
                   AES_AES_CTRL_CCM_L_S);             // CCM_L
    if(ui8Mval >= 2)
    {
        ui32CtrlVal |= (((ui8Mval - 2) >> 1) <<
                        AES_AES_CTRL_CCM_M_S);           // CCM_M
    }
    else
    {
        ui32CtrlVal |= (0 <<
                        AES_AES_CTRL_CCM_M_S);           // CCM_M
    }
    ui32CtrlVal |= (AES_AES_CTRL_CCM);                // CCM
    ui32CtrlVal |= (1 << AES_AES_CTRL_key_size_S);    // key = 128
    ui32CtrlVal |= (0 << AES_AES_CTRL_input_ready);   // decryption
    ui32CtrlVal |= AES_AES_CTRL_CTR;                  // CTR
    ui32CtrlVal |= AES_AES_CTRL_save_context;         // save context
    ui32CtrlVal |= (0x3 << AES_AES_CTRL_ctr_width_S); // CTR width 128
    // program AES-CCM-128 encryption
    HWREG(AES_AES_CTRL) = ui32CtrlVal;

    // write the length of the crypto block (lo)
    HWREG(AES_AES_C_LENGTH_0) = (uint16_t)(ui16LenM) ;
    // write the length of the crypto block (hi)
    HWREG(AES_AES_C_LENGTH_1)  =  0;

    // write the length of the AAD data block may be non-block size aligned
    HWREG(AES_AES_AUTH_LENGTH) = ui16LenA;

    if(ui16LenA != 0)
    {
        // configure DMAC to fetch the AAD data
        // enable DMA channel 0
        HWREG(AES_DMAC_CH0_CTRL)      =  AES_DMAC_CH0_CTRL_EN;
        // base address of the AAD input data in ext. memory
        HWREG(AES_DMAC_CH0_EXTADDR)   = (uint32_t)pui8A;
        // AAD data length in bytes, equal to the AAD length len
        //({aad data}) (may be non-block size aligned)

        HWREG(AES_DMAC_CH0_DMALENGTH) = ui16LenA;

        // wait for completion of the AAD data transfer, DMA_IN_DONE
        do
        {
            ASM_NOP;
        }
        while(!(HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_IN_DONE));

        // check for the absence of error
        if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
        {
            HWREG(AES_CTRL_INT_CLR) |= AES_CTRL_INT_CLR_DMA_BUS_ERR;
            return (AES_DMA_BUS_ERROR);
        }
    }

    // clear interrupt status
    HWREG(AES_CTRL_INT_CLR) = (AES_CTRL_INT_CLR_DMA_IN_DONE |
                               AES_CTRL_INT_CLR_RESULT_AV);

    if(ui8IntEnable)
    {
        IntPendClear(INT_AES);
        IntEnable(INT_AES);
    }

    // enable result available bit in interrupt enable
    HWREG(AES_CTRL_INT_EN) = AES_CTRL_INT_EN_RESULT_AV;

    if(bDecrypt)
    {
        // configure DMAC
        // enable DMA channel 0
        HWREG(AES_DMAC_CH0_CTRL) = AES_DMAC_CH0_CTRL_EN;
        // base address of the payload data in ext. memory
        HWREG(AES_DMAC_CH0_EXTADDR) = (uint32_t)pui8C;
        // payload data length in bytes, equal to the message length len({crypto_data})
        HWREG(AES_DMAC_CH0_DMALENGTH) = (ui16LenM);

        // enable DMA channel 1
        HWREG(AES_DMAC_CH1_CTRL) = AES_DMAC_CH1_CTRL_EN;
        // base address of the output data buffer
        HWREG(AES_DMAC_CH1_EXTADDR) = (uint32_t)pui8C;
        // output data length in bytes, equal to the result data length len({crypto data})
        HWREG(AES_DMAC_CH1_DMALENGTH) = ui16LenM;
    }

    return (AES_SUCCESS);
}

//*****************************************************************************
//
//! CCMInvAuthDecryptCheckResult function checks CCM decrypt and Inverse
//! Authentication result.
//!
//! \return  if result is available or error occurs returns true.  If result
//! is not yet available or no error occurs returns false
//
//*****************************************************************************
uint8_t CCMInvAuthDecryptCheckResult(void)
{
    // check if result is available (or) some error has occured
    return (CCMAuthEncryptCheckResult());
}

//*****************************************************************************
//
//! CCMInvAuthDecryptGetResult gets the result of CCM operation. This
//! function should be called only after CCMInvAuthDecryptStart is called.
//!
//! \param ui8Mval is length of authentication field in octets [0,2,4,6,8,10,
//! 12,14 or 16].
//! \param pui8C is pointer to octet string 'c' = 'm' || auth tag T.
//! \param ui16LenC  is length of message pui8C[] in octets.
//! \param pui8Cstate is pointer to AES state buffer, cannot be part of
//!  pui8C[]).
//!
//! \return  AES_SUCCESS if successful.
//
//*****************************************************************************
uint8_t CCMInvAuthDecryptGetResult(uint8_t ui8Mval,
                                   uint8_t *pui8C,
                                   uint16_t ui16LenC,
                                   uint8_t *pui8Cstate)
{
    uint8_t volatile ui8MIC[16];
    uint16_t ui16LenM = ui16LenC - ui8Mval;
    uint8_t ui8I, ui8J;

    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
    {
        //clear the DMA error
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

    IntDisable(INT_AES);

    // disable the master control/DMA clock
    HWREG(AES_CTRL_ALG_SEL) = 0x00000000;

    // read tag
    // wait for the context ready bit [30]
    do
    {
        ASM_NOP;
    }
    while((HWREG(AES_AES_CTRL) & AES_AES_CTRL_saved_context_ready) !=
            AES_AES_CTRL_saved_context_ready);

    // Read the tag registers
    ((uint32_t  *)&ui8MIC)[0] = HWREG(AES_AES_TAG_OUT_0);
    ((uint32_t  *)&ui8MIC)[1] = HWREG(AES_AES_TAG_OUT_1);
    ((uint32_t  *)&ui8MIC)[2] = HWREG(AES_AES_TAG_OUT_2);
    ((uint32_t  *)&ui8MIC)[3] = HWREG(AES_AES_TAG_OUT_3);

    // clear the interrupt status
    HWREG(AES_CTRL_INT_CLR) |= (AES_CTRL_INT_CLR_DMA_IN_DONE |
                                AES_CTRL_INT_CLR_RESULT_AV);

    // copy tag to pui8Cstate
    for(ui8I = 0; ui8I < ui8Mval; ui8I++)
    {
        pui8Cstate[ui8I] = ui8MIC[ui8I];
    }

    for(ui8J = 0; ui8J < ui8Mval; ui8J++)
    {
        if(pui8Cstate[ui8J] != pui8C[ui16LenM + ui8J])
        {
            return (CCM_AUTHENTICATION_FAILED);
        }
    }

    g_ui8CurrentAESOp = AES_NONE;
    return (AES_SUCCESS);
}

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************

