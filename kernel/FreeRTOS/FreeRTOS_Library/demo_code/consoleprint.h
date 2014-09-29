//*****************************************************************************
//   +--+
//   | ++----+
//   +-++    |
//     |     |
//   +-+--+  |
//   | +--+--+
//   +----+    Copyright (c) 2009 Code Red Technologies Ltd.
//
// consoleprint.h - provides a "print string to console" function that uses
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

#ifndef CONSOLEPRINT_H_
#define CONSOLEPRINT_H_

//#if !(defined(__REDLIB__))
//#error "consoleprint.c can only be built for projects built with Redlib"
//#endif

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
int consoleprint(char *cpstring);

#endif /* LED_CONFIG_H_ */
