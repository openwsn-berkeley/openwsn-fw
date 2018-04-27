/**
\brief Declaration of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/

#ifndef __OPENSERIAL_H
#define __OPENSERIAL_H

#include "opendefs.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
\addtogroup drivers
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
#define SERIAL_OUTPUT_BUFFER_SIZE 1024 // leave at 256!
#define OUTPUT_BUFFER_MASK       0x3FF

/**
\brief Number of bytes of the serial input buffer, in bytes.

\warning Do not pick a number greater than 255, since its filling level is
         encoded by a single byte in the code.
*/
#define SERIAL_INPUT_BUFFER_SIZE  200

// frames sent mote->PC
#define SERFRAME_MOTE2PC_DATA                    ((uint8_t)'D')
#define SERFRAME_MOTE2PC_STATUS                  ((uint8_t)'S')
#define SERFRAME_MOTE2PC_INFO                    ((uint8_t)'I')
#define SERFRAME_MOTE2PC_ERROR                   ((uint8_t)'E')
#define SERFRAME_MOTE2PC_CRITICAL                ((uint8_t)'C')
#define SERFRAME_MOTE2PC_SNIFFED_PACKET          ((uint8_t)'P')
#define SERFRAME_MOTE2PC_PRINTF                  ((uint8_t)'F')
#define SERFRAME_MOTE2PC_ACKREPLY                ((uint8_t)'A')

// frames sent PC->mote
#define SERFRAME_PC2MOTE_SETROOT                 ((uint8_t)'R')
#define SERFRAME_PC2MOTE_RESET                   ((uint8_t)'Q')
#define SERFRAME_PC2MOTE_DATA                    ((uint8_t)'D')
#define SERFRAME_PC2MOTE_TRIGGERSERIALECHO       ((uint8_t)'S')
#define SERFRAME_PC2MOTE_COMMAND                 ((uint8_t)'C')
#define SERFRAME_PC2MOTE_TRIGGERUSERIALBRIDGE    ((uint8_t)'B')

//=========================== typedef =========================================

enum {
    COMMAND_SET_EBPERIOD          =  0,
    COMMAND_SET_CHANNEL           =  1,
    COMMAND_SET_KAPERIOD          =  2,
    COMMAND_SET_DIOPERIOD         =  3,
    COMMAND_SET_DAOPERIOD         =  4,
    COMMAND_SET_DAGRANK           =  5,
    COMMAND_SET_SECURITY_STATUS   =  6,
    COMMAND_SET_SLOTFRAMELENGTH   =  7,
    COMMAND_SET_ACK_STATUS        =  8,
    COMMAND_SET_6P_ADD            =  9,
    COMMAND_SET_6P_DELETE         = 10,
    COMMAND_SET_6P_RELOCATE       = 11,
    COMMAND_SET_6P_COUNT          = 12,
    COMMAND_SET_6P_LIST           = 13,
    COMMAND_SET_6P_CLEAR          = 14,
    COMMAND_SET_SLOTDURATION      = 15,
    COMMAND_SET_6PRESPONSE        = 16,
    COMMAND_SET_UINJECTPERIOD     = 17,
    COMMAND_SET_ECHO_REPLY_STATUS = 18,
    COMMAND_SET_JOIN_KEY          = 19,
    COMMAND_MAX                   = 20,
};

//=========================== variables =======================================

//=========================== prototypes ======================================

typedef void (*openserial_cbt)(void);

typedef struct _openserial_rsvpt {
    uint8_t                       cmdId; ///< serial command (e.g. 'B')
    openserial_cbt                cb;    ///< handler of that command
    struct _openserial_rsvpt*     next;  ///< pointer to the next registered command
} openserial_rsvpt;

typedef struct {
    // admin
    uint8_t             fInhibited;
    uint8_t             ctsStateChanged;
    uint8_t             debugPrintCounter;
    openserial_rsvpt*   registeredCmd;
    // input
    uint8_t             inputBuf[SERIAL_INPUT_BUFFER_SIZE];
    uint8_t             inputBufFillLevel;
    uint8_t             hdlcLastRxByte;
    bool                hdlcBusyReceiving;
    uint16_t            hdlcInputCrc;
    bool                hdlcInputEscaping;
    // output
    uint8_t             outputBuf[SERIAL_OUTPUT_BUFFER_SIZE];
    uint16_t            outputBufIdxW;
    uint16_t            outputBufIdxR;
    bool                fBusyFlushing;
    uint16_t            hdlcOutputCrc;
} openserial_vars_t;

// admin
void openserial_init(void);
void openserial_register(openserial_rsvpt* rsvp);

// transmitting
owerror_t openserial_printStatus(
    uint8_t             statusElement,
    uint8_t*            buffer,
    uint8_t             length
);
owerror_t openserial_printInfo(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
);
owerror_t openserial_printError(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
);
owerror_t openserial_printCritical(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
);
owerror_t openserial_printData(
    uint8_t*            buffer,
    uint8_t             length
);
owerror_t openserial_printSniffedPacket(
    uint8_t*            buffer,
    uint8_t             length,
    uint8_t             channel
);

void      task_openserial_debugPrint(void);

owerror_t openserial_print_str(char* buffer, uint8_t length);
owerror_t openserial_print_uint32_t(uint32_t value);

// receiving
uint8_t   openserial_getInputBufferFillLevel(void);
uint8_t   openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes);

// scheduling
void      openserial_flush(void);
void      openserial_inhibitStart(void);
void      openserial_inhibitStop(void);

// debugprint
bool      debugPrint_outBufferIndexes(void);

// interrupt handlers
uint8_t   isr_openserial_rx(void);
void      isr_openserial_tx(void);

/**
\}
\}
*/

#endif
