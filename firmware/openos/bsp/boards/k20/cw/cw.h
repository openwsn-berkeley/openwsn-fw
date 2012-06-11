/*
 * File:	cw.h
 * Purpose:	Define constants used by CodeWarrior toolchain
 *
 * Notes:	
 *
 */

#ifndef _CW_H_
#define _CW_H_

/********************************************************************/
/* 
 * The source uses __interrupt__ to identify a function as
 * an interrupt or exception handler.  Codewarrior uses 
 * __declspec(interrupt), so we are appeasing it like this.
 */
#define __interrupt__	__declspec(interrupt)

/* Define custom section to force inclusion of vector table by linker */


/* 
 * Define custom sections for relocating code, data, and constants 
 */
#pragma define_section relocate_code ".relocate_code" ".relocate_code" ".relocate_code" far_abs RX
//#pragma define_section relocate_code ".relocate_code" ".relocate_code" ".relocate_code" abs32 RX
//#pragma define_section relocate_code ".relocate_code" far_absolute RX
#pragma define_section relocate_data ".relocate_data" ".relocate_data" ".relocate_data" RW
#pragma define_section relocate_const ".relocate_const" ".relocate_const" ".relocate_const" far_abs R
#define __relocate_code__   __declspec(relocate_code)
#define __relocate_data__   __declspec(relocate_data)
#define __relocate_const__  __declspec(relocate_const)
/********************************************************************/

#endif /* _CW_H_ */
