/**
\brief Declaration of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/

#ifndef __OPENSERIAL_H
#define __OPENSERIAL_H

#include "opendefs.h"
#include "schedule.h"

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

/// Modes of the openserial module.
enum {
   MODE_OFF    = 0, ///< The module is off, no serial activity.
   MODE_INPUT  = 1, ///< The serial is listening or receiving bytes.
   MODE_OUTPUT = 2  ///< The serial is transmitting bytes.
};

// frames sent mote->PC
#define SERFRAME_MOTE2PC_DATA                    ((uint8_t)'D')
#define SERFRAME_MOTE2PC_STATUS                  ((uint8_t)'S')
#define SERFRAME_MOTE2PC_INFO                    ((uint8_t)'I')
#define SERFRAME_MOTE2PC_ERROR                   ((uint8_t)'E')
#define SERFRAME_MOTE2PC_CRITICAL                ((uint8_t)'C')
#define SERFRAME_MOTE2PC_REQUEST                 ((uint8_t)'R')
#define SERFRAME_MOTE2PC_SNIFFED_PACKET          ((uint8_t)'P')
#define SERFRAME_MOTE2PC_STAT                    ((uint8_t)'T')

// frames sent PC->mote
#define SERFRAME_PC2MOTE_SETROOT                 ((uint8_t)'R')
#define SERFRAME_PC2MOTE_RESET                   ((uint8_t)'Q')
#define SERFRAME_PC2MOTE_DATA                    ((uint8_t)'D')
#define SERFRAME_PC2MOTE_TRIGGERSERIALECHO       ((uint8_t)'S')
#define SERFRAME_PC2MOTE_COMMAND                 ((uint8_t)'C')
#define SERFRAME_PC2MOTE_TRIGGERUSERIALBRIDGE    ((uint8_t)'B')

//for the schedule
#define OPENSERIALMAXNUMCELLS   5
#define SCHEDULEIEMAXNUMCELLS   3

//=========================== typedef =========================================

enum{
    SERTYPE_PKT_TX             = 0x01,
    SERTYPE_PKT_RX             = 0x02,
    SERTYPE_CELL               = 0x03,
    SERTYPE_ACK                = 0x04,
    SERTYPE_PKT_DROPPED        = 0x05,
    SERTYPE_DIO                = 0x06,
    SERTYPE_DAO                = 0x07,
    SERTYPE_NODESTATE          = 0x08,
    SERTYPE_6PCMD              = 0x09,
};

enum{
    CELLADD = 1,
    CELLDEL = 2
};


BEGIN_PACK
typedef struct{
    uint8_t     code;
    uint8_t     type;
    uint8_t     shared;
    uint8_t     slotOffset;
    uint8_t     channelOffset;
    uint8_t     neighbor[8];
} evtCell_t;
END_PACK

BEGIN_PACK
typedef struct{
    uint8_t     code;        // RX or TX
    uint8_t     l2addr[8];
}evtAck_t;
END_PACK

BEGIN_PACK
typedef struct{
    uint8_t     length;
    uint8_t     frame_type;
    slotOffset_t slotOffset;
    uint8_t     frequency;
    uint8_t     l2Dest[8];
    uint8_t     txPower;
    uint8_t     numTxAttempts;
    uint8_t     l3_destinationAdd[16];
    uint8_t     l3_sourceAdd[16];
} evtPktTx_t;
END_PACK

BEGIN_PACK
typedef struct{
    uint8_t     length;
    uint8_t     frame_type;
    slotOffset_t slotOffset;
    uint8_t     frequency;
    uint8_t     l2Src[8];
    uint8_t     rssi;
    uint8_t     lqi;
    uint8_t     crc;
} evtPktRx_t;
END_PACK

BEGIN_PACK
typedef struct{
    uint8_t     status;         //enqueued, TXED, RCVD, etc.
    uint8_t     rplinstanceId;
    dagrank_t   rank;
    uint8_t     DODAGID[16];
}evtDIO_t;
END_PACK

BEGIN_PACK
typedef struct{
    uint8_t     status;
    uint8_t     parent[8];     //parent (=next hop) when the DAO was transmitted
    uint8_t     DODAGID[16];
}evtDAO_t;
END_PACK

// the 6P command types
enum{
    CELLADD_REQ = 1,
    CELLADD_REP = 2,
    CELLDEL_REQ = 3,
    CELLDEL_REP = 4,
    CELLLIST_REP = 5,
    CELLLIST_REQ = 6,
    CELLCLEAR_REP = 7,
    CELLCLEAR_REQ = 8,
    CELLCOUNT_REP = 9,
    CELLCOUNT_REQ = 10
};

// type of events for ach 6P command
enum{                   //status of the 6P command
    ENQUEUED  = 1,
    TXED      = 2,
    RCVD      = 3,
    FAILED    = 4
};

BEGIN_PACK
typedef struct{
    uint8_t     sixtop_command;  // type of 6Pcommand (cell add/del req/rep)
    uint8_t     status;          // result of the operation
    uint8_t     neighbor[8];     // parent (=next hop) when the DAO was transmitted
    cellInfo_ht  cells[OPENSERIALMAXNUMCELLS];       //up to OPENSERIALMAXNUMCELLS, cut with the correct size
}evt6PCmd_t;
END_PACK

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
    uint8_t             mode;
    uint8_t             debugPrintCounter;
    openserial_rsvpt*   registeredCmd;
    // input
    uint8_t             reqFrame[1+1+2+1]; // flag (1B), command (2B), CRC (2B), flag (1B)
    uint8_t             reqFrameIdx;
    uint8_t             lastRxByte;
    bool                busyReceiving;
    bool                inputEscaping;
    uint16_t            inputCrc;
    uint8_t             inputBufFill;
    uint8_t             inputBuf[SERIAL_INPUT_BUFFER_SIZE];
    // output
    bool                outputBufFilled;
    uint16_t            outputCrc;
    uint16_t            outputBufIdxW;
    uint16_t            outputBufIdxR;
    uint8_t             outputBuf[SERIAL_OUTPUT_BUFFER_SIZE];
} openserial_vars_t;

// admin
void      openserial_init(void);
void      openserial_register(openserial_rsvpt* rsvp);

// printing
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
owerror_t openserial_printData(uint8_t* buffer, uint8_t length);
owerror_t openserial_printSniffedPacket(uint8_t* buffer, uint8_t length, uint8_t channel);

//statistics to openvisualizer
owerror_t openserial_printStat(uint8_t type, uint8_t *buffer, uint8_t length);
void    openserial_statCell(uint8_t status, scheduleEntry_t* slotContainer);
void    openserial_statAck(uint8_t status, open_addr_t *l2_addr);
void    openserial_statRx(OpenQueueEntry_t* msg);
void    openserial_statTx(OpenQueueEntry_t* msg);
void    openserial_statPktDropped(uint8_t status, OpenQueueEntry_t* msg);
void    openserial_statDIO(uint8_t status, uint8_t rplinstanceId, dagrank_t rank, uint8_t *DODAGID);
void    openserial_statDAO(uint8_t status, uint8_t *parent, uint8_t *DODAGID);
void    openserial_stat6Pcmd(uint8_t command, uint8_t status, open_addr_t *neigh, cellInfo_ht* cells, uint8_t nbCells);

// retrieving inputBuffer
uint8_t   openserial_getInputBufferFilllevel(void);
uint8_t   openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes);

// scheduling
void      openserial_startInput(void);
void      openserial_startOutput(void);
void      openserial_stop(void);

// debugprint
bool      debugPrint_outBufferIndexes(void);

// interrupt handlers
void      isr_openserial_rx(void);
void      isr_openserial_tx(void);

/**
\}
\}
*/

#endif
