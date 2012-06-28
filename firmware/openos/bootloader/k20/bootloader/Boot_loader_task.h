/**
\brief Include the OS and BSP dependent files that define IO functions and
 basic types. You may like to change these files for your board and RTOS.  Based on Freescale bootloader demo for k60. 

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012.
 */

#include "derivative.h"
uint_8          erase_flash(void);
void            Switch_mode(void); 
void            SetOutput(uint_32 output,boolean state); 
void            GPIO_Bootloader_Init(void);
