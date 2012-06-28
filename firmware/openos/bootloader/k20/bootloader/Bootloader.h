/**
\brief This file is for a USB Mass-Storage Device bootloader.  
      This file has the structures and definitions for the bootloader  
      Based on Freescale bootloader demo for k60. 


The memory map for k20 is:
  Bootloader interrupt vector =                            0x0000_0000 to 0x0000_03FF  // 1024 bytes for static isr vector.
  Flash protection and security registers =                0x0000_0400 to 0x0000_040F 
  Bootloader Flash (47Kb) =                                0x0000_0410 to 0x0000_BFFF
  App interrupt vector =                                   0x0000_C000 to 0x0000_C3FF //1024 bytes
  App Flash protection and security registers (not used) = 0x0000_C400 to 0x0000_C40F
  App Flash (206kb) =                                      0x0000_C410 to 0x0003_FFFF
  Redirected interrupt vector in RAM =                     0x1FFF_8000 to 0x1FFF_83FF //1024bytes
  RAM available  =                                         0x1FFF_8400 to 0x2000_8000 // 63kb

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012.
*/

#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "types.h"
#include "derivative.h"
#if (defined MCU_MK60N512VMD100)
#define MIN_RAM1_ADDRESS        0x1FFF0000
#define MAX_RAM1_ADDRESS        0x20010000
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x0007FFFF
#define IMAGE_ADDR              ((uint_32_ptr)0xC000)
#define PROT_VALUE0				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE1				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE2				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE3				0xF8 	  // Protects 0x0 - 0xBFFF
#define ERASE_SECTOR_SIZE       (0x800)  /* 2K bytes*/
#elif (defined MCU_MK20D7)
#define MIN_RAM1_ADDRESS        0x1FFF8000//32kb before 0x1FFF_FFFF
#define MAX_RAM1_ADDRESS        0x20008000//32kb after 0x2000_0000
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x0003FFFF //256kb
#define IMAGE_ADDR              ((uint_32_ptr)0xC000)
#define PROT_VALUE0				0xFF 	  // Protects 0x0 - 0xBFFF -- bootloader boundary
#define PROT_VALUE1				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE2				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE3				0xF8 	  // Protects 0x0 - 0xBFFF
#define ERASE_SECTOR_SIZE       (0x800)  /* 2K bytes*/
#endif


#define S19_RECORD_HEADER       0x53303033
/* DES space less than this is protected */
#define FLASH_PROTECTED_ADDRESS (int)&IMAGE_ADDR[0]
#define FLASH_ADDR_OFFSET       0x44000000 //TODO, check that adddress.
#define First4Bytes             4

/* RAM locations for USB Buffers */
#define USB_BUFFER_START        0x20008400
#define MSD_BUFFER_SIZE         512
#define BDT_SIZE                16
#define ICP_BUFFER_SIZE         64

/* File types */
#define UNKNOWN                 0
#define RAW_BINARY              1
#define CODE_WARRIOR_BINARY     2
#define S19_RECORD              3

void _Entry(void) ;

#if (defined MCU_MK60N512VMD100)|| (defined MCU_MK20D7)
	extern long __SP_INIT;
#endif
	
/* Bootloader Status */
#define BootloaderReady         0
#define BootloaderS19Error      1
#define BootloaderFlashError    2
#define BootloaderSuccess       3
#define BootloaderStarted       4


#define  FLASH_IMAGE_SUCCESS    0
#define  FLASH_IMAGE_ERROR      1

#define BUFFER_LENGTH           (1024)    /* 1K bytes*/


uint_8           FlashApplication(uint_8* arr,uint_32 length);

#if (defined MCU_MK60N512VMD100)	||(defined MCU_MK20D7)
	extern int _startup(void);
#endif

#endif
/* EOF */
