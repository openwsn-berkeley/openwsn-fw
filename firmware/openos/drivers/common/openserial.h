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
*/
#define SERIAL_OUTPUT_BUFFER_SIZE 300

/**
\brief Number of bytes of the serial input buffer, in bytes.

\warning Do not pick a number greater than 255, since its filling level is
         encoded by a single byte in the code.
*/
#define SERIAL_INPUT_BUFFER_SIZE  200

/// Modes of the openserial module.
enum {
   MODE_OFF    = 0, ///< The module is off, no serial activity.
   MODE_INPUT  = 1, ///< The serial is listening or receiving bytes.
   MODE_OUTPUT = 2  ///< The serial is transmitting bytes.
};

// frames sent mote->PC
#define SERFRAME_MOTE2PC_DATA               ((uint8_t)'D')
#define SERFRAME_MOTE2PC_STATUS             ((uint8_t)'S')
#define SERFRAME_MOTE2PC_INFO               ((uint8_t)'I')
#define SERFRAME_MOTE2PC_ERROR              ((uint8_t)'E')
#define SERFRAME_MOTE2PC_CRITICAL           ((uint8_t)'C')
#define SERFRAME_MOTE2PC_REQUEST            ((uint8_t)'R')

// frames sent PC->mote
#define SERFRAME_PC2MOTE_SETROOT            ((uint8_t)'R')
#define SERFRAME_PC2MOTE_SETBRIDGE          ((uint8_t)'B')
#define SERFRAME_PC2MOTE_DATA               ((uint8_t)'D')
#define SERFRAME_PC2MOTE_TRIGGERTCPINJECT   ((uint8_t)'T')
#define SERFRAME_PC2MOTE_TRIGGERUDPINJECT   ((uint8_t)'U')
#define SERFRAME_PC2MOTE_TRIGGERICMPv6ECHO  ((uint8_t)'E')
#define SERFRAME_PC2MOTE_TRIGGERSERIALECHO  ((uint8_t)'S')

//=========================== typedef =========================================

//=========================== prototypes ======================================

void    openserial_init();
error_t openserial_printStatus(uint8_t statusElement, uint8_t* buffer, uint16_t length);
error_t openserial_printInfo(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
error_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
error_t openserial_printCritical(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
error_t openserial_printData(uint8_t* buffer, uint8_t length);
uint8_t openserial_getNumDataBytes();
uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes);
void    openserial_startInput();
void    openserial_startOutput();
void    openserial_stop();
bool    debugPrint_outBufferIndexes();


// interrupt handlers
void    isr_openserial_rx();
void    isr_openserial_tx();

/**
\}
\}
*/

#endif