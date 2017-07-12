/**
\brief Definition of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/

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
#include "sf0.h"
#include "debugpins.h"

//=========================== variables =======================================

openserial_vars_t openserial_vars;

//=========================== prototypes ======================================

extern void sniffer_setListeningChannel(uint8_t channel);

// printing
owerror_t openserial_printInfoErrorCritical(
    char             severity,
    uint8_t          calling_component,
    uint8_t          error_code,
    errorparameter_t arg1,
    errorparameter_t arg2
);

// command handlers
void openserial_handleRxFrame(void);
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

// sniffer
void sniffer_setListeningChannel(uint8_t channel);

//=========================== public ==========================================

//===== admin

void openserial_init() {
    // reset variable
    memset(&openserial_vars,0,sizeof(openserial_vars_t));
    
    // admin
    openserial_vars.fInhibited         = FALSE;
    openserial_vars.ctsStateChanged    = FALSE;
    openserial_vars.debugPrintCounter  = 0;
    
    // input
    openserial_vars.hdlcLastRxByte     = HDLC_FLAG;
    openserial_vars.hdlcBusyReceiving  = FALSE;
    openserial_vars.hdlcInputEscaping  = FALSE;
    openserial_vars.inputBufFillLevel  = 0;
    
    // ouput
    openserial_vars.outputBufIdxR      = 0;
    openserial_vars.outputBufIdxW      = 0;
    openserial_vars.fBusyFlushing      = FALSE;
    
    // UART
    uart_setCallbacks(
        isr_openserial_tx,
        isr_openserial_rx
    );
    uart_enableInterrupts();
}

void openserial_register(openserial_rsvpt* rsvp) {
    // FIXME: register multiple commands (linked list)
    openserial_vars.registeredCmd = rsvp;
}

//===== transmitting

owerror_t openserial_printStatus(
    uint8_t             statusElement,
    uint8_t*            buffer,
    uint8_t             length
) {
    uint8_t i;
    
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_STATUS);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(statusElement);
    for (i=0;i<length;i++){
        outputHdlcWrite(buffer[i]);
    }
    outputHdlcClose();
    
    // start TX'ing
    openserial_flush();
    
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
    
    // openserial_flush called by openserial_printInfoErrorCritical()
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
    
    // openserial_flush called by openserial_printInfoErrorCritical()
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
    
    // openserial_flush called by openserial_printInfoErrorCritical()
}

owerror_t openserial_printData(uint8_t* buffer, uint8_t length) {
    uint8_t  i;
    uint8_t  asn[5];
    
    // retrieve ASN
    ieee154e_getAsn(asn);
    
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_DATA);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    outputHdlcWrite(asn[0]);
    outputHdlcWrite(asn[1]);
    outputHdlcWrite(asn[2]);
    outputHdlcWrite(asn[3]);
    outputHdlcWrite(asn[4]);
    for (i=0;i<length;i++){
        outputHdlcWrite(buffer[i]);
    }
    outputHdlcClose();
    
    // start TX'ing
    openserial_flush();
    
    return E_SUCCESS;
}

owerror_t openserial_printSniffedPacket(uint8_t* buffer, uint8_t length, uint8_t channel) {
    uint8_t  i;
    
    outputHdlcOpen();
    outputHdlcWrite(SERFRAME_MOTE2PC_SNIFFED_PACKET);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[1]);
    outputHdlcWrite(idmanager_getMyID(ADDR_16B)->addr_16b[0]);
    for (i=0;i<length;i++){
       outputHdlcWrite(buffer[i]);
    }
    outputHdlcWrite(channel);
    outputHdlcClose();
    
    // start TX'ing
    openserial_flush();
    
    return E_SUCCESS;
}

void openserial_triggerDebugprint() {
    uint8_t debugPrintCounter;
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    debugPrintCounter = openserial_vars.debugPrintCounter;
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
    
    if (openserial_vars.outputBufIdxW!=openserial_vars.outputBufIdxR) {
        return;
    }
    // FIX: remove when openserial_triggerDebugprint called in a task...
    
    debugPrintCounter++;
    if (debugPrintCounter==STATUS_MAX) {
       debugPrintCounter = 0;
    }
    
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
        default:
            debugPrintCounter=0;
    }
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    openserial_vars.debugPrintCounter = debugPrintCounter;
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
}

//===== receiving

uint8_t openserial_getInputBufferFillLevel() {
    uint8_t inputBufFillLevel;
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    inputBufFillLevel = openserial_vars.inputBufFillLevel;
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
    
    return inputBufFillLevel-1; // removing the command byte
}

uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes) {
    uint8_t numBytesWritten;
    uint8_t inputBufFillLevel;
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    inputBufFillLevel = openserial_vars.inputBufFillLevel;
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
     
    if (maxNumBytes<inputBufFillLevel-1) {
        openserial_printError(
            COMPONENT_OPENSERIAL,
            ERR_GETDATA_ASKS_TOO_FEW_BYTES,
            (errorparameter_t)maxNumBytes,
            (errorparameter_t)inputBufFillLevel-1
        );
        numBytesWritten = 0;
    } else {
        numBytesWritten = inputBufFillLevel-1;
        //<<<<<<<<<<<<<<<<<<<<<<<
        DISABLE_INTERRUPTS();
        memcpy(bufferToWrite,&(openserial_vars.inputBuf[1]),numBytesWritten);
        ENABLE_INTERRUPTS();
        //>>>>>>>>>>>>>>>>>>>>>>>
    }
    
    return numBytesWritten;
}

//===== scheduling

void openserial_flush(void) {
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    if (openserial_vars.fBusyFlushing==FALSE) {
        if (openserial_vars.ctsStateChanged==TRUE) {
            // send CTS
#ifdef FASTSIM
#else
            if (openserial_vars.fInhibited==TRUE) {
                uart_setCTS(FALSE);
            } else {
                uart_setCTS(TRUE);
            }
#endif
            openserial_vars.ctsStateChanged = FALSE;
        } else {
            if (openserial_vars.fInhibited==TRUE) {
                // currently inhibited
            } else {
                // not inhibited
                if (openserial_vars.outputBufIdxW!=openserial_vars.outputBufIdxR) {
                    // I have some bytes to transmit
            
#ifdef FASTSIM
                    uart_writeCircularBuffer_FASTSIM(
                        openserial_vars.outputBuf,
                        &openserial_vars.outputBufIdxR,
                        &openserial_vars.outputBufIdxW
                    );
#else
                    uart_writeByte(openserial_vars.outputBuf[openserial_vars.outputBufIdxR++]);
                    openserial_vars.fBusyFlushing = TRUE;
#endif
                }
            }
        }
    }
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
}

void openserial_inhibitStart(void) {
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    openserial_vars.fInhibited      = TRUE;
#ifdef FASTSIM
#else
    openserial_vars.ctsStateChanged = TRUE;
#endif
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
    
    // it's openserial_flush() which will set CTS
    openserial_flush();
}

void openserial_inhibitStop(void) {
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    openserial_vars.fInhibited      = FALSE;
#ifdef FASTSIM
#else
    openserial_vars.ctsStateChanged = TRUE;
#endif
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
    
    // it's openserial_flush() which will set CTS
    openserial_flush();
}

//===== debugprint

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_outBufferIndexes() {
    uint16_t temp_buffer[2];
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    temp_buffer[0] = openserial_vars.outputBufIdxW;
    temp_buffer[1] = openserial_vars.outputBufIdxR;
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
    
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
    
    // start TX'ing
    openserial_flush();
    
    return E_SUCCESS;
}

//===== command handlers

// executed in ISR
void openserial_handleRxFrame() {
    uint8_t cmdByte;
    
    cmdByte = openserial_vars.inputBuf[0];
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
            openserial_handleEcho(
                &openserial_vars.inputBuf[1],
                openserial_vars.inputBufFillLevel-1
            );
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

void openserial_handleEcho(uint8_t* buf, uint8_t bufLen){
    // echo back what you received
    openserial_printData(
        buf,
        bufLen
    );
}

void openserial_get6pInfo(uint8_t commandId, uint8_t* code,uint8_t* cellOptions,uint8_t* numCells,cellInfo_ht* celllist_add,cellInfo_ht* celllist_delete,uint8_t* listOffset,uint8_t* maxListLen,uint8_t ptr, uint8_t commandLen){
    uint8_t i; 
    
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
        while(commandLen>0){
            celllist_add[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_add[i].channeloffset  = DEFAULT_CHANNEL_OFFSET;
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
        i = 0;
        while(commandLen>0){
            celllist_delete[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_delete[i].channeloffset  = DEFAULT_CHANNEL_OFFSET;
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
            celllist_delete[i].channeloffset  = DEFAULT_CHANNEL_OFFSET;
            celllist_delete[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        // retrieve cell list to be relocated
        i = 0;
        while(commandLen>0){
            celllist_add[i].slotoffset     = openserial_vars.inputBuf[ptr];
            celllist_add[i].channeloffset  = DEFAULT_CHANNEL_OFFSET;
            celllist_add[i].isUsed         = TRUE;
            ptr         += 1;
            commandLen  -= 1;
            i++;
        }
        return;
    }
}

void openserial_handleCommands(void){

    uint8_t  input_buffer[10];
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
   
   numDataBytes = openserial_getInputBufferFillLevel();
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
            // set listening channel for sniffer
            sniffer_setListeningChannel(comandParam_8); // one byte
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
            if (sixtop_setHandler(SIX_HANDLER_SF0)==FALSE){
                // one sixtop transcation is happening, only one instance at one time
                return;
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
                sf0_getsfid(),     // sfid
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
            sf0_appPktPeriod(comandParam_8);
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
        default:
            // wrong command ID
            break;
   }
}

//===== misc

void openserial_board_reset_cb(opentimers_id_t id) {
    board_reset();
}

//===== hdlc (output)

/**
\brief Start an HDLC frame in the output buffer.
*/
port_INLINE void outputHdlcOpen() {
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    
    // initialize the value of the CRC
    
    openserial_vars.hdlcOutputCrc                                    = HDLC_CRCINIT;
    
    // write the opening HDLC flag
    openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]       = HDLC_FLAG;
    
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
}
/**
\brief Add a byte to the outgoing HDLC frame being built.
*/
port_INLINE void outputHdlcWrite(uint8_t b) {
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    
    // iterate through CRC calculator
    openserial_vars.hdlcOutputCrc = crcIteration(openserial_vars.hdlcOutputCrc,b);
    
    // add byte to buffer
    if (b==HDLC_FLAG || b==HDLC_ESCAPE) {
        openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]   = HDLC_ESCAPE;
        b                                                            = b^HDLC_ESCAPE_MASK;
    }
    openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]       = b;
    
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
}
/**
\brief Finalize the outgoing HDLC frame.
*/
port_INLINE void outputHdlcClose() {
    uint16_t   finalCrc;
    INTERRUPT_DECLARATION();
    
    //<<<<<<<<<<<<<<<<<<<<<<<
    DISABLE_INTERRUPTS();
    
    // finalize the calculation of the CRC
    finalCrc   = ~openserial_vars.hdlcOutputCrc;

    // write the CRC value
    outputHdlcWrite((finalCrc>>0)&0xff);
    outputHdlcWrite((finalCrc>>8)&0xff);

    // write the closing HDLC flag
    openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]       = HDLC_FLAG;
    
    ENABLE_INTERRUPTS();
    //>>>>>>>>>>>>>>>>>>>>>>>
}

//===== hdlc (input)

/**
\brief Start an HDLC frame in the input buffer.
*/
port_INLINE void inputHdlcOpen() {
    // reset the input buffer index
    openserial_vars.inputBufFillLevel                                = 0;

    // initialize the value of the CRC
    openserial_vars.hdlcInputCrc                                     = HDLC_CRCINIT;
}
/**
\brief Add a byte to the incoming HDLC frame.
*/
port_INLINE void inputHdlcWrite(uint8_t b) {
    if (b==HDLC_ESCAPE) {
        openserial_vars.hdlcInputEscaping = TRUE;
    } else {
        if (openserial_vars.hdlcInputEscaping==TRUE) {
            b                             = b^HDLC_ESCAPE_MASK;
            openserial_vars.hdlcInputEscaping = FALSE;
        }
        
        // add byte to input buffer
        openserial_vars.inputBuf[openserial_vars.inputBufFillLevel] = b;
        openserial_vars.inputBufFillLevel++;
        
        // iterate through CRC calculator
        openserial_vars.hdlcInputCrc = crcIteration(openserial_vars.hdlcInputCrc,b);
    }
}
/**
\brief Finalize the incoming HDLC frame.
*/
port_INLINE void inputHdlcClose() {
    
    // verify the validity of the frame
    if (openserial_vars.hdlcInputCrc==HDLC_CRCGOOD) {
        // the CRC is correct
        
        // remove the CRC from the input buffer
        openserial_vars.inputBufFillLevel    -= 2;
    } else {
        // the CRC is incorrect
        
        // drop the incoming frame
        openserial_vars.inputBufFillLevel     = 0;
    }
}

//=========================== interrupt handlers ==============================

// executed in ISR, called from scheduler.c
void isr_openserial_tx() {
    if (openserial_vars.ctsStateChanged==TRUE) {
        // set CTS
        
        if (openserial_vars.fInhibited==TRUE) {
            uart_setCTS(FALSE);
            openserial_vars.fBusyFlushing = FALSE;
        } else {
            uart_setCTS(TRUE);
        }
        openserial_vars.ctsStateChanged = FALSE;
    } else if (openserial_vars.fInhibited==TRUE) {
        // currently inhibited
        
        openserial_vars.fBusyFlushing = FALSE;
    } else {
        // not inhibited
        
        if (openserial_vars.outputBufIdxW!=openserial_vars.outputBufIdxR) {
            // I have some bytes to transmit
            
            uart_writeByte(openserial_vars.outputBuf[openserial_vars.outputBufIdxR++]);
            openserial_vars.fBusyFlushing = TRUE;
        } else {
            // I'm done sending bytes
            
            openserial_vars.fBusyFlushing = FALSE;
        }
    }
}

/**
\pre executed in ISR, called from scheduler.c

\returns 1 if don't receiving frame, 0 if not
*/
uint8_t isr_openserial_rx() {
    uint8_t rxbyte;
    uint8_t returnVal;
    
    returnVal = 0;
    
    // read byte just received
    rxbyte = uart_readByte();
    
    if (
        openserial_vars.hdlcBusyReceiving==FALSE  &&
        openserial_vars.hdlcLastRxByte==HDLC_FLAG &&
        rxbyte!=HDLC_FLAG
    ) {
        // start of frame

        // I'm now receiving
        openserial_vars.hdlcBusyReceiving         = TRUE;

        // create the HDLC frame
        inputHdlcOpen();

        // add the byte just received
        inputHdlcWrite(rxbyte);
    } else if (
        openserial_vars.hdlcBusyReceiving==TRUE   &&
        rxbyte!=HDLC_FLAG
    ) {
        // middle of frame

        // add the byte just received
        inputHdlcWrite(rxbyte);
        if (openserial_vars.inputBufFillLevel+1>SERIAL_INPUT_BUFFER_SIZE){
            // input buffer overflow
            openserial_printError(
                COMPONENT_OPENSERIAL,
                ERR_INPUT_BUFFER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)0
            );
            openserial_vars.inputBufFillLevel      = 0;
            openserial_vars.hdlcBusyReceiving      = FALSE;
        }
    } else if (
        openserial_vars.hdlcBusyReceiving==TRUE   &&
        rxbyte==HDLC_FLAG
    ) {
        // end of frame
        
        // finalize the HDLC frame
        inputHdlcClose();
        openserial_vars.hdlcBusyReceiving      = FALSE;
        
        if (openserial_vars.inputBufFillLevel==0){
            // invalid HDLC frame
            openserial_printError(
                COMPONENT_OPENSERIAL,
                ERR_WRONG_CRC_INPUT,
                (errorparameter_t)openserial_vars.inputBufFillLevel,
                (errorparameter_t)0
            );
        } else {
            openserial_handleRxFrame();
            openserial_vars.inputBufFillLevel = 0;
            returnVal = 1;
        }
    }
    
    openserial_vars.hdlcLastRxByte = rxbyte;
    
    return returnVal;
}



