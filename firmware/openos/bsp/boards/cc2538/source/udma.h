/******************************************************************************
*  Filename:       udma.h
*  Revised:        $Date: 2013-02-19 10:35:37 +0100 (Tue, 19 Feb 2013) $
*  Revision:       $Revision: 9322 $
*
*  Description:    Prototypes and macros for the uDMA controller.
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

#ifndef __UDMA_H__
#define __UDMA_H__

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
//! \addtogroup udma_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// A structure that defines an entry in the channel control table.  These
// fields are used by the uDMA controller and normally it is not necessary for
// software to directly read or write fields in the table.
//
//*****************************************************************************
typedef struct
{
    //
    // The ending source address of the data transfer.
    //
    volatile void *pvSrcEndAddr;

    //
    // The ending destination address of the data transfer.
    //
    volatile void *pvDstEndAddr;

    //
    // The channel control mode.
    //
    volatile uint32_t ui32Control;

    //
    // An unused location.
    //
    volatile uint32_t ui32Spare;
}
tDMAControlTable;

//*****************************************************************************
//
//! A helper macro for building scatter-gather task table entries.
//!
//! \param ui32TransferCount is the count of items to transfer for this task.
//! \param ui32ItemSize is the bit size of the items to transfer for this task.
//! \param ui32SrcIncrement is the bit size increment for source data.
//! \param pvSrcAddr is the starting address of the data to transfer.
//! \param ui32DstIncrement is the bit size increment for destination data.
//! \param pvDstAddr is the starting address of the destination data.
//! \param ui32ArbSize is the arbitration size to use for the transfer task.
//! \param ui32Mode is the transfer mode for this task.
//!
//! This macro is intended to be used to help populate a table of uDMA tasks
//! for a scatter-gather transfer.  This macro will calculate the values for
//! the fields of a task structure entry based on the input parameters.
//!
//! There are specific requirements for the values of each parameter.  No
//! checking is done so it is up to the caller to ensure that correct values
//! are used for the parameters.
//!
//! The \e ui32TransferCount parameter is the number of items that will be
//! transferred by this task.  It must be in the range 1-1024.
//!
//! The \e ui32ItemSize parameter is the bit size of the transfer data.  It must
//! be one of \b UDMA_SIZE_8, \b UDMA_SIZE_16, or \b UDMA_SIZE_32.
//!
//! The \e ui32SrcIncrement parameter is the increment size for the source data.
//! It must be one of \b UDMA_SRC_INC_8, \b UDMA_SRC_INC_16,
//! \b UDMA_SRC_INC_32, or \b UDMA_SRC_INC_NONE.
//!
//! The \e pvSrcAddr parameter is a void pointer to the beginning of the source
//! data.
//!
//! The \e ui32DstIncrement parameter is the increment size for the destination
//! data.  It must be one of \b UDMA_DST_INC_8, \b UDMA_DST_INC_16,
//! \b UDMA_DST_INC_32, or \b UDMA_DST_INC_NONE.
//!
//! The \e pvDstAddr parameter is a void pointer to the beginning of the
//! location where the data will be transferred.
//!
//! The \e ui32ArbSize parameter is the arbitration size for the transfer, and
//! must be one of \b UDMA_ARB_1, \b UDMA_ARB_2, \b UDMA_ARB_4, and so on
//! up to \b UDMA_ARB_1024.  This is used to select the arbitration size in
//! powers of 2, from 1 to 1024.
//!
//! The \e ui32Mode parameter is the mode to use for this transfer task.  It
//! must be one of \b UDMA_MODE_BASIC, \b UDMA_MODE_AUTO,
//! \b UDMA_MODE_MEM_SCATTER_GATHER, or \b UDMA_MODE_PER_SCATTER_GATHER.  Note
//! that normally all tasks will be one of the scatter-gather modes while the
//! last task is a task list will be AUTO or BASIC.
//!
//! This macro is intended to be used to initialize individual entries of
//! a structure of tDMAControlTable type, like this:
//!
//! \verbatim
//!     tDMAControlTable MyTaskList[] =
//!     {
//!         uDMATaskStructEntry(Task1Count, UDMA_SIZE_8,
//!                             UDMA_SRC_INC_8, MySourceBuf,
//!                             UDMA_DST_INC_8, MyDestBuf,
//!                             UDMA_ARB_8, UDMA_MODE_MEM_SCATTER_GATHER),
//!         uDMATaskStructEntry(Task2Count, ... ),
//!     }
//! \endverbatim
//!
//! \return Nothing; this is not a function.
//
//*****************************************************************************
#define uDMATaskStructEntry(ui32TransferCount,                                  \
                            ui32ItemSize,                                       \
                            ui32SrcIncrement,                                   \
                            pvSrcAddr,                                        \
                            ui32DstIncrement,                                   \
                            pvDstAddr,                                        \
                            ui32ArbSize,                                        \
                            ui32Mode)                                           \
    {                                                                         \
        (((ui32SrcIncrement) == UDMA_SRC_INC_NONE) ? (pvSrcAddr) :              \
            ((void *)(&((uint8_t *)(pvSrcAddr))[((ui32TransferCount) <<   \
                                         ((ui32SrcIncrement) >> 26)) - 1]))),   \
        (((ui32DstIncrement) == UDMA_DST_INC_NONE) ? (pvDstAddr) :              \
            ((void *)(&((uint8_t *)(pvDstAddr))[((ui32TransferCount) <<   \
                                         ((ui32DstIncrement) >> 30)) - 1]))),   \
        (ui32SrcIncrement) | (ui32DstIncrement) | (ui32ItemSize) | (ui32ArbSize) |    \
        (((ui32TransferCount) - 1) << 4) |                                      \
        ((((ui32Mode) == UDMA_MODE_MEM_SCATTER_GATHER) ||                       \
          ((ui32Mode) == UDMA_MODE_PER_SCATTER_GATHER)) ?                       \
                (ui32Mode) | UDMA_MODE_ALT_SELECT : (ui32Mode)), 0                \
    }

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Flags that can be passed to uDMAChannelAttributeEnable(),
// uDMAChannelAttributeDisable(), and returned from uDMAChannelAttributeGet().
//
//*****************************************************************************
#define UDMA_ATTR_USEBURST      0x00000001
#define UDMA_ATTR_ALTSELECT     0x00000002
#define UDMA_ATTR_HIGH_PRIORITY 0x00000004
#define UDMA_ATTR_REQMASK       0x00000008
#define UDMA_ATTR_ALL           0x0000000F

//*****************************************************************************
//
// DMA control modes that can be passed to uDMAModeSet() and returned
// uDMAModeGet().
//
//*****************************************************************************
#define UDMA_MODE_STOP          0x00000000
#define UDMA_MODE_BASIC         0x00000001
#define UDMA_MODE_AUTO          0x00000002
#define UDMA_MODE_PINGPONG      0x00000003
#define UDMA_MODE_MEM_SCATTER_GATHER                                          \
                                0x00000004
#define UDMA_MODE_PER_SCATTER_GATHER                                          \
                                0x00000006
#define UDMA_MODE_ALT_SELECT    0x00000001

//*****************************************************************************
//
// Channel configuration values that can be passed to uDMAControlSet().
//
//*****************************************************************************
#define UDMA_DST_INC_8          0x00000000
#define UDMA_DST_INC_16         0x40000000
#define UDMA_DST_INC_32         0x80000000
#define UDMA_DST_INC_NONE       0xc0000000
#define UDMA_SRC_INC_8          0x00000000
#define UDMA_SRC_INC_16         0x04000000
#define UDMA_SRC_INC_32         0x08000000
#define UDMA_SRC_INC_NONE       0x0c000000
#define UDMA_SIZE_8             0x00000000
#define UDMA_SIZE_16            0x11000000
#define UDMA_SIZE_32            0x22000000
#define UDMA_ARB_1              0x00000000
#define UDMA_ARB_2              0x00004000
#define UDMA_ARB_4              0x00008000
#define UDMA_ARB_8              0x0000c000
#define UDMA_ARB_16             0x00010000
#define UDMA_ARB_32             0x00014000
#define UDMA_ARB_64             0x00018000
#define UDMA_ARB_128            0x0001c000
#define UDMA_ARB_256            0x00020000
#define UDMA_ARB_512            0x00024000
#define UDMA_ARB_1024           0x00028000
#define UDMA_NEXT_USEBURST      0x00000008

//*****************************************************************************
//
// Flags to be OR'd with the channel ID to indicate if the primary or alternate
// control structure should be used.
//
//*****************************************************************************
#define UDMA_PRI_SELECT         0x00000000
#define UDMA_ALT_SELECT         0x00000020

//*****************************************************************************
//
// uDMA interrupt sources, to be passed to uDMAIntRegister() and
// uDMAIntUnregister().
//
//*****************************************************************************
#define UDMA_INT_SW             62
#define UDMA_INT_ERR            63

//*****************************************************************************
//
// Values that can be passed to uDMAChannelMapConfigure() to select peripheral
// mapping for each channel.  The channels named RESERVED may be assigned
// to a peripheral in future parts.
//
//*****************************************************************************
//
// Channel 0
//
#define UDMA_CH0_RESERVED0          0x00000000
#define UDMA_CH0_RESERVED1          0x00010000
#define UDMA_CH0_RESERVED2          0x00020000
#define UDMA_CH0_RESERVED3          0x00030000
#define UDMA_CH0_USB                0x00040000

//
// Channel 1
//
#define UDMA_CH1_RESERVED0          0x00000001
#define UDMA_CH1_RESERVED1          0x00010001
#define UDMA_CH1_RESERVED2          0x00020001
#define UDMA_CH1_RESERVED3          0x00030001
#define UDMA_CH1_ADC                0x00040001

//
// Channel 2
//
#define UDMA_CH2_RESERVED0          0x00000002
#define UDMA_CH2_TIMER3A            0x00010002
#define UDMA_CH2_RESERVED2          0x00020002
#define UDMA_CH2_RESERVED3          0x00030002
#define UDMA_CH2_FLASH              0x00040002

//
// Channel 3
//
#define UDMA_CH3_RESERVED0          0x00000003
#define UDMA_CH3_TIMER3B            0x00010003
#define UDMA_CH3_RESERVED2          0x00020003
#define UDMA_CH3_RESERVED3          0x00030003
#define UDMA_CH3_RFCORETRG1         0x00040003

//
// Channel 4
//
#define UDMA_CH4_RESERVED0          0x00000004
#define UDMA_CH4_TIMER2A            0x00010004
#define UDMA_CH4_RESERVED2          0x00020004
#define UDMA_CH4_RESERVED3          0x00030004
#define UDMA_CH4_RFCORETRG2         0x00040004

//
// Channel 5
//
#define UDMA_CH5_RESERVED0          0x00000005
#define UDMA_CH5_TIMER2B            0x00010005
#define UDMA_CH5_RESERVED2          0x00020005
#define UDMA_CH5_RESERVED3          0x00030005
#define UDMA_CH5_RESERVED4          0x00040005

//
// Channel 6
//
#define UDMA_CH6_RESERVED0          0x00000006
#define UDMA_CH6_TIMER2A            0x00010006
#define UDMA_CH6_RESERVED2          0x00020006
#define UDMA_CH6_RESERVED3          0x00030006
#define UDMA_CH6_RESERVED4          0x00040006

//
// Channel 7
//
#define UDMA_CH7_RESERVED0          0x00000007
#define UDMA_CH7_TIMER2B            0x00010007
#define UDMA_CH7_RESERVED2          0x00020007
#define UDMA_CH7_RESERVED3          0x00030007
#define UDMA_CH7_RESERVED4          0x00040007

//
// Channel 8
//
#define UDMA_CH8_UART0RX            0x00000008
#define UDMA_CH8_UART1RX            0x00010008
#define UDMA_CH8_RESERVED2          0x00020008
#define UDMA_CH8_RESERVED3          0x00030008
#define UDMA_CH8_RESERVED4          0x00040008

//
// Channel 9
//
#define UDMA_CH9_UART0TX            0x00000009
#define UDMA_CH9_UART1TX            0x00010009
#define UDMA_CH9_RESERVED2          0x00020009
#define UDMA_CH9_RESERVED3          0x00030009
#define UDMA_CH9_RESERVED4          0x00040009

//
// Channel 10
//
#define UDMA_CH10_SSI0RX            0x0000000A
#define UDMA_CH10_SSI1RX            0x0001000A
#define UDMA_CH10_RESERVED2         0x0002000A
#define UDMA_CH10_RESERVED3         0x0003000A
#define UDMA_CH10_RESERVED4         0x0004000A

//
// Channel 11
//
#define UDMA_CH11_SSI0TX            0x0000000B
#define UDMA_CH11_SSI1TX            0x0001000B
#define UDMA_CH11_RESERVED2         0x0002000B
#define UDMA_CH11_RESERVED3         0x0003000B
#define UDMA_CH11_RESERVED4         0x0004000B

//
// Channel 12
//
#define UDMA_CH12_RESERVED0         0x0000000C
#define UDMA_CH12_RESERVED1         0x0001000C
#define UDMA_CH12_RESERVED2         0x0002000C
#define UDMA_CH12_RESERVED3         0x0003000C
#define UDMA_CH12_RESERVED4         0x0004000C

//
// Channel 13
//
#define UDMA_CH13_RESERVED0         0x0000000D
#define UDMA_CH13_RESERVED1         0x0001000D
#define UDMA_CH13_RESERVED2         0x0002000D
#define UDMA_CH13_RESERVED3         0x0003000D
#define UDMA_CH13_RESERVED4         0x0004000D

//
// Channel 14
//
#define UDMA_CH14_ADC0              0x0000000E
#define UDMA_CH14_TIMER2A           0x0001000E
#define UDMA_CH14_RESERVED2         0x0002000E
#define UDMA_CH14_RESERVED3         0x0003000E
#define UDMA_CH14_RESERVED4         0x0004000E

//
// Channel 15
//
#define UDMA_CH15_ADC1              0x0000000F
#define UDMA_CH15_TIMER2B           0x0001000F
#define UDMA_CH15_RESERVED2         0x0002000F
#define UDMA_CH15_RESERVED3         0x0003000F
#define UDMA_CH15_RESERVED4         0x0004000F

//
// Channel 16
//
#define UDMA_CH16_ADC2              0x00000010
#define UDMA_CH16_RESERVED1         0x00010010
#define UDMA_CH16_RESERVED2         0x00020010
#define UDMA_CH16_RESERVED3         0x00030010
#define UDMA_CH16_RESERVED4         0x00040010

//
// Channel 17
//
#define UDMA_CH17_ADC3              0x00000011
#define UDMA_CH17_RESERVED1         0x00010011
#define UDMA_CH17_RESERVED2         0x00020011
#define UDMA_CH17_RESERVED3         0x00030011
#define UDMA_CH17_RESERVED4         0x00040011

//
// Channel 18
//
#define UDMA_CH18_TIMER0A           0x00000012
#define UDMA_CH18_TIMER1A           0x00010012
#define UDMA_CH18_RESERVED2         0x00020012
#define UDMA_CH18_RESERVED3         0x00030012
#define UDMA_CH18_RESERVED4         0x00040012

//
// Channel 19
//
#define UDMA_CH19_TIMER0B           0x00000013
#define UDMA_CH19_TIMER1B           0x00010013
#define UDMA_CH19_RESERVED2         0x00020013
#define UDMA_CH19_RESERVED3         0x00030013
#define UDMA_CH19_RESERVED4         0x00040013

//
// Channel 20
//
#define UDMA_CH20_TIMER1A           0x00000014
#define UDMA_CH20_RESERVED1         0x00010014
#define UDMA_CH20_RESERVED2         0x00020014
#define UDMA_CH20_RESERVED3         0x00030014
#define UDMA_CH20_RESERVED4         0x00040014

//
// Channel 21
//
#define UDMA_CH21_TIMER1B           0x00000015
#define UDMA_CH21_RESERVED1         0x00010015
#define UDMA_CH21_RESERVED2         0x00020015
#define UDMA_CH21_RESERVED3         0x00030015
#define UDMA_CH21_RESERVED4         0x00040015

//
// Channel 22
//
#define UDMA_CH22_UART1RX           0x00000016
#define UDMA_CH22_RESERVED1         0x00010016
#define UDMA_CH22_RESERVED2         0x00020016
#define UDMA_CH22_RESERVED3         0x00030016
#define UDMA_CH22_RESERVED4         0x00040016

//
// Channel 23
//
#define UDMA_CH23_UART1TX           0x00000017
#define UDMA_CH23_RESERVED1         0x00010017
#define UDMA_CH23_RESERVED2         0x00020017
#define UDMA_CH23_RESERVED3         0x00030017
#define UDMA_CH23_RESERVED4         0x00040017

//
// Channel 24
//
#define UDMA_CH24_SSI1RX            0x00000018
#define UDMA_CH24_ADC4              0x00010018
#define UDMA_CH24_RESERVED2         0x00020018
#define UDMA_CH24_RESERVED3         0x00030018
#define UDMA_CH24_RESERVED4         0x00040018

//
// Channel 25
//
#define UDMA_CH25_SSI1TX            0x00000019
#define UDMA_CH25_ADC5              0x00010019
#define UDMA_CH25_RESERVED2         0x00020019
#define UDMA_CH25_RESERVED3         0x00030019
#define UDMA_CH25_RESERVED4         0x00040019

//
// Channel 26
//
#define UDMA_CH26_RESERVED0         0x0000001A
#define UDMA_CH26_ADC6              0x0001001A
#define UDMA_CH26_RESERVED2         0x0002001A
#define UDMA_CH26_RESERVED3         0x0003001A
#define UDMA_CH26_RESERVED4         0x0004001A

//
// Channel 27
//
#define UDMA_CH27_RESERVED0         0x0000001B
#define UDMA_CH27_ADC7              0x0001001B
#define UDMA_CH27_RESERVED2         0x0002001B
#define UDMA_CH27_RESERVED3         0x0003001B
#define UDMA_CH27_RESERVED4         0x0004001B

//
// Channel 28
//
#define UDMA_CH28_RESERVED0         0x0000001C
#define UDMA_CH28_RESERVED1         0x0001001C
#define UDMA_CH28_RESERVED2         0x0002001C
#define UDMA_CH28_RESERVED3         0x0003001C
#define UDMA_CH28_RESERVED4         0x0004001C

//
// Channel 29
//
#define UDMA_CH29_RESERVED0         0x0000001D
#define UDMA_CH29_RESERVED1         0x0001001D
#define UDMA_CH29_RESERVED2         0x0002001D
#define UDMA_CH29_RESERVED3         0x0003001D
#define UDMA_CH29_RFCORET2TRG1      0x0004001D

//
// Channel 30
//
#define UDMA_CH30_SW                0x0000001E
#define UDMA_CH30_RESERVED1         0x0001001E
#define UDMA_CH30_RESERVED2         0x0002001E
#define UDMA_CH30_RESERVED3         0x0003001E
#define UDMA_CH30_RFCORET2TRG2      0x0004001E

//
// Channel 31
//
#define UDMA_CH31_RESERVED0         0x0000001F
#define UDMA_CH31_RESERVED1         0x0001001F
#define UDMA_CH31_RESERVED2         0x0002001F
#define UDMA_CH31_RESERVED3         0x0003001F
#define UDMA_CH31_RESERVED4         0x0004001F

// Enumerations used for channel ctrl
#define UDMA_CHCTL_DSTINC_8     0x00000000  // Byte
#define UDMA_CHCTL_DSTINC_16    0x40000000  // Half-word
#define UDMA_CHCTL_DSTINC_32    0x80000000  // Word

#define UDMA_CHCTL_DSTSIZE_8    0x00000000  // Byte
#define UDMA_CHCTL_DSTSIZE_16   0x10000000  // Half-word
#define UDMA_CHCTL_DSTSIZE_32   0x20000000  // Word

#define UDMA_CHCTL_SRCINC_8     0x00000000  // Byte
#define UDMA_CHCTL_SRCINC_16    0x04000000  // Half-word
#define UDMA_CHCTL_SRCINC_32    0x08000000  // Word

#define UDMA_CHCTL_SRCSIZE_8    0x00000000  // Byte
#define UDMA_CHCTL_SRCSIZE_16   0x01000000  // Half-word
#define UDMA_CHCTL_SRCSIZE_32   0x02000000  // Word

#define UDMA_CHCTL_ARBSIZE_1    0x00000000  // 1 Transfer
#define UDMA_CHCTL_ARBSIZE_2    0x00004000  // 2 Transfers
#define UDMA_CHCTL_ARBSIZE_4    0x00008000  // 4 Transfers
#define UDMA_CHCTL_ARBSIZE_8    0x0000C000  // 8 Transfers
#define UDMA_CHCTL_ARBSIZE_16   0x00010000  // 16 Transfers
#define UDMA_CHCTL_ARBSIZE_32   0x00014000  // 32 Transfers
#define UDMA_CHCTL_ARBSIZE_64   0x00018000  // 64 Transfers
#define UDMA_CHCTL_ARBSIZE_128  0x0001C000  // 128 Transfers
#define UDMA_CHCTL_ARBSIZE_256  0x00020000  // 256 Transfers
#define UDMA_CHCTL_ARBSIZE_512  0x00024000  // 512 Transfers
#define UDMA_CHCTL_ARBSIZE_1024 0x00028000  // 1024 Transfers

#define UDMA_CHCTL_XFERMODE_STOP \
                                0x00000000  // Stop
#define UDMA_CHCTL_XFERMODE_BASIC \
                                0x00000001  // Basic
#define UDMA_CHCTL_XFERMODE_AUTO \
                                0x00000002  // Auto-Request
#define UDMA_CHCTL_XFERMODE_PINGPONG \
                                0x00000003  // Ping-Pong
#define UDMA_CHCTL_XFERMODE_MEM_SG \
                                0x00000004  // Memory Scatter-Gather
#define UDMA_CHCTL_XFERMODE_MEM_SGA \
                                0x00000005  // Alternate Memory Scatter-Gather
#define UDMA_CHCTL_XFERMODE_PER_SG \
                                0x00000006  // Peripheral Scatter-Gather
#define UDMA_CHCTL_XFERMODE_PER_SGA \
                                0x00000007  // Alternate Peripheral
                                            // Scatter-Gather

//*****************************************************************************
//
// API Function prototypes
//
//*****************************************************************************
extern void uDMAEnable(void);
extern void uDMADisable(void);
extern uint32_t uDMAErrorStatusGet(void);
extern void uDMAErrorStatusClear(void);
extern void uDMAChannelEnable(uint32_t ui32ChannelNum);
extern void uDMAChannelDisable(uint32_t ui32ChannelNum);
extern bool uDMAChannelIsEnabled(uint32_t ui32ChannelNum);
extern void uDMAControlBaseSet(void *pControlTable);
extern void *uDMAControlBaseGet(void);
extern void *uDMAControlAlternateBaseGet(void);
extern void uDMAChannelRequest(uint32_t ui32ChannelNum);
extern void uDMAChannelAttributeEnable(uint32_t ui32ChannelNum,
                                       uint32_t ui32Attr);
extern void uDMAChannelAttributeDisable(uint32_t ui32ChannelNum,
                                        uint32_t ui32Attr);
extern uint32_t uDMAChannelAttributeGet(uint32_t ui32ChannelNum);
extern void uDMAChannelControlSet(uint32_t ui32ChannelStructIndex,
                                  uint32_t ui32Control);
extern void uDMAChannelTransferSet(uint32_t ui32ChannelStructIndex,
                                   uint32_t ui32Mode, void *pvSrcAddr,
                                   void *pvDstAddr,
                                   uint32_t ui32TransferSize);
extern void uDMAChannelScatterGatherSet(uint32_t ui32ChannelNum,
                                        uint32_t ui32TaskCount,
void *pvTaskList,
                                        uint32_t ui32IsPeriphSG);
extern uint32_t uDMAChannelSizeGet(uint32_t ui32ChannelStructIndex);
extern uint32_t uDMAChannelModeGet(uint32_t ui32ChannelStructIndex);
extern void uDMAIntRegister(uint32_t ui32IntChannel,
                            void (*pfnHandler)(void));
extern void uDMAIntUnregister(uint32_t ui32IntChannel);
extern uint32_t uDMAIntStatus(void);
extern void uDMAIntClear(uint32_t ui32ChanMask);
extern void uDMAChannelAssign(uint32_t ui32Mapping);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __UDMA_H__
