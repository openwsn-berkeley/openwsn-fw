/**
\brief Declaration of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/

#ifndef OPENWSN_OPENSERIAL_H
#define OPENWSN_OPENSERIAL_H

#include "config.h"
#include "opendefs.h"

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
#define SERFRAME_MOTE2PC_VERBOSE                 ((uint8_t)'V')
#define SERFRAME_MOTE2PC_INFO                    ((uint8_t)'I')
#define SERFRAME_MOTE2PC_WARNING                 ((uint8_t)'W')
#define SERFRAME_MOTE2PC_SUCCESS                 ((uint8_t)'U')
#define SERFRAME_MOTE2PC_ERROR                   ((uint8_t)'E')
#define SERFRAME_MOTE2PC_CRITICAL                ((uint8_t)'C')
#define SERFRAME_MOTE2PC_SNIFFED_PACKET          ((uint8_t)'P')
#define SERFRAME_MOTE2PC_PRINTF                  ((uint8_t)'F')

// frames sent PC->mote
#define SERFRAME_PC2MOTE_SETROOT                 ((uint8_t)'R')
#define SERFRAME_PC2MOTE_RESET                   ((uint8_t)'Q')
#define SERFRAME_PC2MOTE_DATA                    ((uint8_t)'D')
#define SERFRAME_PC2MOTE_TRIGGERSERIALECHO       ((uint8_t)'S')

//=========================== macros =========================================

#ifndef OPENWSN_DEBUG_LEVEL
#define OPENWSN_DEBUG_LEVEL     4
#endif

#if (OPENWSN_DEBUG_LEVEL >= 6)
#define LOG_VERBOSE(component, message, p1, p2)   openserial_printLog(VERBOSE, (component), (message), (p1), (p2))
#else
#define LOG_VERBOSE(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 5)
#define LOG_INFO(component, message, p1, p2)   openserial_printLog(INFO, (component), (message), (p1), (p2))
#else
#define LOG_INFO(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 4)
#define LOG_WARNING(component, message, p1, p2)   openserial_printLog(WARNING, (component), (message), (p1), (p2))
#else
#define LOG_WARNING(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 3)
#define LOG_SUCCESS(component, message, p1, p2)   openserial_printLog(SUCCESS, (component), (message), (p1), (p2))
#else
#define LOG_SUCCESS(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 2)
#define LOG_ERROR(component, message, p1, p2)   openserial_printLog(ERROR, (component), (message), (p1), (p2))
#else
#define LOG_ERROR(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 1)
#define LOG_CRITICAL(component, message, p1, p2)   openserial_printLog(CRITICAL, (component), (message), (p1), (p2))
#else
#define LOG_CRITICAL(component, message, p1, p2)
#endif
//=========================== typedef =========================================

enum {
    CRITICAL = 1,
    ERROR = 2,
    SUCCESS = 3,
    WARNING = 4,
    INFO = 5,
    VERBOSE = 6
};

//=========================== variables =======================================

//=========================== prototypes ======================================

typedef struct {
    // admin
    uint8_t fInhibited;
    uint8_t ctsStateChanged;
    uint8_t debugPrintCounter;
    uint8_t reset_timerId;
    uint8_t debugPrint_timerId;
    // input
    uint8_t inputBuf[SERIAL_INPUT_BUFFER_SIZE];
    uint8_t inputBufFillLevel;
    uint8_t hdlcLastRxByte;
    bool hdlcBusyReceiving;
    uint16_t hdlcInputCrc;
    bool hdlcInputEscaping;
    // output
    uint8_t outputBuf[SERIAL_OUTPUT_BUFFER_SIZE];
    uint16_t outputBufIdxW;
    uint16_t outputBufIdxR;
    bool fBusyFlushing;
    uint16_t hdlcOutputCrc;
} openserial_vars_t;

// admin
void openserial_init(void);

// transmitting
owerror_t openserial_printStatus(
        uint8_t statusElement,
        uint8_t *buffer,
        uint8_t length
);

owerror_t openserial_printLog(
        uint8_t log_level,
        uint8_t calling_component,
        uint8_t error_code,
        errorparameter_t arg1,
        errorparameter_t arg2
);

owerror_t openserial_printData(uint8_t *buffer, uint8_t length);

owerror_t openserial_printSniffedPacket(uint8_t *buffer, uint8_t length, uint8_t channel);

void task_openserial_debugPrint(void);

owerror_t openserial_printf(char *buffer, ...);

// receiving
uint8_t openserial_getInputBufferFillLevel(void);

uint8_t openserial_getInputBuffer(uint8_t *bufferToWrite, uint8_t maxNumBytes);

// scheduling
void openserial_flush(void);

void openserial_inhibitStart(void);

void openserial_inhibitStop(void);

// debugprint
bool debugPrint_outBufferIndexes(void);

// interrupt handlers
uint8_t isr_openserial_rx(void);

void isr_openserial_tx(void);

/**
\}
\}
*/

#endif
