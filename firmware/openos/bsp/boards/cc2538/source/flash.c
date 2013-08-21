/******************************************************************************
*  Filename:       flash.c
*  Revised:        $Date: 2013-03-24 14:46:31 +0100 (Sun, 24 Mar 2013) $
*  Revision:       $Revision: 9524 $
*
*  Description:    Driver for programming the on-chip flash.
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
//! \addtogroup flash_api
//! @{
//
//*****************************************************************************

#include "hw_flash_ctrl.h"
#include "hw_memmap.h"
#include "debug.h"
#include "flash.h"
#include "rom.h"

//*****************************************************************************
//
//! Erases a flash main page with use of ROM function
//!
//! \param ui32Address is the start address of the flash main page to be erased.
//!
//! This function erases one 2 kB main page of the on-chip flash. After
//! erasing, the page is filled with 0xFF bytes. Locked pages cannot be
//! erased. The flash main pages do not include the upper page.
//!
//! This function does not return until the page is erased or an error
//! encountered.
//!
//! \return Returns 0 on success, -1 if erasing error is encountered,
//!         or -2 in case of illegal parameter use.
//
//*****************************************************************************
int32_t
FlashMainPageErase(uint32_t ui32Address)
{
    int32_t          i32Stat;               // 0 = pass, -1 = fail
    uint32_t ui32CurrentCacheMode;

    i32Stat = 0;

    //
    // Check the arguments.
    //
    ASSERT(!(ui32Address < FLASH_BASE));
    ASSERT(!(ui32Address >= (FLASH_BASE + (FlashSizeGet() * 1024) -
                             FLASH_ERASE_SIZE)));
    ASSERT(!(ui32Address & (FLASH_ERASE_SIZE - 1)));

    //
    // Save current cache mode since the ROM function will change it.
    //
    ui32CurrentCacheMode = FlashCacheModeGet();

    //
    // Erase the specified flash main page by calling ROM function.
    //
    i32Stat = ROM_PageErase(ui32Address, FLASH_ERASE_SIZE);

    //
    // Restore cache mode.
    //
    FlashCacheModeSet(ui32CurrentCacheMode);

    //
    // Return status pass or fail.
    //
    return(i32Stat);
}

//*****************************************************************************
//
//! Erases the upper flash page with use of ROM function
//!
//! This function erases the 2 kB upper page of the on-chip flash. After
//! erasing, the page is filled with 0xFF bytes. A locked page cannot
//! be erased.
//!
//! This function does not return until the flash page is erased or
//! an error encountered.
//!
//! \return Returns 0 on success, -1 if erasing error is encountered
//!         or, -2 in case of illegal parameter use.
//
//*****************************************************************************
int32_t
FlashUpperPageErase(void)
{
    uint32_t ui32UpperPageAddr;
    uint32_t ui32CurrentCacheMode;
    int32_t  i32Stat;                  // 0 = pass, -1 = fail, -2 = wrong param

    i32Stat = 0;

    //
    // Find start address of upper flash page
    //
    ui32UpperPageAddr = FLASH_BASE + (FlashSizeGet() * 1024) - FLASH_ERASE_SIZE;

    //
    // Save current cache mode since the ROM function will change it.
    //
    ui32CurrentCacheMode = FlashCacheModeGet();

    //
    // Erase the upper flash page by calling ROM function.
    //
    i32Stat = ROM_PageErase(ui32UpperPageAddr, FLASH_ERASE_SIZE);

    //
    // Restore cache mode.
    //
    FlashCacheModeSet(ui32CurrentCacheMode);

    //
    // Return status pass or fail.
    //
    return(i32Stat);
}

//*****************************************************************************
//
//! Programs the flash main pages by use of ROM function
//!
//! \param pui32Data is a pointer to the data to be programmed.
//! \param ui32Address is the starting address in flash to be programmed. Must
//! be a multiple of four and within the flash main pages.
//! \param ui32Count is the number of bytes to be programmed. Must be a multiple
//! of four.
//!
//! This function programs a sequence of words into the on-chip flash.
//! Programming each location consists of the result of an AND operation
//! of the new data and the existing data; in other words, bits that contain
//! 1 can remain 1 or be changed to 0, but bits that are 0 cannot be changed
//! to 1. Therefore, a word can be programmed multiple times as long as these
//! rules are followed; if a program operation attempts to change a 0 bit to
//! a 1 bit, that bit will not have its value changed.
//!
//! Because the flash is programmed one word at a time, the starting address and
//! byte count must both be multiples of four. The caller must
//! verify the programmed contents, if verification is required.
//!
//! This function does not return until the data is programmed or an
//! error encountered. Locked flash pages cannot be programmed.
//!
//! \return Returns 0 on success, -1 if a programming error is encountered
//!         or, -2 in case of illegal parameter use.
//
//*****************************************************************************
int32_t
FlashMainPageProgram(uint32_t *pui32Data, uint32_t ui32Address,
                     uint32_t ui32Count)
{
    uint32_t ui32CurrentCacheMode;
    int32_t  i32Stat;     // 0 = pass, -1 = fail, -2 = wrong param

    i32Stat = 0;            // Start out passing

    //
    // Check the arguments.
    //
    ASSERT(!(ui32Address             < FLASH_BASE));
    ASSERT(!((ui32Address + ui32Count) > (FLASH_BASE + (FlashSizeGet() * 1024) -
                                          FLASH_ERASE_SIZE)));
    ASSERT(!(ui32Address & 3));
    ASSERT(!(ui32Count   & 3));

    //
    // Save current cache mode since the ROM function will change it.
    //
    ui32CurrentCacheMode = FlashCacheModeGet();

    //
    // Program flash by executing function in ROM.
    //
    i32Stat = ROM_ProgramFlash(pui32Data, ui32Address, ui32Count);

    //
    // Restore cache mode.
    //
    FlashCacheModeSet(ui32CurrentCacheMode);

    //
    // Return status pass or fail.
    //
    return(i32Stat);
}

//*****************************************************************************
//
//! Programs the upper page of the flash by use of ROM function
//!
//! \param pui32Data is a pointer to the data to be programmed.
//! \param ui32Address is the starting address within the flash upper page to be
//! programmed. Must be a multiple of four and within the flash upper page.
//! \param ui32Count is the number of bytes to be programmed.  Must be a multiple
//! of four.
//!
//! This function programs a sequence of words into the on-chip flash.
//! Programming each location consists of the result of an AND operation
//! of the new data and the existing data; in other words, bits that contain
//! 1 can remain 1 or be changed to 0, but bits that are 0 cannot be changed
//! to 1. Therefore, a word can be programmed multiple times as long as these
//! rules are followed; if a program operation attempts to change a 0 bit to
//! a 1 bit, that bit will not have its value changed.
//!
//! Because the flash is programmed one word at a time, the starting address and
//! byte count must both be multiples of four. The caller must
//! verify the programmed contents, if such verification is required.
//!
//! This function does not return until the data is programmed or an
//! error encountered. A locked flash page cannot be programmed.
//!
//! \return Returns 0 on success, -1 if a programming error is encountered
//!         or, -2 in case of illegal parameter use.
//
//*****************************************************************************
int32_t
FlashUpperPageProgram(uint32_t *pui32Data, uint32_t ui32Address,
                      uint32_t ui32Count)
{
    uint32_t ui32CurrentCacheMode;
    int32_t  i32Stat;                // 0 = pass, -1 = fail, -2 = wrong param

    i32Stat = 0;                     // Start out passing

    //
    // Check the arguments.
    //
    ASSERT(!(ui32Address < (FLASH_BASE + (FlashSizeGet() * 1024) -
                            FLASH_ERASE_SIZE)));
    ASSERT(!((ui32Address + ui32Count) > (FLASH_BASE +
                                          (FlashSizeGet() * 1024))));
    ASSERT(!(ui32Address & 3));
    ASSERT(!(ui32Count   & 3));

    //
    // Save current cache mode since the ROM function will change it.
    //
    ui32CurrentCacheMode = FlashCacheModeGet();

    //
    // Program flash by executing function in ROM.
    //
    i32Stat = ROM_ProgramFlash(pui32Data, ui32Address, ui32Count);

    //
    // Clear flash controller register bit set by ROM function.
    //
    HWREG(FLASH_CTRL_FCTL) &= (~FLASH_CTRL_FCTL_UPPER_PAGE_ACCESS);

    //
    // Restore cache mode.
    //
    FlashCacheModeSet(ui32CurrentCacheMode);

    //
    // Return status pass or fail.
    //
    return(i32Stat);
}

//*****************************************************************************
//
//! Gets the current contents of the flash at the designated address
//!
//! \param ui32Addr is the desired address to be read within the flash.
//!
//! This function helps differentiate flash memory reads from flash
//! register reads.
//!
//! \return Returns the 32bit value as an uint32_t value.
//
//*****************************************************************************
uint32_t
FlashGet(uint32_t ui32Addr)
{
    return(HWREG(ui32Addr));
}

//*****************************************************************************
//
//! Gets the current state of the flash Cache Mode
//!
//! This function gets the current setting for the Cache Mode.
//!
//! \return Returns the CM bits. Return value should match one of the
//! FLASH_CACHE_MODE_<> macros defined in flash.h.
//
//*****************************************************************************
uint32_t
FlashCacheModeGet(void)
{
    //
    // Return a FLASH_CACHE_MODE_<> macro value.
    //
    return(HWREG(FLASH_CTRL_FCTL) & FLASH_CTRL_FCTL_CM_M);
}

//*****************************************************************************
//
//! Sets the flash Cache Mode state
//!
//! \param ui32CacheMode is the desired cache mode.
//!
//! This function sets the flash Cache Mode to the desired state and accepts
//! a right justified 2 bit setting for the Cachemode bits. The function waits
//! for the flash to be idle, reads the FCTL register contents, masks in the
//! requested setting, and writes it into the FCTL register.
//!
//! The parameter \e ui32CacheMode can have one of the following values:
//!
//! - \b FLASH_CTRL_CACHE_MODE_DISABLE
//! - \b FLASH_CTRL_CACHE_MODE_ENABLE
//! - \b FLASH_CTRL_CACHE_MODE_PREFETCH_ENABLE
//! - \b FLASH_CTRL_CACHE_MODE_REALTIME
//!
//! \return None
//
//*****************************************************************************
void
FlashCacheModeSet(uint32_t ui32CacheMode)
{
    uint32_t ui32Busy;
    uint32_t ui32TempValue;

    //
    // Check the arguments.
    //
    ASSERT((ui32CacheMode == FLASH_CTRL_CACHE_MODE_DISABLE) ||
           (ui32CacheMode == FLASH_CTRL_CACHE_MODE_ENABLE) ||
           (ui32CacheMode == FLASH_CTRL_CACHE_MODE_PREFETCH_ENABLE) ||
           (ui32CacheMode == FLASH_CTRL_CACHE_MODE_REALTIME));

    //
    // Wait until FLASH is not busy.
    //
    ui32Busy = 1;
    while(ui32Busy)
    {
        ui32TempValue = HWREG(FLASH_CTRL_FCTL);
        ui32Busy      = ui32TempValue & FLASH_CTRL_FCTL_BUSY;
    }

    //
    // Set desired cache mode.
    //
    ui32TempValue           &= ~FLASH_CTRL_FCTL_CM_M;
    HWREG(FLASH_CTRL_FCTL) = ui32TempValue | ui32CacheMode;
}

//*****************************************************************************
//
//! Returns the flash size in number of KBytes
//!
//! This function returns the size of the flash in KBytes as determined by
//! examining the FLASH_DIECFG0 register settings.
//!
//! \return Returns the flash size in KBytes
//
//*****************************************************************************
uint32_t
FlashSizeGet(void)
{
    uint32_t ui32RegValue;
    uint32_t ui32Size;

    ui32RegValue = HWREG(FLASH_CTRL_DIECFG0);
    ui32RegValue = (ui32RegValue & FLASH_CTRL_DIECFG0_FLASH_SIZE_M) >>
                   FLASH_CTRL_DIECFG0_FLASH_SIZE_S;

    switch(ui32RegValue)
    {
    case 0x04:
        ui32Size = 512;
        break;
    case 0x03:
        ui32Size = 384;
        break;
    case 0x02:
        ui32Size = 256;
        break;
    case 0x01:
        ui32Size = 128;
        break;
    case 0x00:
        ui32Size =  64;
        break;
    default:
        ui32Size =  64;
        break;
    }
    return(ui32Size);
}

//*****************************************************************************
//
//! Returns the SRAM size in number of KBytes
//!
//! This function returns the size of the SRAM in KBytes as determined by
//! examining the FLASH_DIECFG0 register settings.
//!
//! \return Returns the SRAM size in KBytes
//
//*****************************************************************************
uint32_t
FlashSramSizeGet(void)
{
    uint32_t ui32RegValue;
    uint32_t ui32Size;

    ui32RegValue = HWREG(FLASH_CTRL_DIECFG0);
    ui32RegValue = (ui32RegValue & FLASH_CTRL_DIECFG0_SRAM_SIZE_M) >>
                   FLASH_CTRL_DIECFG0_SRAM_SIZE_S;

    switch(ui32RegValue)
    {
    case 0x04:
        ui32Size = 32;
        break;
    case 0x01:
        ui32Size =  8;
        break;
    case 0x00:
        ui32Size = 16;
        break;
    default:
        ui32Size = 32;
        break;
    }
    return(ui32Size);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
