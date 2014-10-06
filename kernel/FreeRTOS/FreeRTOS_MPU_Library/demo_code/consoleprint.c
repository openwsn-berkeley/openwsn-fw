//*****************************************************************************
//   +--+
//   | ++----+
//   +-++    |
//     |     |
//   +-+--+  |
//   | +--+--+
//   +----+    Copyright (c) 2009 Code Red Technologies Ltd.
//
// consoleprint.c - provides a "print string to console" function that uses
//                  the CodeRed semihosting debug channel functionality.
//
// Software License Agreement
//
// The software is owned by Code Red Technologies and/or its suppliers, and is
// protected under applicable copyright laws.  All rights are reserved.  Any
// use in violation of the foregoing restrictions may subject the user to criminal
// sanctions under applicable laws, as well as to civil liability for the breach
// of the terms and conditions of this license.
//
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// USE OF THIS SOFTWARE FOR COMMERCIAL DEVELOPMENT AND/OR EDUCATION IS SUBJECT
// TO A CURRENT END USER LICENSE AGREEMENT (COMMERCIAL OR EDUCATIONAL) WITH
// CODE RED TECHNOLOGIES LTD.
//
//*****************************************************************************

#include <stdio.h>
#include <string.h>
#include "consoleprint.h"

/*
 * Prototype for Code Red library stub function that carries out
 * semihosting "SYS_WRITE" operation. Note that Code Red makes no
 * guarantees that the interface to this stub function will not
 * change in future versions of the tools.
 */
#if (defined(__NEWLIB__))
#define LIBSTUB_SYS_WRITE _swiwrite
#else // __REDLIB__
#define LIBSTUB_SYS_WRITE __write
#endif
int LIBSTUB_SYS_WRITE (int, char *, int);

/*
 * consoleprint() -  provides a "print string to console" function that uses
 * the CodeRed semihosting debug channel functionality. Because this can
 * send a full string in one operation, rather than a single character (as
 * using printf will) this will provide faster prints to the console.
 *
 * Input - pointer to zero-terminated character string
 * Returns 0 if successful
 *
 * Note that in order to consoleprint() to actually display to the console,
 * the semihosting interface using stdout must have been setup. The easiest
 * way to do this is to call printf() once at the start of your application
 */
int consoleprint(char *cpstring)
{
	 int slen, res;
	 // Calculate length of string
	 slen = strlen (cpstring);
	 // Call function to carry out semihosting "SYS_WRITE" operation
	 res = LIBSTUB_SYS_WRITE (0, cpstring,slen);	// Note that '0' represents stdout
	 return res;
}
