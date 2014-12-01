/**
\brief  The file includes flash routines.
 Based on Freescale Flash driver. 

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012.
 */


/*********************************** Includes ***********************************/

/*********************************** Macros ************************************/

/*********************************** Defines ***********************************/
#ifndef _FLASH_FTFL_H_
#define _FLASH_FTFL_H_
#include "types.h"
/* error code */
#define Flash_OK                    0x00
#define Flash_FACCERR               0x01
#define Flash_FPVIOL                0x02
#define Flash_MGSTAT0               0x04
#define Flash_RDCOLERR              0x08
#define Flash_NOT_ERASED            0x10
#define Flash_CONTENTERR            0x11

/* flash commands */
#define FlashCmd_ProgramLongWord    0x06
#define FlashCmd_SectorErase        0x09


/********************************** Constant ***********************************/

/*********************************** Variables *********************************/

/*********************************** Prototype *********************************/
void Flash_Init(unsigned char FlashClockDiv);
unsigned char Flash_SectorErase(uint_32 FlashPtr);
unsigned char Flash_ByteProgram(uint_32 FlashStartAdd,uint_32 *DataSrcPtr,uint_32 NumberOfBytes);
void SpSub(void);
void SpSubEnd(void);
#endif /*_FLASH_FTFL_H_*/
/* EOF */