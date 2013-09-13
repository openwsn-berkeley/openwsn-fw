/******************************************************************************
*  Filename:       sha256.c
*  Revised:        $Date: 2013-04-03 14:12:40 +0200 (Wed, 03 Apr 2013) $
*  Revision:       $Revision: 9611 $
*
*  Description:    Support for Hardware SHA 256
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
//! \addtogroup sha256_api
//! @{
//
//*****************************************************************************

#include "aes.h"
#include "sha256.h"
#include <string.h>

static uint8_t SHA256HashResume(tSHA256State * psMd, uint8_t *ui8In, uint8_t *ui8Out);
static uint8_t SHA256HashNew(tSHA256State * psMd, uint8_t *ui8In, uint8_t *ui8Out);

//*****************************************************************************
//
//! SHA256init initializes the hash state.
//!
//! \param psMd is the pointer to hash state you wish to initialize.
//!
//! For the pointer to hash state parameter \e psMd the calling function has to
//! allocate the hash state structure and pass the pointer to the structure.
//!
//! \return  SHA256_SUCCESS if successful.
//
//*****************************************************************************
uint8_t SHA256Init(tSHA256State * psMd)
{
    if(psMd == NULL)
    {
        return (SHA256_NULL_ERROR);
    }

    psMd->curlen = 0;
    psMd->length = 0;
    psMd->new_digest = true;
    psMd->final_digest = false;
    return (SHA256_SUCCESS);
}

//*****************************************************************************
//
//! SHA256Process processes a block of memory through the hash. This
//! function must be called only after SHA256init().
//!
//! \param   psMd is the pointer to hash state.
//! \param   ui8In is the pointer to the data to hash.
//! \param   ui32InLen is the length of the data to hash ui8In bytes (octets).
//!
//! For the pointer to hash state parameter \e psMd the calling function must
//! allocate the hash state structure and pass the pointer to the structure.
//!
//! \return  SHA256_SUCCESS if successful.
//
//*****************************************************************************
uint8_t SHA256Process(tSHA256State * psMd, uint8_t *ui8In, uint32_t ui32InLen)
{
    uint8_t  ui8Err;
    uint32_t ui32N, ui32I;

    if(psMd == NULL)
    {
        return (SHA256_NULL_ERROR);
    }
    if(ui8In == NULL)
    {
        return (SHA256_NULL_ERROR);
    }

    if(psMd->curlen > sizeof(psMd->buf))
    {
        return (SHA256_INVALID_PARAM);
    }

    g_ui8CurrentAESOp = AES_SHA256;
    if(ui32InLen > 0 && psMd->new_digest == true)
    {
        if(psMd->curlen == 0 && ui32InLen > SHA256_BLOCK_SIZE)
        {
            for(ui32I = 0; ui32I < SHA256_BLOCK_SIZE; ui32I++)
            {
                psMd->buf[psMd->curlen + ui32I] = ui8In[ui32I];
            }
            if((ui8Err = SHA256HashNew(psMd, (uint8_t *)psMd->buf,
                                       (uint8_t *)psMd->state)) != SHA256_SUCCESS)
            {
                g_ui8CurrentAESOp = AES_NONE;
                return (ui8Err);
            }
            psMd->new_digest = false;
            psMd->length += SHA256_BLOCK_SIZE * 8;
            ui32InLen -= SHA256_BLOCK_SIZE;
            ui8In += SHA256_BLOCK_SIZE;

        }
        else
        {
            ui32N = MIN(ui32InLen, (SHA256_BLOCK_SIZE - psMd->curlen));
            for(ui32I = 0; ui32I < ui32N; ui32I++)
            {
                psMd->buf[psMd->curlen + ui32I] = ui8In[ui32I];
            }
            psMd->curlen += ui32N;
            ui8In          += ui32N;
            ui32InLen       -= ui32N;
            if(psMd->curlen == SHA256_BLOCK_SIZE && ui32InLen > 0)
            {
                if((ui8Err = SHA256HashNew(psMd, (uint8_t *)psMd->buf,
                                           (uint8_t *)psMd->state)) != SHA256_SUCCESS)
                {
                    g_ui8CurrentAESOp = AES_NONE;
                    return (ui8Err);
                }
                psMd->new_digest = false;
                psMd->length += 8 * SHA256_BLOCK_SIZE;
                psMd->curlen = 0;
            }
        }
    }

    while(ui32InLen > 0 && psMd->new_digest == false)
    {
        if(psMd->curlen == 0 && ui32InLen > SHA256_BLOCK_SIZE)
        {
            for(ui32I = 0; ui32I < SHA256_BLOCK_SIZE; ui32I++)
            {
                psMd->buf[psMd->curlen + ui32I] = ui8In[ui32I];
            }
            if((ui8Err = SHA256HashResume(psMd, (uint8_t *)psMd->buf,
                                          (uint8_t *)psMd->state)) != SHA256_SUCCESS)
            {
                g_ui8CurrentAESOp = AES_NONE;
                return (ui8Err);
            }
            psMd->length += SHA256_BLOCK_SIZE * 8;
            ui8In += SHA256_BLOCK_SIZE;
            ui32InLen -= SHA256_BLOCK_SIZE;
        }
        else
        {
            ui32N = MIN(ui32InLen, (SHA256_BLOCK_SIZE - psMd->curlen));
            for(ui32I = 0; ui32I < ui32N; ui32I++)
            {
                psMd->buf[psMd->curlen + ui32I] = ui8In[ui32I];
            }
            psMd->curlen += ui32N;
            ui8In  += ui32N;
            ui32InLen  -= ui32N;
            if(psMd->curlen == SHA256_BLOCK_SIZE && ui32InLen > 0)
            {
                if((ui8Err = SHA256HashResume(psMd, (uint8_t *) psMd->buf,
                                              (uint8_t *)psMd->state)) != SHA256_SUCCESS)
                {
                    g_ui8CurrentAESOp = AES_NONE;
                    return (ui8Err);
                }
                psMd->length += 8 * SHA256_BLOCK_SIZE;
                psMd->curlen = 0;
            }
        }
    }
    g_ui8CurrentAESOp = AES_NONE;
    return (SHA256_SUCCESS);
}

//*****************************************************************************
//
//! SHA256Done function terminates hash session to get the digest. This
//! function must be called only after SHA256Process().
//!
//! \param   psMd is the pointer to hash state.
//! \param   ui8Out is the pointer to hash.
//!
//! For the pointer to hash state parameter \e psMd the calling function has to
//! allocate the hash state structure and pass the pointer to the structure.
//!
//! \return  SHA256_SUCCESS if successful.
//
//*****************************************************************************
uint8_t SHA256Done(tSHA256State * psMd, uint8_t *ui8Out)
{
    uint8_t ui8Err;
    if(psMd  == NULL || ui8Out == NULL)
    {
        return (SHA256_NULL_ERROR);
    }

    if(psMd->curlen > sizeof(psMd->buf))
    {
        return (SHA256_INVALID_PARAM);
    }

    g_ui8CurrentAESOp = AES_SHA256;

    // increase the length of the message
    psMd->length += psMd->curlen * 8;
    psMd->final_digest = true;
    if(psMd->new_digest == true)
    {
        if((ui8Err = SHA256HashNew(psMd, (uint8_t *)psMd->buf,
                                   (uint8_t *)ui8Out)) != SHA256_SUCCESS)
        {
            g_ui8CurrentAESOp = AES_NONE;
            return (ui8Err);
        }
    }
    else
    {
        if((ui8Err = SHA256HashResume(psMd, (uint8_t *)psMd->buf,
                                      (uint8_t *)ui8Out)) != SHA256_SUCCESS)
        {
            g_ui8CurrentAESOp = AES_NONE;
            return (ui8Err);
        }
    }
    psMd->new_digest = false;
    psMd->final_digest = false;

    g_ui8CurrentAESOp = AES_NONE;
    return (SHA256_SUCCESS);
}

//*****************************************************************************
//
//! SHA256HashNew function is to start a new Hash session in hardware.
//!
//! \param   psMd is the hash state.
//! \param   ui8In is the pointer to input message.
//! \param   ui8Out is the destination of the hash (32 bytes).
//!
//! \return  SHA256_SUCCESS if successful.
//
//*****************************************************************************
uint8_t SHA256HashNew(tSHA256State * psMd, uint8_t *ui8In, uint8_t *ui8Out)
{
    // workaround for AES registers not retained after PM2
    IntDisable(INT_AES);
    HWREG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
    HWREG(AES_CTRL_INT_EN)  = (AES_CTRL_INT_EN_RESULT_AV |
                               AES_CTRL_INT_EN_DMA_IN_DONE);

    // configure master control module
    // enable DMA path to the SHA-256 engine + Digest readout
    HWREG(AES_CTRL_ALG_SEL) = (AES_CTRL_ALG_SEL_TAG | AES_CTRL_ALG_SEL_HASH);
    // clear any outstanding events
    HWREG(AES_CTRL_INT_CLR) =  AES_CTRL_INT_CLR_RESULT_AV;

    // configure hash engine
    // indicate start of a new hash session and SHA256
    HWREG(AES_HASH_MODE_IN) = (AES_HASH_MODE_IN_SHA256_MODE |
                               AES_HASH_MODE_IN_NEW_HASH);

    // if the final digest is required (pad the input DMA data),
    // write the following register
    //
    if(psMd->final_digest)
    {
        // write length of the message (lo)
        HWREG(AES_HASH_LENGTH_IN_L) = (uint32_t)psMd->length;
        // write length of the message (hi)
        HWREG(AES_HASH_LENGTH_IN_H) = (uint32_t)(psMd->length >> 16);
        // pad the DMA-ed data
        HWREG(AES_HASH_IO_BUF_CTRL) = AES_HASH_IO_BUF_CTRL_PAD_DMA_MESSAGE;
    }

    // enable DMA channel 0 for message data
    HWREG(AES_DMAC_CH0_CTRL) |= AES_DMAC_CH0_CTRL_EN;
    // base address of the data in ext. memory
    HWREG(AES_DMAC_CH0_EXTADDR) = (uint32_t)ui8In;
    if(psMd->final_digest)
    {
        // input data length in bytes, equal to the message
        HWREG(AES_DMAC_CH0_DMALENGTH) = psMd->curlen;
    }
    else
    {
        HWREG(AES_DMAC_CH0_DMALENGTH) = SHA256_BLOCK_SIZE;
    }

    // enable DMA channel 1 for result digest
    HWREG(AES_DMAC_CH1_CTRL) |= AES_DMAC_CH1_CTRL_EN;
    // base address of the digest buffer
    HWREG(AES_DMAC_CH1_EXTADDR) = (uint32_t)ui8Out;
    // length of the result digest
    HWREG(AES_DMAC_CH1_DMALENGTH) = SHA256_OUTPUT_LEN;

    // wait for completion of the operation
    do
    {
        ASM_NOP;
    }
    while(!(HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV));


    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
    {
        return (AES_DMA_BUS_ERROR);
    }

    // clear the interrupt
    HWREG(AES_CTRL_INT_CLR) = (AES_CTRL_INT_CLR_DMA_IN_DONE |
                               AES_CTRL_INT_CLR_RESULT_AV);
    // disable master control/DMA clock
    HWREG(AES_CTRL_ALG_SEL) = 0x00000000;
    // clear mode
    HWREG(AES_AES_CTRL) = 0x00000000;

    return (SHA256_SUCCESS);
}

//*****************************************************************************
//
//! SHA256HashResume function resumes an already started hash session in
//! hardware.
//!
//! \param   psMd is the hash state.
//! \param   ui8In is the pointer to the input message.
//! \param   ui8Out is the pointer to the destination of the hash (32 bytes).
//!
//! \return  SHA256_SUCCESS if successful.
//
//*****************************************************************************
uint8_t SHA256HashResume(tSHA256State * psMd, uint8_t *ui8In, uint8_t *ui8Out)
{
    IntDisable(INT_AES);
    // workaround for AES registers not retained after PM2
    HWREG(AES_CTRL_INT_CFG) = AES_CTRL_INT_CFG_LEVEL;
    HWREG(AES_CTRL_INT_EN)  = (AES_CTRL_INT_EN_RESULT_AV |
                               AES_CTRL_INT_EN_DMA_IN_DONE);

    // configure master control module and enable
    // the DMA path to the SHA-256 engine
    //
    HWREG(AES_CTRL_ALG_SEL) = AES_CTRL_ALG_SEL_HASH;

    // clear any outstanding events
    HWREG(AES_CTRL_INT_CLR) =  AES_CTRL_INT_CLR_RESULT_AV;

    // configure hash engine
    // indicate the start of a resumed hash session and SHA256
    HWREG(AES_HASH_MODE_IN) = AES_HASH_MODE_IN_SHA256_MODE;

    // if the final digest is required (pad the input DMA data)
    if(psMd->final_digest)
    {
        // write length of the message (lo)
        HWREG(AES_HASH_LENGTH_IN_L) = (uint32_t)psMd->length;
        // write length of the message (hi)
        HWREG(AES_HASH_LENGTH_IN_H) = (uint32_t)(psMd->length >> 16);
    }

    // write the initial digest
    HWREG(AES_HASH_DIGEST_A) = (uint32_t)psMd->state[0];
    HWREG(AES_HASH_DIGEST_B) = (uint32_t)psMd->state[1];
    HWREG(AES_HASH_DIGEST_C) = (uint32_t)psMd->state[2];
    HWREG(AES_HASH_DIGEST_D) = (uint32_t)psMd->state[3];
    HWREG(AES_HASH_DIGEST_E) = (uint32_t)psMd->state[4];
    HWREG(AES_HASH_DIGEST_F) = (uint32_t)psMd->state[5];
    HWREG(AES_HASH_DIGEST_G) = (uint32_t)psMd->state[6];
    HWREG(AES_HASH_DIGEST_H) = (uint32_t)psMd->state[7];

    // If final digest, pad the DMA-ed data
    if(psMd->final_digest)
    {
        HWREG(AES_HASH_IO_BUF_CTRL) = AES_HASH_IO_BUF_CTRL_PAD_DMA_MESSAGE;
    }

    // enable DMA channel 0 for message data
    HWREG(AES_DMAC_CH0_CTRL) |= AES_DMAC_CH0_CTRL_EN;
    // base address of the data in ext. memory
    HWREG(AES_DMAC_CH0_EXTADDR) = (uint32_t)ui8In;
    // input data length in bytes, equal to the message
    if(psMd->final_digest)
    {
        HWREG(AES_DMAC_CH0_DMALENGTH) = psMd->curlen;
    }
    else
    {
        HWREG(AES_DMAC_CH0_DMALENGTH) = SHA256_BLOCK_SIZE;
    }

    // wait for completion of the operation
    do
    {
        ASM_NOP;
    }
    while(!(HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_RESULT_AV));

    // check for any DMA Bus errors
    if((HWREG(AES_CTRL_INT_STAT) & AES_CTRL_INT_STAT_DMA_BUS_ERR))
    {
        return (AES_DMA_BUS_ERROR);
    }

    // read digest
    ((uint32_t  *)ui8Out)[0] = HWREG(AES_HASH_DIGEST_A);
    ((uint32_t  *)ui8Out)[1] = HWREG(AES_HASH_DIGEST_B);
    ((uint32_t  *)ui8Out)[2] = HWREG(AES_HASH_DIGEST_C);
    ((uint32_t  *)ui8Out)[3] = HWREG(AES_HASH_DIGEST_D);
    ((uint32_t  *)ui8Out)[4] = HWREG(AES_HASH_DIGEST_E);
    ((uint32_t  *)ui8Out)[5] = HWREG(AES_HASH_DIGEST_F);
    ((uint32_t  *)ui8Out)[6] = HWREG(AES_HASH_DIGEST_G);
    ((uint32_t  *)ui8Out)[7] = HWREG(AES_HASH_DIGEST_H);

    // acknowledge reading of the digest
    HWREG(AES_HASH_IO_BUF_CTRL) = AES_HASH_IO_BUF_CTRL_OUTPUT_FULL;

    // clear the interrupt
    HWREG(AES_CTRL_INT_CLR) = (AES_CTRL_INT_CLR_DMA_IN_DONE |
                               AES_CTRL_INT_CLR_RESULT_AV);
    // acknowledge result and clear interrupts
    // disable master control/DMA clock
    HWREG(AES_CTRL_ALG_SEL) = 0x00000000;
    // clear mode
    HWREG(AES_AES_CTRL) = 0x00000000;

    return (SHA256_SUCCESS);
}

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//
//*****************************************************************************

