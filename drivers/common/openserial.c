/**
\brief Definition of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "opendefs.h"
#include "openserial.h"
#include "IEEE802154E.h"
#include "neighbors.h"
#include "sixtop.h"
#include "icmpv6echo.h"
#include "idmanager.h"
#include "openqueue.h"
#include "openbridge.h"
#include "leds.h"
#include "schedule.h"
#include "uart.h"
#include "opentimers.h"
#include "openhdlc.h"
#include "schedule.h"
#include "icmpv6rpl.h"
#include "icmpv6echo.h"
#include "msf.h"



//=========================== variables =======================================

openserial_vars_t openserial_vars;

enum{
    TYPE_STR = 0,       //subtype for the printf message
    TYPE_INT = 1
};
//=========================== prototypes ======================================

// printing
owerror_t openserial_printInfoErrorCritical(
    char             severity,
    uint8_t          calling_component,
    uint8_t          error_code,
    errorparameter_t arg1,
    errorparameter_t arg2
);

// command handlers
void openserial_handleEcho(uint8_t* but, uint8_t bufLen);
void openserial_get6pInfo(uint8_t commandId, uint8_t* code,uint8_t* cellOptions,uint8_t* numCells,cellInfo_ht* celllist_add,cellInfo_ht* celllist_delete,uint8_t* listOffset,uint8_t* maxListLen,uint8_t ptr, uint8_t commandLen);
void openserial_handleCommands(void);

// misc
void openserial_board_reset_cb(opentimers_id_t id);

// HDLC output
void outputHdlcOpen(void);
void outputHdlcWrite(uint8_t b);
void outputHdlcClose(void);

// HDLC input
void inputHdlcOpen(void);
void inputHdlcWrite(uint8_t b);
void inputHdlcClose(void);

//=========================== public ==========================================

//===== admin

void openserial_init(void) {
    uint16_t crc;

    // reset variable
    memset(&openserial_vars,0,sizeof(openserial_vars_t));

    // admin
    openserial_vars.mode               = MODE_OFF;
    openserial_vars.debugPrintCounter  = 0;

    // input
    openserial_vars.reqFrame[0]        = HDLC_FLAG;
    openserial_vars.reqFrame[1]        = SERFRAME_MOTE2PC_REQUEST;
    crc = HDLC_CRCINIT;
    crc = crcIteration(crc,openserial_vars.reqFrame[1]);
    crc = ~crc;
    openserial_vars.reqFrame[2]        = (crc>>0)&0xff;
    openserial_vars.reqFrame[3]        = (crc>>8)&0xff;
    openserial_vars.reqFrame[4]        = HDLC_FLAG;
    openserial_vars.reqFrameIdx        = 0;
    openserial_vars.lastRxByte         = HDLC_FLAG;
    openserial_vars.busyReceiving      = FALSE;
    openserial_vars.inputEscaping      = FALSE;
    openserial_vars.inputBufFill       = 0;

    // ouput
    openserial_vars.outputBufFilled    = FALSE;
    openserial_vars.outputBufIdxR      = 0;
    openserial_vars.outputBufIdxW      = 0;

    // set callbacks
    uart_setCallbacks(
        isr_openserial_tx,
        isr_openserial_rx
    );
}

void openserial_register(openserial_rsvpt* rsvp) {
    // FIXME: register multiple commands (linked list)
    openserial_vars.registeredCmd = rsvp;
}

//===== printing

owerror_t openserial_printStatus(
    uint8_t             statusElement,
    uint8_t*            buffer,
    uint8_t             length
) {
    uint8_t i;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_STATUS);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(statusElement);
    for (i=0;i<length;i++){
        outputHdlcWrite(buffer[i]);
    }
    outputHdlcClose();
    ENABLE_INTERRUPTS();

    return E_SUCCESS;
}

owerror_t openserial_printInfo(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
) {
    return openserial_printInfoErrorCritical(
        SERFRAME_MOTE2PC_INFO,
        calling_component,
        error_code,
        arg1,
        arg2
    );
}

owerror_t openserial_printError(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
) {
    // toggle error LED
    leds_error_toggle();

    return openserial_printInfoErrorCritical(
        SERFRAME_MOTE2PC_ERROR,
        calling_component,
        error_code,
        arg1,
        arg2
    );
}

owerror_t openserial_printCritical(
    uint8_t             calling_component,
    uint8_t             error_code,
    errorparameter_t    arg1,
    errorparameter_t    arg2
) {
    opentimers_id_t id;
    uint32_t         reference;

    // blink error LED, this is serious
    leds_error_blink();

    // schedule for the mote to reboot in 10s
    id        = opentimers_create();
    reference = opentimers_getValue();
    opentimers_scheduleAbsolute(
        id,                             // timerId
        10000,                          // duration
        reference,                      // reference
        TIME_MS,                        // timetype
        openserial_board_reset_cb       // callback
    );

    return openserial_printInfoErrorCritical(
        SERFRAME_MOTE2PC_CRITICAL,
        calling_component,
        error_code,
        arg1,
        arg2
    );
}

owerror_t openserial_printData(uint8_t* buffer, uint8_t length) {
    uint8_t  i;
    uint8_t  asn[5];
    INTERRUPT_DECLARATION();

    // retrieve ASN
    ieee154e_getAsn(asn);

    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_DATA);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(asn[0]);
    outputHdlcWrite(asn[1]);
    outputHdlcWrite(asn[2]);
    outputHdlcWrite(asn[3]);
    outputHdlcWrite(asn[4]);
    for (i=0;i<length;i++){
        outputHdlcWrite(buffer[i]);
    }
    outputHdlcClose();
    ENABLE_INTERRUPTS();

    return E_SUCCESS;
}

owerror_t openserial_printSniffedPacket(uint8_t* buffer, uint8_t length, uint8_t channel) {
    uint8_t  i;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_SNIFFED_PACKET);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    for (i=0;i<length;i++){
       outputHdlcWrite(buffer[i]);
    }
    outputHdlcWrite(channel);
    outputHdlcClose();

    ENABLE_INTERRUPTS();

    return E_SUCCESS;
}

owerror_t openserial_print_uint32_t(uint32_t value) {
#ifdef OPENSERIAL_PRINTF
   uint8_t  i;
   uint8_t  pvalue[4];
   INTERRUPT_DECLARATION();

   DISABLE_INTERRUPTS();
   openserial_vars.outputBufFilled  = TRUE;
   outputHdlcOpen();
   outputHdlcWrite(SERFRAME_MOTE2PC_PRINTF);
   outputHdlcWrite(TYPE_INT);
   memcpy(pvalue, &value, 4);
   for (i=0;i<4;i++)
      outputHdlcWrite(pvalue[i]);
   outputHdlcClose();
   ENABLE_INTERRUPTS();

#endif

   return E_SUCCESS;
}

owerror_t openserial_print_str(char* buffer, uint8_t length) {
#ifdef OPENSERIAL_PRINTF
   uint8_t  i;
   uint8_t  asn[5];
   INTERRUPT_DECLARATION();

   // retrieve ASN
   ieee154e_getAsn(asn);

   DISABLE_INTERRUPTS();
   openserial_vars.outputBufFilled  = TRUE;
   outputHdlcOpen();
   outputHdlcWrite(SERFRAME_MOTE2PC_PRINTF);
   outputHdlcWrite(TYPE_STR);
   outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
   outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
   for(i=0; i<5;i++)
       outputHdlcWrite(asn[i]);
   for (i=0;i<length;i++)
      outputHdlcWrite(buffer[i]);
   outputHdlcClose();
   ENABLE_INTERRUPTS();

#endif

   return E_SUCCESS;
}

//===== retrieving inputBuffer

uint8_t openserial_getInputBufferFilllevel(void) {
    uint8_t inputBufFill;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    inputBufFill = openserial_vars.inputBufFill;
    ENABLE_INTERRUPTS();

    return inputBufFill-1; // removing the command byte
}

uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes) {
    uint8_t numBytesWritten;
    uint8_t inputBufFill;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    inputBufFill = openserial_vars.inputBufFill;
    ENABLE_INTERRUPTS();

    if (maxNumBytes<inputBufFill-1) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_GETDATA_ASKS_TOO_FEW_BYTES,
            (errorparameter_t)maxNumBytes,
            (errorparameter_t)inputBufFill-1
        );
        numBytesWritten = 0;
    } else {
        numBytesWritten = inputBufFill-1;
        memcpy(bufferToWrite,&(openserial_vars.inputBuf[1]),numBytesWritten);
    }

    return numBytesWritten;
}

//===== scheduling

void openserial_startInput(void) {
    INTERRUPT_DECLARATION();

    if (openserial_vars.inputBufFill>0) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_INPUTBUFFER_LENGTH,
            (errorparameter_t)openserial_vars.inputBufFill,
            (errorparameter_t)0
        );
        DISABLE_INTERRUPTS();
        openserial_vars.inputBufFill=0;
        ENABLE_INTERRUPTS();
    }

    uart_clearTxInterrupts();
    uart_clearRxInterrupts();      // clear possible pending interrupts
    uart_enableInterrupts();       // Enable USCI_A1 TX & RX interrupt

    DISABLE_INTERRUPTS();
    openserial_vars.busyReceiving  = FALSE;
    openserial_vars.mode           = MODE_INPUT;
    openserial_vars.reqFrameIdx    = 0;
#ifdef FASTSIM
    uart_writeBufferByLen_FASTSIM(
        openserial_vars.reqFrame,
        sizeof(openserial_vars.reqFrame)
    );
    openserial_vars.reqFrameIdx = sizeof(openserial_vars.reqFrame);
#else
    uart_writeByte(openserial_vars.reqFrame[openserial_vars.reqFrameIdx]);
#endif
    ENABLE_INTERRUPTS();
}

void openserial_startOutput(void) {
    uint8_t debugPrintCounter;
    INTERRUPT_DECLARATION();

    //=== made modules print debug information

    DISABLE_INTERRUPTS();
    openserial_vars.debugPrintCounter = (openserial_vars.debugPrintCounter+1)%STATUS_MAX;
    debugPrintCounter = openserial_vars.debugPrintCounter;
    ENABLE_INTERRUPTS();

    switch (debugPrintCounter) {
        case STATUS_ISSYNC:
            if (debugPrint_isSync()==TRUE) {
                break;
            }
        case STATUS_ID:
            if (debugPrint_id()==TRUE) {
               break;
            }
        case STATUS_DAGRANK:
            if (debugPrint_myDAGrank()==TRUE) {
                break;
            }
        case STATUS_OUTBUFFERINDEXES:
            if (debugPrint_outBufferIndexes()==TRUE) {
                break;
            }
        case STATUS_ASN:
            if (debugPrint_asn()==TRUE) {
                break;
            }
        case STATUS_MACSTATS:
            if (debugPrint_macStats()==TRUE) {
                break;
            }
        case STATUS_SCHEDULE:
            if(debugPrint_schedule()==TRUE) {
                break;
            }
        case STATUS_BACKOFF:
            if(debugPrint_backoff()==TRUE) {
                break;
            }
        case STATUS_QUEUE:
            if(debugPrint_queue()==TRUE) {
                break;
            }
        case STATUS_NEIGHBORS:
            if (debugPrint_neighbors()==TRUE) {
                break;
            }
        case STATUS_KAPERIOD:
            if (debugPrint_kaPeriod()==TRUE) {
                break;
            }
        case STATUS_JOINED:
            if (debugPrint_joined()==TRUE) {
                break;
            }
        default:
            DISABLE_INTERRUPTS();
            openserial_vars.debugPrintCounter=0;
            ENABLE_INTERRUPTS();
    }

    //=== flush TX buffer

    uart_clearTxInterrupts();
    uart_clearRxInterrupts();          // clear possible pending interrupts
    uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt

    DISABLE_INTERRUPTS();
    openserial_vars.mode=MODE_OUTPUT;
    if (openserial_vars.outputBufFilled) {
#ifdef FASTSIM
        uart_writeCircularBuffer_FASTSIM(
            openserial_vars.outputBuf,
            &openserial_vars.outputBufIdxR,
            &openserial_vars.outputBufIdxW
        );
#else
        uart_writeByte(openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxR++)]);
#endif
    } else {
        openserial_stop();
    }
    ENABLE_INTERRUPTS();
}

void openserial_stop(void) {
    uint8_t inputBufFill;
    uint8_t cmdByte;
    bool    busyReceiving;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    busyReceiving = openserial_vars.busyReceiving;
    inputBufFill  = openserial_vars.inputBufFill;
    ENABLE_INTERRUPTS();

    // disable UART interrupts
    uart_disableInterrupts();

    DISABLE_INTERRUPTS();
    openserial_vars.mode=MODE_OFF;
    ENABLE_INTERRUPTS();

    // the inputBuffer has to be reset if it is not reset where the data is read.
    // or the function openserial_getInputBuffer is called (which resets the buffer)
    if (busyReceiving==TRUE) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_BUSY_RECEIVING,
            (errorparameter_t)0,
            (errorparameter_t)inputBufFill
        );
    }

    if (busyReceiving==FALSE && inputBufFill>0) {
        DISABLE_INTERRUPTS();
        cmdByte = openserial_vars.inputBuf[0];
        ENABLE_INTERRUPTS();
        // call hard-coded commands
        // FIXME: needs to be replaced by registered commands only
        switch (cmdByte) {
            case SERFRAME_PC2MOTE_SETROOT:
                idmanager_triggerAboutRoot();
                break;
            case SERFRAME_PC2MOTE_RESET:
                board_reset();
                break;
            case SERFRAME_PC2MOTE_DATA:
                openbridge_triggerData();
                break;
            case SERFRAME_PC2MOTE_TRIGGERSERIALECHO:
                openserial_handleEcho(&openserial_vars.inputBuf[1],inputBufFill-1);
                break;
            case SERFRAME_PC2MOTE_COMMAND:
                openserial_handleCommands();
                break;
        }
        // call registered commands
        if (openserial_vars.registeredCmd!=NULL && openserial_vars.registeredCmd->cmdId==cmdByte) {

            openserial_vars.registeredCmd->cb();
        }
    }

    DISABLE_INTERRUPTS();
    openserial_vars.inputBufFill  = 0;
    openserial_vars.busyReceiving = FALSE;
    ENABLE_INTERRUPTS();
}

//===== debugprint

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_outBufferIndexes(void) {
    uint16_t temp_buffer[2];
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    temp_buffer[0] = openserial_vars.outputBufIdxW;
    temp_buffer[1] = openserial_vars.outputBufIdxR;
    ENABLE_INTERRUPTS();

    openserial_printStatus(
        STATUS_OUTBUFFERINDEXES,
        (uint8_t*)temp_buffer,
        sizeof(temp_buffer)
    );

    return TRUE;
}

//=========================== private =========================================

//===== printing

owerror_t openserial_printInfoErrorCritical(
    char             severity,
    uint8_t          calling_component,
    uint8_t          error_code,
    errorparameter_t arg1,
    errorparameter_t arg2
) {
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    openserial_vars.outputBufFilled  = TRUE;
    outputHdlcOpen();
    outputHdlcWrite(severity);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(calling_component);
    outputHdlcWrite(error_code);
    outputHdlcWrite((uint8_t)((arg1 & 0xff00)>>8));
    outputHdlcWrite((uint8_t) (arg1 & 0x00ff));
    outputHdlcWrite((uint8_t)((arg2 & 0xff00)>>8));
    outputHdlcWrite((uint8_t) (arg2 & 0x00ff));
    outputHdlcClose();
    ENABLE_INTERRUPTS();

    return E_SUCCESS;
}

//===== command handlers

void openserial_handleEcho(uint8_t* buf, uint8_t bufLen){
    // echo back what you received
    openserial_printData(
        buf,
        bufLen
    );
}

void openserial_get6pInfo(uint8_t commandId, uint8_t* code,uint8_t* cellOptions,uint8_t* numCells,cellInfo_ht* celllist_add,cellInfo_ht* celllist_delete,uint8_t* listOffset,uint8_t* maxListLen,uint8_t ptr, uint8_t commandLen){
    uint8_t i;
    uint8_t celllistLen;

    // clear command
    if (commandId == COMMAND_SET_6P_CLEAR){
        *code = IANA_6TOP_CMD_CLEAR;
        return;
    }

    *cellOptions  = openserial_vars.inputBuf[ptr];
    ptr          += 1;
    commandLen   -= 1;

    // list command
    if (commandId == COMMAND_SET_6P_LIST){
        *code = IANA_6TOP_CMD_LIST;
        *listOffset   = openserial_vars.inputBuf[ptr];
        ptr += 1;
        *maxListLen   = openserial_vars.inputBuf[ptr];
        ptr += 1;
        return;
    }

    // count command
    if (commandId == COMMAND_SET_6P_COUNT){
        *code = IANA_6TOP_CMD_COUNT;
        return;
    }

    *numCells   = openserial_vars.inputBuf[ptr];
    ptr        += 1;
    commandLen -= 1;

    // add command
    if (commandId == COMMAND_SET_6P_ADD){
        *code = IANA_6TOP_CMD_ADD;
        // retrieve cell list
        i = 0;
        celllistLen = commandLen/2;
        while(commandLen>celllistLen){
            celllist_add[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_add[i].channeloffset  = openserial_vars.inputBuf[ptr+celllistLen];
            celllist_add[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }

    // delete command
    if (commandId == COMMAND_SET_6P_DELETE){
        *code = IANA_6TOP_CMD_DELETE;
        // retrieve cell list
        i = 0;
        celllistLen = commandLen/2;
        while(commandLen>celllistLen){
            celllist_delete[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_delete[i].channeloffset  = openserial_vars.inputBuf[ptr+celllistLen];
            celllist_delete[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }

    // relocate command
    if (commandId == COMMAND_SET_6P_RELOCATE){
        *code = IANA_6TOP_CMD_RELOCATE;
        // retrieve cell list to be relocated
        i = 0;
        while(i<*numCells){
            celllist_delete[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_delete[i].channeloffset  = openserial_vars.inputBuf[ptr+*numCells];
            celllist_delete[i].isUsed         = TRUE;
            ptr         += 1;
            i++;
        }

        commandLen      -= (*numCells) * 2;
        ptr             += *numCells;
        // retrieve cell list to be relocated
        i = 0;
        celllistLen = commandLen/2;
        while(commandLen>celllistLen){
            celllist_add[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_add[i].channeloffset  = openserial_vars.inputBuf[ptr+celllistLen];
            celllist_add[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }
}

void openserial_handleCommands(void){
    uint8_t  input_buffer[20];
    uint8_t  numDataBytes;
    uint8_t  commandId;
    uint8_t  commandLen;
    uint8_t  comandParam_8;
    uint16_t comandParam_16;

    uint8_t  code,cellOptions,numCell,listOffset,maxListLen;
    uint8_t  ptr;
    cellInfo_ht celllist_add[CELLLIST_MAX_LEN];
    cellInfo_ht celllist_delete[CELLLIST_MAX_LEN];

    open_addr_t neighbor;
    bool        foundNeighbor;

    ptr = 0;
    memset(celllist_add,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));
    memset(celllist_delete,0,CELLLIST_MAX_LEN*sizeof(cellInfo_ht));

    numDataBytes = openserial_getInputBufferFilllevel();
    //copying the buffer
    openserial_getInputBuffer(&(input_buffer[ptr]),numDataBytes);
    ptr++;
    commandId  = openserial_vars.inputBuf[ptr];
    ptr++;
    commandLen = openserial_vars.inputBuf[ptr];
    ptr++;

    switch(commandId) {
        case COMMAND_SET_EBPERIOD:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            sixtop_setEBPeriod(comandParam_8); // one byte, in seconds
            break;
        case COMMAND_SET_CHANNEL:
           comandParam_8 = openserial_vars.inputBuf[ptr];
           // set communication channel for protocol stack
           ieee154e_setSingleChannel(comandParam_8); // one byte
           break;
        case COMMAND_SET_KAPERIOD: // two bytes, in slots
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00);
            sixtop_setKaPeriod(comandParam_16);
            break;
        case COMMAND_SET_DIOPERIOD: // two bytes, in mili-seconds
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00);
            icmpv6rpl_setDIOPeriod(comandParam_16);
            break;
        case COMMAND_SET_DAOPERIOD: // two bytes, in mili-seconds
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00);
            icmpv6rpl_setDAOPeriod(comandParam_16);
            break;
        case COMMAND_SET_DAGRANK: // two bytes
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00);
            icmpv6rpl_setMyDAGrank(comandParam_16);
            break;
        case COMMAND_SET_SECURITY_STATUS: // one byte
            comandParam_8 = openserial_vars.inputBuf[ptr];
            ieee154e_setIsSecurityEnabled(comandParam_8);
            break;
        case COMMAND_SET_SLOTFRAMELENGTH: // two bytes
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00);
            schedule_setFrameLength(comandParam_16);
            break;
        case COMMAND_SET_ACK_STATUS:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            ieee154e_setIsAckEnabled(comandParam_8);
            break;
        case COMMAND_SET_6P_ADD:
        case COMMAND_SET_6P_DELETE:
        case COMMAND_SET_6P_RELOCATE:
        case COMMAND_SET_6P_COUNT:
        case COMMAND_SET_6P_LIST:
        case COMMAND_SET_6P_CLEAR:
            // get preferred parent
            foundNeighbor =icmpv6rpl_getPreferredParentEui64(&neighbor);
            if (foundNeighbor==FALSE) {
                break;
            }
            // the following sequence of bytes are, slotframe, cellOption, numCell, celllist
            openserial_get6pInfo(commandId,&code,&cellOptions,&numCell,celllist_add,celllist_delete,&listOffset,&maxListLen,ptr,commandLen);
            sixtop_request(
                code,              // code
                &neighbor,         // neighbor
                numCell,           // number cells
                cellOptions,       // cellOptions
                celllist_add,      // celllist to add
                celllist_delete,   // celllist to delete (not used)
                msf_getsfid(),     // sfid
                listOffset,        // list command offset (not used)
                maxListLen         // list command maximum celllist (not used)
            );
            break;
        case COMMAND_SET_SLOTDURATION:
            comandParam_16 = (openserial_vars.inputBuf[ptr] & 0x00ff) | \
                ((openserial_vars.inputBuf[ptr+1]<<8) & 0xff00);
            ieee154e_setSlotDuration(comandParam_16);
            break;
        case COMMAND_SET_6PRESPONSE:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            if (comandParam_8 ==1) {
                sixtop_setIsResponseEnabled(TRUE);
            } else {
                if (comandParam_8 == 0) {
                    sixtop_setIsResponseEnabled(FALSE);
                } else {
                    // security only can be 1 or 0
                    break;
                }
            }
            break;
        case COMMAND_SET_UINJECTPERIOD:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            msf_appPktPeriod(comandParam_8);
            break;
        case COMMAND_SET_ECHO_REPLY_STATUS:
            comandParam_8 = openserial_vars.inputBuf[ptr];
            if (comandParam_8 == 1) {
                icmpv6echo_setIsReplyEnabled(TRUE);
            } else {
                if (comandParam_8 == 0) {
                    icmpv6echo_setIsReplyEnabled(FALSE);
                } else {
                    // ack reply
                    break;
                }
            }
            break;
        case COMMAND_SET_JOIN_KEY:
            if (commandLen != 16) { break; }
            idmanager_setJoinKey(&openserial_vars.inputBuf[ptr]);
            break;
        default:
           // wrong command ID
           break;
   }
}

//===== misc

void openserial_board_reset_cb(opentimers_id_t id) {
    (void)id;
    board_reset();
}

//===== hdlc (output)

/**
\brief Start an HDLC frame in the output buffer.
*/
port_INLINE void outputHdlcOpen(void) {
    // initialize the value of the CRC
    openserial_vars.outputCrc                                        = HDLC_CRCINIT;

    // write the opening HDLC flag
    openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]       = HDLC_FLAG;
}
/**
\brief Add a byte to the outgoing HDLC frame being built.
*/
port_INLINE void outputHdlcWrite(uint8_t b) {

    // iterate through CRC calculator
    openserial_vars.outputCrc = crcIteration(openserial_vars.outputCrc,b);

    // add byte to buffer
    if (b==HDLC_FLAG || b==HDLC_ESCAPE) {
        openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]   = HDLC_ESCAPE;
        b                                                            = b^HDLC_ESCAPE_MASK;
    }
    openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]       = b;
}
/**
\brief Finalize the outgoing HDLC frame.
*/
port_INLINE void outputHdlcClose(void) {
    uint16_t   finalCrc;

    // finalize the calculation of the CRC
    finalCrc   = ~openserial_vars.outputCrc;

    // write the CRC value
    outputHdlcWrite((finalCrc>>0)&0xff);
    outputHdlcWrite((finalCrc>>8)&0xff);

    // write the closing HDLC flag
    openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxW++)]       = HDLC_FLAG;
}

//===== hdlc (input)

/**
\brief Start an HDLC frame in the input buffer.
*/
port_INLINE void inputHdlcOpen(void) {
    // reset the input buffer index
    openserial_vars.inputBufFill                                     = 0;

    // initialize the value of the CRC
    openserial_vars.inputCrc                                         = HDLC_CRCINIT;
}
/**
\brief Add a byte to the incoming HDLC frame.
*/
port_INLINE void inputHdlcWrite(uint8_t b) {
    if (b==HDLC_ESCAPE) {
        openserial_vars.inputEscaping = TRUE;
    } else {
        if (openserial_vars.inputEscaping==TRUE) {
            b                             = b^HDLC_ESCAPE_MASK;
            openserial_vars.inputEscaping = FALSE;
        }

        // add byte to input buffer
        openserial_vars.inputBuf[openserial_vars.inputBufFill] = b;
        openserial_vars.inputBufFill++;

        // iterate through CRC calculator
        openserial_vars.inputCrc = crcIteration(openserial_vars.inputCrc,b);
    }
}
/**
\brief Finalize the incoming HDLC frame.
*/
port_INLINE void inputHdlcClose(void) {

    // verify the validity of the frame
    if (openserial_vars.inputCrc==HDLC_CRCGOOD) {
        // the CRC is correct

        // remove the CRC from the input buffer
        openserial_vars.inputBufFill    -= 2;
    } else {
        // the CRC is incorrect

        // drop the incoming fram
        openserial_vars.inputBufFill     = 0;
    }
}

//=========================== interrupt handlers ==============================

// executed in ISR, called from scheduler.c
void isr_openserial_tx(void) {
    switch (openserial_vars.mode) {
        case MODE_INPUT:
            openserial_vars.reqFrameIdx++;
            if (openserial_vars.reqFrameIdx<sizeof(openserial_vars.reqFrame)) {
                uart_writeByte(openserial_vars.reqFrame[openserial_vars.reqFrameIdx]);
            }
            break;
        case MODE_OUTPUT:
            if (openserial_vars.outputBufIdxW==openserial_vars.outputBufIdxR) {
                openserial_vars.outputBufFilled = FALSE;
            }
            if (openserial_vars.outputBufFilled) {
                uart_writeByte(openserial_vars.outputBuf[OUTPUT_BUFFER_MASK & (openserial_vars.outputBufIdxR++)]);
            }
            break;
        case MODE_OFF:
            default:
            break;
    }
}

// executed in ISR, called from scheduler.c
void isr_openserial_rx(void) {
    uint8_t rxbyte;
    uint8_t inputBufFill;

    // stop if I'm not in input mode
    if (openserial_vars.mode!=MODE_INPUT) {
        return;
    }

    // read byte just received
    rxbyte = uart_readByte();
    // keep length
    inputBufFill=openserial_vars.inputBufFill;

    if (
        openserial_vars.busyReceiving==FALSE  &&
        openserial_vars.lastRxByte==HDLC_FLAG &&
        rxbyte!=HDLC_FLAG
    ) {
        // start of frame

        // I'm now receiving
        openserial_vars.busyReceiving         = TRUE;

        // create the HDLC frame
        inputHdlcOpen();

        // add the byte just received
        inputHdlcWrite(rxbyte);
    } else if (
        openserial_vars.busyReceiving==TRUE   &&
        rxbyte!=HDLC_FLAG
    ) {
        // middle of frame

        // add the byte just received
        inputHdlcWrite(rxbyte);
        if (openserial_vars.inputBufFill+1>SERIAL_INPUT_BUFFER_SIZE){
            // input buffer overflow
            openserial_printError(
                COMPONENT_OPENSERIAL,
                ERR_INPUT_BUFFER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)0
            );
            openserial_vars.inputBufFill       = 0;
            openserial_vars.busyReceiving      = FALSE;
            openserial_stop();
        }
    } else if (
        openserial_vars.busyReceiving==TRUE   &&
        rxbyte==HDLC_FLAG
    ) {
        // end of frame

        // finalize the HDLC frame
        inputHdlcClose();

        if (openserial_vars.inputBufFill==0){
            // invalid HDLC frame
            openserial_printError(
                COMPONENT_OPENSERIAL,
                ERR_WRONG_CRC_INPUT,
                (errorparameter_t)inputBufFill,
                (errorparameter_t)0
            );
        }

        openserial_vars.busyReceiving      = FALSE;
        openserial_stop();
    }

    openserial_vars.lastRxByte = rxbyte;
}



