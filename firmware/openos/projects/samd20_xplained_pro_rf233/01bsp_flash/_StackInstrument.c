/**
 *  \brief Stack checker extension utility file.
 *
 *  This file has been automatically added to your project by the Stack Checker
 *  Atmel Studio extension. If you are not currently using this extension, this
 *  file may be removed from your project.
 */

#include <stdint.h>

#ifndef __GNUC__
#  error The stack instrumentation code is designed for GCC toolchains only.
#endif

#ifndef __AVR__
#  error The stack instumentation code is intended for AVR 8-bit targets only.
#endif

/** Linker provided symbols for the end of the static data section, and the
 *  of the stack.
 */
extern void *_end, *__stack;

/** Aligns a given memory byte address to the nearest 32-bit address, rounding downwards. */
#define __ALIGN32_DOWNWARDS(x) ((uintptr_t)(x) - ((uintptr_t)(x) & 0x03))

/** Aligns a given memory byte address to the nearest 32-bit address, rounding upwards. */
#define __ALIGN32_UPWARDS(x)   ((uintptr_t)(x) + ((uintptr_t)(x) & 0x03))

/** Swaps the endianness of a given 16-bit value. */
#define __SWAP_ENDIAN16(x)     ((( (x) & 0xFF00) >> 8) | (( (x) & 0x00FF) << 8))

/** Swaps the endianness of a given 32-bit value. */
#define __SWAP_ENDIAN32(x)     (__SWAP_ENDIAN16(( (x) & 0xFFFF0000UL) >> 16) | (__SWAP_ENDIAN16( (x) & 0x0000FFFFUL) << 16))

/** \internal
 *  \brief Low level stack painting function, hooked into the avr-libc initialization code.
 *
 *  Paints the internal SRAM between the end of the static data and the start
 *  of the stack with a known \c 0xDEADBEEF hex pattern. This is then detected
 *  by the Stack Checker extension when a debug session is halted to determine
 *  the maximum stack usage of the running application.
 */
void _StackPaint(void) __attribute__((naked)) __attribute__((optimize("O3"))) __attribute__((section (".init1")));
void _StackPaint(void)
{
	uint32_t* fill_start = (uint32_t*)__ALIGN32_UPWARDS(&_end);
	uint32_t* fill_end   = (uint32_t*)__ALIGN32_DOWNWARDS(&__stack);
	
	for (uint32_t* fill_pos = fill_start; fill_pos < fill_end; fill_pos++)
		*fill_pos = __SWAP_ENDIAN32(0xDEADBEEF);
}
