/**
\brief Declaration of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
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
#define SERIAL_OUTPUT_BUFFER_SIZE 256 // leave at 256!


/**
\brief Number of output buffers for the serial.

*/

#define  OPENSERIAL_OUTPUT_NBBUFFERS        10


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
#define SERFRAME_MOTE2PC_SNIFFED_PACKET     ((uint8_t)'P')
#define SERFRAME_MOTE2PC_STAT               ((uint8_t)'T')
#define SERFRAME_MOTE2PC_PRINTF             ((uint8_t)'F')


// frames sent PC->mote
#define SERFRAME_PC2MOTE_SETROOT            ((uint8_t)'R')
#define SERFRAME_PC2MOTE_DATA               ((uint8_t)'D')
#define SERFRAME_PC2MOTE_TRIGGERSERIALECHO  ((uint8_t)'S')
#define SERFRAME_PC2MOTE_COMMAND_GD         ((uint8_t)'G')



enum{
   SERTYPE_DATA_GENERATION    = 0x01,
   SERTYPE_DATA_RX            = 0x02,
   SERTYPE_PKT_TX             = 0x03,
   SERTYPE_PKT_RX             = 0x04,
   SERTYPE_CELL_ADD           = 0x05,
   SERTYPE_CELL_REMOVE        = 0x06,
   SERTYPE_ACK_TX             = 0x07,
   SERTYPE_ACK_RX             = 0x08,
   SERTYPE_PKT_TIMEOUT        = 0x09,
   SERTYPE_PKT_ERROR          = 0x0a,
   SERTYPE_PKT_BUFFEROVERFLOW = 0x0b,
   SERTYPE_DIOTX              = 0x0c,
   SERTYPE_DAOTX              = 0x0d,
   SERTYPE_NODESTATE          = 0x0e,
};


//=========================== typedef =========================================

enum {
   COMMAND_SET_EBPERIOD          =  0,
   COMMAND_SET_CHANNEL           =  1,
   COMMAND_SET_KAPERIOD          =  2,
   COMMAND_SET_DIOPERIOD         =  3,
   COMMAND_SET_DAOPERIOD         =  4,
   COMMAND_PING_MOTE             =  5,
   COMMAND_SET_DAGRANK           =  6,
   COMMAND_SET_SECURITY_STATUS   =  7,
   COMMAND_SET_FRAMELENGTH       =  8,
   COMMAND_SET_ACK_STATUS        =  9,
   COMMAND_MAX                   = 10,
};




BEGIN_PACK
typedef struct{
   uint16_t    track_instance;
   uint8_t     track_owner[8];
   uint8_t     slotOffset;
   uint8_t     type;
   uint8_t     shared;
   uint8_t     channelOffset;
   uint8_t     neighbor[8];
} evtCellAdd_t;
END_PACK

BEGIN_PACK
typedef struct{
   uint16_t    track_instance;
   uint8_t     track_owner[8];
   uint8_t     slotOffset;
   uint8_t     type;
   uint8_t     shared;
   uint8_t     channelOffset;
   uint8_t     neighbor[8];
} evtCellRem_t;
END_PACK

BEGIN_PACK
typedef struct{
   uint16_t    track_instance;
   uint8_t     track_owner[8];
   uint32_t    seqnum;
   uint8_t     l3Source[8];
   uint8_t     l3Dest[8];
} evtPktData_t;
END_PACK

BEGIN_PACK
typedef struct{
   uint16_t    track_instance;
   uint8_t     track_owner[8];
   uint8_t     length;
   uint8_t     frame_type;
   slotOffset_t slotOffset;
   uint8_t     frequency;
   uint8_t     l2Dest[8];
   uint8_t     txPower;
   uint8_t     numTxAttempts;
   uint8_t     l4_protocol;
   uint16_t    l4_sourcePortORicmpv6Type;
   uint16_t    l4_destination_port;
   uint8_t     l3Source[16];
   uint8_t     l3Dest[16];
} evtPktTx_t;
END_PACK

BEGIN_PACK
typedef struct{
   uint16_t    track_instance;
   uint8_t     track_owner[8];
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
   uint8_t     parent[8];     //parent (=next hop) when the DAO was transmitted
}evtDaOTx_t;
END_PACK

BEGIN_PACK
typedef struct{
   uint32_t    numTicsOn;
   uint32_t    numTicsTotal;
   uint8_t     numDeSync;
}evtState;
END_PACK


BEGIN_PACK
typedef struct {
   uint8_t         track_mgmt;
   uint8_t         distr_cells;
   uint8_t         rpl_metric;
   uint8_t         scheduling_algo;
   uint32_t        cexample_period;
} debugParamsEntry_t;
END_PACK


//=========================== module variables ================================

typedef struct {
   // admin
   uint8_t    mode;
   uint8_t    debugPrintCounter;
   // input
   uint8_t    reqFrame[1+1+2+1]; // flag (1B), command (2B), CRC (2B), flag (1B)
   uint8_t    reqFrameIdx;
   uint8_t    lastRxByte;
   bool       busyReceiving;
   bool       inputEscaping;
   uint16_t   inputCrc;
   uint8_t    inputBufFill;
   uint8_t    inputBuf[SERIAL_INPUT_BUFFER_SIZE];
   // output (we have [OPENSERIAL_OUTPUT_NBBUFFERS] buffers, flushed and filled iteratively
   uint8_t    outputCurrentR;   //current buffer id to read and push to serial
   uint8_t    outputCurrentW;   //current buffer id to write our data
   bool       outputBufFilled[OPENSERIAL_OUTPUT_NBBUFFERS];
   uint16_t   outputCrc[OPENSERIAL_OUTPUT_NBBUFFERS];
   uint8_t    outputBufIdxW[OPENSERIAL_OUTPUT_NBBUFFERS];
   uint8_t    outputBufIdxR[OPENSERIAL_OUTPUT_NBBUFFERS];
   uint8_t    outputBuf[OPENSERIAL_OUTPUT_NBBUFFERS][SERIAL_OUTPUT_BUFFER_SIZE];
} openserial_vars_t;

//=========================== prototypes ======================================

void    openserial_init(void);
owerror_t openserial_printStat(uint8_t type, uint8_t calling_component, uint8_t *buffer, uint8_t length);
owerror_t openserial_printf(uint8_t calling_component, char* buffer, uint8_t length);
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
owerror_t openserial_printPacket(uint8_t* buffer, uint8_t length, uint8_t channel);
uint8_t openserial_getNumDataBytes(void);
uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes);
void    openserial_startInput(void);
void    openserial_startOutput(void);
void    openserial_stop(void);
bool    debugPrint_outBufferIndexes(void);
bool    debugPrint_params(void);
void    openserial_echo(uint8_t* but, uint8_t bufLen);

// interrupt handlers
void    isr_openserial_rx(void);
void    isr_openserial_tx(void);


//statistics to openvisualizer
void  openserial_statCelladd(scheduleEntry_t* slotContainer);
void  openserial_statCellremove(scheduleEntry_t* slotContainer);
void  openserial_statAckTx(void);
void  openserial_statAckRx(void);
void  openserial_statRxCrcFalse(OpenQueueEntry_t* msg);
void  openserial_statRx(OpenQueueEntry_t* msg);
void  openserial_statTx(OpenQueueEntry_t* msg);
void  openserial_statPktTimeout(OpenQueueEntry_t* msg);
void  openserial_statPktBufferOverflow(OpenQueueEntry_t* msg);
void  openserial_statDataGen(uint32_t seqnum, track_t *track, open_addr_t *src, open_addr_t *dest);
void  openserial_statDataRx(uint32_t seqnum, track_t *track, open_addr_t *src, open_addr_t *dest);
void  openserial_statDIOtx(void);
void  openserial_statDAOtx(uint8_t *parent);

// -- tools
//append a uint8_t at the end of a string
char *openserial_ncat_uint32_t(char *str, uint32_t val, uint8_t length);
char *openserial_ncat_uint8_t_hex(char *str, uint8_t val, uint8_t length);



uint8_t openserial_getInputBufferFilllevel(void);

/**
\}
\}
*/

#endif
