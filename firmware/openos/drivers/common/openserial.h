/**
\brief Declaration of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
*/

#ifndef __OPENSERIAL_H
#define __OPENSERIAL_H

#include "openwsn.h"

/**
\addtogroup cross-layers
\{
\addtogroup OpenSerial
\{
*/

//=========================== define ==========================================

/**
\brief Number of bytes of the serial output buffer, in bytes.

\warning should be exactly 256 so wrap-around on the index does not require
         the use of a slow modulo operator.
*/
#define SERIAL_OUTPUT_BUFFER_SIZE 256 // leave at 256!

/**
\brief Number of bytes of the serial input buffer, in bytes.

\warning Do not pick a number greater than 255, since its filling level is
         encoded by a single byte in the code.
*/
#define SERIAL_INPUT_BUFFER_SIZE  200

//=========================== typedef =========================================

//=========================== prototypes ======================================

void    openserial_init();
owerror_t openserial_printStatus(uint8_t statusElement, uint8_t* buffer, uint8_t length);
owerror_t openserial_printInfo(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
owerror_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
owerror_t openserial_printCritical(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
owerror_t openserial_printData(uint8_t* buffer, uint8_t length);
uint8_t openserial_getNumDataBytes();
uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes);
void    openserial_startInput();
void    openserial_startOutput();
void    openserial_stop();
bool    debugPrint_outBufferIndexes();
void    openserial_echo(uint8_t* but, uint8_t bufLen);

// interrupt handlers
void    isr_openserial_rx();
void    isr_openserial_tx();

/**
\}
\}
*/

#endif