/*
 * File:		io.c
 * Purpose:		Serial Input/Output routines
 *
 * Notes:       TERMINAL_PORT defined in <board>.h
 */

#include "common.h"
#include "uart.h"

/********************************************************************/
char
in_char (void)
{
//	return uart_getchar(TERM_PORT);
}
/********************************************************************/
void
out_char (char ch)
{
//	uart_putchar(TERM_PORT, ch);
}
/********************************************************************/
int
char_present (void)
{
	//return uart_getchar_present(TERM_PORT);
}
/********************************************************************/
