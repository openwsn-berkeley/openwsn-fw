/**
\brief Definition of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
\author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
*/

#include "opendefs.h"
#include "openserial.h"
#include "packetfunctions.h"
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
#if (SFMETHOD == SFMETHOD_SF0)
    #include "sf0.h"
#endif
#if (SFMETHOD == SFMETHOD_SFLOC)
    #include "sfloc.h"
#endif


//#define _DEBUG_OPENSERIAL_              // debug variables to verify that openstat works properly
#define OPENSERIAL_STAT_FALSECRC_       // should we send to openvisualizer the CRC failed frames? // it consumes bandwidth through the serial line
//#define OPENSERIAL_STAT_OVERFLOW_       // should we send to openvisualizer the buffer overflow? // it consumes bandwidth through the serial line
#define OPENSERIAL_STAT                   // push the statistics to openVisualizer

//#define FASTSIM
#define _DEBUG_OPENSERIAL_
#ifdef _DEBUG_OPENSERIAL_
uint8_t caller;

enum{
   PRINTSTAT      = 1,
   PRINTF         = 2,
   PRINTSTATUS    = 3,
   PRINTERRORCRIT = 4,
   PRINTDATA      = 5,
   PRINTPACKET    = 6
};

//to debug buffer overflows
uint16_t  _size_remain = 0;
uint16_t  _size_towrite = 0;

#endif

//=========================== variables =======================================

openserial_vars_t openserial_vars;

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
void openserial_handleCommands(void);

// misc
void openserial_board_reset_cb(
    opentimer_id_t id
);

// HDLC output
uint8_t openserial_get_output_buffer(char *buffer, uint8_t length);
void outputHdlcOpen(uint8_t bufindex);
void outputHdlcWrite(uint8_t bufindex, uint8_t b);
void outputHdlcClose(uint8_t bufindex);

// HDLC input
void inputHdlcOpen(void);
void inputHdlcWrite(uint8_t b);
void inputHdlcClose(void);

//Parameters for the execution to push to openVisualizer
bool debugPrint_params(void);

//=========================== public ==========================================

//===== admin

void openserial_init() {
    uint16_t    crc;
    uint8_t     i;
    
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
    openserial_vars.outputCurrentW  = 0;
    openserial_vars.outputCurrentR  = 0;
    for(i=0; i<OPENSERIAL_OUTPUT_NBBUFFERS; i++){
        openserial_vars.outputBufFilled[i] = FALSE;
        openserial_vars.outputBufIdxR[i]       = 0;
        openserial_vars.outputBufIdxW[i]       = 0;

        // set callbacks
        uart_setCallbacks(
        isr_openserial_tx,
        isr_openserial_rx
        );
    }
}

void openserial_register(openserial_rsvpt* rsvp) {
    // FIXME: register multiple commands (linked list)
    openserial_vars.registeredCmd = rsvp;
}

//===== printing


owerror_t openserial_push_output_buffer(char *buffer_out, uint8_t buffer_out_length){
   uint8_t  buf_id;              //the buffer to use
   uint8_t  buffer_out_index;    //to walk in the buffer to push

   INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    buf_id = openserial_get_output_buffer(buffer_out, buffer_out_length);
    if (buf_id >= OPENSERIAL_OUTPUT_NBBUFFERS){
       //leds_error_toggle();
       ENABLE_INTERRUPTS();
       return(E_FAIL);
    }

    //write the buffer
    openserial_vars.outputBufFilled[buf_id] = TRUE;
    outputHdlcOpen(buf_id);
    for (buffer_out_index=0; buffer_out_index<buffer_out_length; buffer_out_index++){
       outputHdlcWrite(buf_id, buffer_out[buffer_out_index]);
    }
    outputHdlcClose(buf_id);

    ENABLE_INTERRUPTS();

    return E_SUCCESS;
}


owerror_t openserial_printStat(uint8_t type, uint8_t calling_component, uint8_t *buffer_in, uint8_t buffer_in_length) {
#ifndef OPENSERIAL_STAT
   return(E_SUCCESS);
#endif

   char     buffer_out[256];     //the message to send
   uint8_t  buffer_out_length = 0;   //its length
   uint8_t  asn[5];

   //prepare the headers and the content
   ieee154e_getAsn(asn);// byte01,byte23,byte4
   buffer_out[buffer_out_length++] = SERFRAME_MOTE2PC_STAT;
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[0];
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[1];
   buffer_out[buffer_out_length++] = calling_component;
   buffer_out[buffer_out_length++] = asn[0];
   buffer_out[buffer_out_length++] = asn[1];
   buffer_out[buffer_out_length++] = asn[2];
   buffer_out[buffer_out_length++] = asn[3];
   buffer_out[buffer_out_length++] = asn[4];
   buffer_out[buffer_out_length++] = type;

   //copy the "payload"
   memcpy(&(buffer_out[buffer_out_length]), buffer_in, buffer_in_length);
   buffer_out_length += buffer_in_length;


#ifdef _DEBUG_OPENSERIAL_
   caller = PRINTSTAT;
#endif

   //push the buffer
   return(openserial_push_output_buffer(buffer_out, buffer_out_length));
}


owerror_t openserial_printf(uint8_t calling_component, char* buffer_in, uint8_t buffer_in_length) {
#ifndef OPENSERIAL_PRINTF
   return(E_SUCCESS);
#endif

   char     buffer_out[256];     //the message to send
   uint8_t  buffer_out_length = 0;   //its length
  uint8_t  asn[5];

   //prepare the headers  and the content
   ieee154e_getAsn(asn);// byte01,byte23,byte4
   buffer_out[buffer_out_length++] = SERFRAME_MOTE2PC_PRINTF;
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[0];
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[1];
   buffer_out[buffer_out_length++] = calling_component;
   buffer_out[buffer_out_length++] = asn[0];
   buffer_out[buffer_out_length++] = asn[1];
   buffer_out[buffer_out_length++] = asn[2];
   buffer_out[buffer_out_length++] = asn[3];
   buffer_out[buffer_out_length++] = asn[4];

   //copy the "payload"
    memcpy(&(buffer_out[buffer_out_length]), buffer_in, buffer_in_length);
    buffer_out_length += buffer_in_length;

#ifdef _DEBUG_OPENSERIAL_
   caller = PRINTF;
#endif

   //push the buffer
   return(openserial_push_output_buffer(buffer_out, buffer_out_length));
}



owerror_t openserial_printStatus(uint8_t statusElement, uint8_t* buffer_in, uint8_t buffer_in_length) {
   char     buffer_out[256];     //the message to send
   uint8_t  buffer_out_length = 0;   //its length

   //prepare the headers  and the content
   buffer_out[buffer_out_length++] = SERFRAME_MOTE2PC_STATUS;
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[0];
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[1];
   buffer_out[buffer_out_length++] = statusElement;

   //copy the "payload"
    memcpy(&(buffer_out[buffer_out_length]), buffer_in, buffer_in_length);
    buffer_out_length += buffer_in_length;

#ifdef _DEBUG_OPENSERIAL_
   caller = 10 + statusElement;
#endif

   //push the buffer
   return(openserial_push_output_buffer(buffer_out, buffer_out_length));
}




owerror_t openserial_printInfo(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_INFO,
      calling_component,
      error_code,
      arg1,
      arg2
   );
}

owerror_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   // blink error LED, this is serious
   switch(error_code){
      case ERR_INVALIDPACKETFROMRADIO:
            return(E_SUCCESS);
         break;

      case ERR_LARGE_TIMECORRECTION:
      case ERR_WRONG_STATE_IN_ENDFRAME_SYNC:
      case ERR_WRONG_STATE_IN_STARTSLOT:
      case ERR_WRONG_STATE_IN_TIMERFIRES:
      case ERR_WRONG_STATE_IN_NEWSLOT:
      case ERR_WRONG_STATE_IN_ENDOFFRAME:
         break;
      default:
         leds_error_toggle();

   }

   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_ERROR,
      calling_component,
      error_code,
      arg1,
      arg2
   );
}

owerror_t openserial_printCritical(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   // blink error LED, this is serious
   leds_error_blink();

   // schedule for the mote to reboot in 10s
   opentimers_start(10000,
                    TIMER_ONESHOT,TIME_MS,
                    openserial_board_reset_cb);

   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_CRITICAL,
      calling_component,
      error_code,
      arg1,
      arg2
   );
}


owerror_t openserial_printData(uint8_t* buffer_in, uint8_t buffer_in_length) {
   char     buffer_out[256];     //the message to send
   uint8_t  buffer_out_length = 0;   //its length
   uint8_t  asn[5];

   //prepare the headers  and the content
   ieee154e_getAsn(asn);// byte01,byte23,byte4
   buffer_out[buffer_out_length++] = SERFRAME_MOTE2PC_DATA;
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[0];
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[1];
   buffer_out[buffer_out_length++] = asn[0];
   buffer_out[buffer_out_length++] = asn[1];
   buffer_out[buffer_out_length++] = asn[2];
   buffer_out[buffer_out_length++] = asn[3];
   buffer_out[buffer_out_length++] = asn[4];

   //copy the "payload"
   memcpy(&(buffer_out[buffer_out_length]), buffer_in, buffer_in_length);
   buffer_out_length += buffer_in_length;

#ifdef _DEBUG_OPENSERIAL_
   caller = PRINTDATA;
#endif

   //push the buffer
   return(openserial_push_output_buffer(buffer_out, buffer_out_length));
}

owerror_t openserial_printSniffedPacket(uint8_t* buffer_in, uint8_t buffer_in_length, uint8_t channel) {
   char     buffer_out[256];     //the message to send
   uint8_t  buffer_out_length = 0;   //its length

   //prepare the headers  and the content
   buffer_out[buffer_out_length++] = SERFRAME_MOTE2PC_SNIFFED_PACKET;
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[0];
   buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[1];

   //copy the "payload"
   memcpy(&(buffer_out[buffer_out_length]), buffer_in, buffer_in_length);
   buffer_out_length += buffer_in_length;
   buffer_out[buffer_out_length++] = channel;

#ifdef _DEBUG_OPENSERIAL_
   caller = PRINTPACKET;
#endif

   //push the buffer
   return(openserial_push_output_buffer(buffer_out, buffer_out_length));
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
        case STATUS_PARAMS:
           if (debugPrint_params()==TRUE) {
              break;
           }
           // to refresh more frequently the status of the queue and the schedule (multiple rows)
        case STATUS_SCHEDULEBIS:
           if(debugPrint_schedule()==TRUE) {
              break;
           }
        case STATUS_QUEUEBIS:
           if(debugPrint_queue()==TRUE) {
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
    
    //STAT buffer
    uint8_t bufindex = openserial_vars.outputCurrentR;

    DISABLE_INTERRUPTS();

    //STAT OUTPUT buffer
    openserial_vars.mode=MODE_OUTPUT;
   if (openserial_vars.outputBufFilled[bufindex]){

 #ifdef FASTSIM
       uart_writeCircularBuffer_FASTSIM(
          openserial_vars.outputBuf[bufindex],
          &openserial_vars.outputBufIdxR[bufindex],
          &openserial_vars.outputBufIdxW[bufindex]
       );
 #else
       uart_writeByte(openserial_vars.outputBuf[bufindex][openserial_vars.outputBufIdxR[bufindex]++]);
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

               // openbridge_triggerData();
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
    uint16_t temp_buffer[4];
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    temp_buffer[0] = openserial_vars.outputBufIdxW[openserial_vars.outputCurrentR];
    temp_buffer[1] = openserial_vars.outputBufIdxR[openserial_vars.outputCurrentR];
    temp_buffer[2] = openserial_vars.outputCurrentW;
    temp_buffer[3] = openserial_vars.outputCurrentR;
    ENABLE_INTERRUPTS();
    openserial_printStatus(STATUS_OUTBUFFERINDEXES,(uint8_t*)temp_buffer,sizeof(temp_buffer));
    return TRUE;
}


/**
 * \brief sends the current parameter values for this node
 *
 * debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.
 *
 */
bool debugPrint_params(void){
   debugParamsEntry_t   temp;


   memcpy(temp.addr, idmanager_getMyID(ADDR_64B), sizeof(temp.addr));
   temp.track_mgmt        = TRACK_MGMT;
#ifdef SCHEDULE_SHAREDCELLS_DISTRIBUTED
   temp.distr_cells       = TRUE;
#else
   temp.distr_cells       = FALSE;
#endif
   temp.rpl_metric        = RPL_METRIC;
   temp.scheduling_algo   = SCHEDULING_ALGO;
   temp.cexample_period   = CEXAMPLE_PERIOD;
   temp.sf                = SFMETHOD;
   openserial_printStatus(STATUS_PARAMS,(uint8_t*)(&temp),sizeof(temp));


#if ((SCHEDULING_ALGO != SCHEDULING_RANDOM) && (SCHEDULING_ALGO != SCHEDULING_RANDOM_CONTIGUOUS))
   char str[150];
   sprintf(str, "ERROR: the scheduling algo is incorrect");
   openserial_printf(COMPONENT_OPENSERIAL, str, strlen(str));
#endif

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
    char     buffer_out[256];     //the message to send
    uint8_t  buffer_out_length = 0;   //its length

    //prepare the headers  and the content
    buffer_out[buffer_out_length++] = severity;
    buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[0];
    buffer_out[buffer_out_length++] = idmanager_getMyID(ADDR_16B)->addr_16b[1];
    buffer_out[buffer_out_length++] = calling_component;
    buffer_out[buffer_out_length++] = error_code;
    buffer_out[buffer_out_length++] = (uint8_t)((arg1 & 0xff00)>>8);
    buffer_out[buffer_out_length++] = (uint8_t) (arg1 & 0x00ff);
    buffer_out[buffer_out_length++] = (uint8_t)((arg2 & 0xff00)>>8);
    buffer_out[buffer_out_length++] = (uint8_t) (arg2 & 0x00ff);

 #ifdef _DEBUG_OPENSERIAL_
    caller = PRINTERRORCRIT;
 #endif

    //push the buffer
    return(openserial_push_output_buffer(buffer_out, buffer_out_length));
}

//===== command handlers

void openserial_handleEcho(uint8_t* buf, uint8_t bufLen){
    
   //TODO
   return;

    // echo back what you received
    openserial_printData(
        buf,
        bufLen
    );
}

void openserial_handleCommands(void){
   uint8_t  input_buffer[10];
   uint8_t  numDataBytes;
   uint8_t  commandId;
   uint8_t  commandLen;
   uint8_t  comandParam_8;
   uint16_t comandParam_16;
   cellInfo_ht cellList[SCHEDULEIEMAXNUMCELLS];
   uint8_t  i;
   
   open_addr_t neighbor;
   bool        foundNeighbor;
   
   memset(cellList,0,sizeof(cellList));
   
   numDataBytes = openserial_getInputBufferFilllevel();
   //copying the buffer
   openserial_getInputBuffer(&(input_buffer[0]),numDataBytes);
   commandId  = openserial_vars.inputBuf[1];
   commandLen = openserial_vars.inputBuf[2];
   
   if (commandLen>3) {
       // the max command Len is 2, except ping commands
       return;
   } else {
       if (commandLen == 1) {
           comandParam_8 = openserial_vars.inputBuf[3];
       } else {
           // commandLen == 2
           comandParam_16 = (openserial_vars.inputBuf[3]      & 0x00ff) | \
                            ((openserial_vars.inputBuf[4]<<8) & 0xff00); 
       }
   }
   
   switch(commandId) {
       case COMMAND_SET_EBPERIOD:
           sixtop_setEBPeriod(comandParam_8); // one byte, in seconds
           break;
       case COMMAND_SET_CHANNEL:
           // set communication channel for protocol stack
           ieee154e_setSingleChannel(comandParam_8); // one byte
           // set listenning channel for sniffer
           //sniffer_setListeningChannel(comandParam_8); // one byte
           break;
       case COMMAND_SET_KAPERIOD: // two bytes, in slots
           sixtop_setKaPeriod(comandParam_16);
           break;
       case COMMAND_SET_DIOPERIOD: // two bytes, in mili-seconds
           icmpv6rpl_setDIOPeriod(comandParam_16);
           break;
       case COMMAND_SET_DAOPERIOD: // two bytes, in mili-seconds
           icmpv6rpl_setDAOPeriod(comandParam_16);
           break;
       case COMMAND_SET_DAGRANK: // two bytes
           icmpv6rpl_setMyDAGrank(comandParam_16);
           break;
       case COMMAND_SET_SECURITY_STATUS: // one byte
           if (comandParam_8 ==1) {
               ieee154e_setIsSecurityEnabled(TRUE);
           } else {
               if (comandParam_8 == 0) {
                  ieee154e_setIsSecurityEnabled(FALSE);
               } else {
                   // security only can be 1 or 0 
                   break;
               }
           }
           break;
       case COMMAND_SET_SLOTFRAMELENGTH: // two bytes
           schedule_setFrameLength(comandParam_16);
           break;
       case COMMAND_SET_ACK_STATUS:
           if (comandParam_8 == 1) {
               ieee154e_setIsAckEnabled(TRUE);
           } else {
               if (comandParam_8 == 0) {
                   ieee154e_setIsAckEnabled(FALSE);
               } else {
                   // ack reply
                   break;
               }
           }
           break;
        case COMMAND_SET_6P_ADD:
        case COMMAND_SET_6P_DELETE:
        case COMMAND_SET_6P_COUNT:
        case COMMAND_SET_6P_LIST:
        case COMMAND_SET_6P_CLEAR:
            // get preferred parent
            foundNeighbor =icmpv6rpl_getPreferredParentEui64(&neighbor);
            if (foundNeighbor==FALSE) {
                break;
            }

#if (SFMETHOD == SFMETHOD_SF0)
            sixtop_setHandler(SIX_HANDLER_SF0);
#endif
#if (SFMETHOD == SFMETHOD_SFLOC)
            sixtop_setHandler(SIX_HANDLER_SFLOC);
#endif
#if (SFMETHOD == SFMETHOD_NO)
            sixtop_setHandler(SIX_HANDLER_NONE);
#endif

            if ( 
                    (
                            commandId != COMMAND_SET_6P_ADD &&
                            commandId != COMMAND_SET_6P_DELETE
                ) ||
                (
                    ( 
                      commandId == COMMAND_SET_6P_ADD ||
                      commandId == COMMAND_SET_6P_DELETE
                    ) && 
                    commandLen == 0
                ) 
            ){

                // randomly select cell
                sixtop_request(commandId-8,&neighbor,1, sixtop_get_trackbesteffort(), NULL);
            } else {
                for (i=0;i<commandLen;i++){
                    cellList[i].tsNum           = openserial_vars.inputBuf[3+i];
                    cellList[i].choffset        = DEFAULT_CHANNEL_OFFSET;
                    cellList[i].linkoptions     = CELLTYPE_TX;
                }
                sixtop_addORremoveCellByInfo(commandId-8,&neighbor,cellList);
            }
            break;
       case COMMAND_SET_SLOTDURATION:
            ieee154e_setSlotDuration(comandParam_16);
            break;
       case COMMAND_SET_6PRESPONSE:
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
#if (SFMETHOD == SFMETHOD_SF0)
           sf0_appPktPeriod(comandParam_8);
#endif
#if (SFMETHOD == SFMETHOD_SFLOC)
           sfloc_appPktPeriod(comandParam_8);
#endif

            break;
       default:
           // wrong command ID
           break;
   }
}

//===== misc

void openserial_board_reset_cb(opentimer_id_t id) {
    board_reset();
}

/**
\brief Call this function before writing in the outputbuffer (to get the correct buffer to use).

\returns the ouput buffer we should now use
*/

//
uint8_t openserial_get_output_buffer(char *buffer, uint8_t length){
//   uint32_t length_filled;
   uint8_t  bufindex;
   uint8_t  i;
   uint8_t  length_escaped = 0;

#ifndef _DEBUG_OPENSERIAL_
   uint16_t _size_remain = 0;
#endif


   //compute the size of the buffer (with escape characters)
   for(i=0; i<length; i++)
      if (buffer[i]==HDLC_FLAG || buffer[i]==HDLC_ESCAPE)
         length_escaped += 2;
      else
         length_escaped += 1;
   length_escaped += 2;      // start frame delimiters (HDLC)
   length_escaped += 2;      // CRC (2B)

   //consider the current bufferindex
   bufindex = openserial_vars.outputCurrentW;

   //number of remainig cells from W to R
   //W=currently written by our module, R=Currently pushed (read by the serial line)
   if (openserial_vars.outputBufIdxW[bufindex] < openserial_vars.outputBufIdxR[bufindex])
      _size_remain = openserial_vars.outputBufIdxR[bufindex] - openserial_vars.outputBufIdxW[bufindex];
   else
      _size_remain = 255 - openserial_vars.outputBufIdxW[bufindex] + openserial_vars.outputBufIdxR[bufindex];

   //do we have enough space?
   if (_size_remain < length_escaped){

      //the next buffer is also filled -> not anymore space
      if ((openserial_vars.outputCurrentR == ((openserial_vars.outputCurrentW + 1) % SERIAL_OUTPUT_BUFFER_SIZE)) || (length_escaped > SERIAL_OUTPUT_BUFFER_SIZE))
         return(-1);

      //else, get the next buffer in the cycle
      if (openserial_vars.outputCurrentW == OPENSERIAL_OUTPUT_NBBUFFERS - 1)
         openserial_vars.outputCurrentW = 0;
      else
         (openserial_vars.outputCurrentW)++;

#ifdef _DEBUG_OPENSERIAL_
      _size_remain = SERIAL_OUTPUT_BUFFER_SIZE;
#endif
   }

#ifdef _DEBUG_OPENSERIAL_
      _size_towrite = length_escaped;
#endif

   return(openserial_vars.outputCurrentW);
}



//===== hdlc (output)

/**
\brief Start an HDLC frame in the output buffer.
*/
port_INLINE void outputHdlcOpen(uint8_t bufindex) {
    // initialize the value of the CRC
    openserial_vars.outputCrc[bufindex]                          = HDLC_CRCINIT;

    // write the opening HDLC flag
    openserial_vars.outputBuf[bufindex][openserial_vars.outputBufIdxW[bufindex]++]     = HDLC_FLAG;

}
/**
\brief Add a byte to the outgoing HDLC frame being built.
*/
port_INLINE void outputHdlcWrite(uint8_t bufindex, uint8_t b) {
    //buffer overflow: the last cell overwrites the first one!
    if (((uint8_t) openserial_vars.outputBufIdxW[bufindex] + 1) == openserial_vars.outputBufIdxR[bufindex]){


#ifdef _DEBUG_OPENSERIAL_
        openserial_printCritical(COMPONENT_OPENSERIAL, ERR_OPENSERIAL_BUFFER_OVERFLOW,
                (errorparameter_t)_size_remain,
                (errorparameter_t)_size_towrite);

        openserial_printCritical(COMPONENT_OPENSERIAL, ERR_GENERIC,
                (errorparameter_t)caller,
                (errorparameter_t)0);
#else
        openserial_printCritical(COMPONENT_OPENSERIAL, ERR_OPENSERIAL_BUFFER_OVERFLOW,
                (errorparameter_t)0,
                (errorparameter_t)255);

#endif

        return;
    }

    // iterate through CRC calculator
    openserial_vars.outputCrc[bufindex] = crcIteration(openserial_vars.outputCrc[bufindex],b);

    // add byte to buffer
    if (b==HDLC_FLAG || b==HDLC_ESCAPE) {
        openserial_vars.outputBuf[bufindex][openserial_vars.outputBufIdxW[bufindex]++]  = HDLC_ESCAPE;
        b                                               = b^HDLC_ESCAPE_MASK;
    }
    openserial_vars.outputBuf[bufindex][openserial_vars.outputBufIdxW[bufindex]++]     = b;
}
/**
\brief Finalize the outgoing HDLC frame.
*/
port_INLINE void outputHdlcClose(uint8_t bufindex) {
    uint16_t   finalCrc;

    // finalize the calculation of the CRC
    finalCrc   = ~openserial_vars.outputCrc[bufindex];

    // write the CRC value
    outputHdlcWrite(bufindex, (finalCrc>>0)&0xff);
    outputHdlcWrite(bufindex, (finalCrc>>8)&0xff);

    // write the closing HDLC flag
    openserial_vars.outputBuf[bufindex][openserial_vars.outputBufIdxW[bufindex]++]   = HDLC_FLAG;
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
    uint8_t bufindex;

    switch (openserial_vars.mode) {
        case MODE_INPUT:
            openserial_vars.reqFrameIdx++;
            if (openserial_vars.reqFrameIdx<sizeof(openserial_vars.reqFrame)) {
                uart_writeByte(openserial_vars.reqFrame[openserial_vars.reqFrameIdx]);
            }
            break;
        case MODE_OUTPUT:

                bufindex  = openserial_vars.outputCurrentR;

                //that's the end of this buffer
                if (openserial_vars.outputBufIdxW[bufindex]==openserial_vars.outputBufIdxR[bufindex]) {
                   openserial_vars.outputBufFilled[bufindex] = FALSE;

                   //considers the next buffer only if this one was entirely filled and the written one is the next one
                   if (openserial_vars.outputCurrentW != openserial_vars.outputCurrentR)
                      openserial_vars.outputCurrentR = (1 + openserial_vars.outputCurrentR) % OPENSERIAL_OUTPUT_NBBUFFERS;
                }

                //we push the next byte
                else if (openserial_vars.outputBufFilled[bufindex]) {
                   uart_writeByte(openserial_vars.outputBuf[bufindex][openserial_vars.outputBufIdxR[bufindex]++]);
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






//========= SERIAL FOR STATS ======





//a cell was inserted in the schedule
void openserial_statCelladd(scheduleEntry_t* slotContainer){

   #ifdef OPENSERIAL_STAT

   evtCellAdd_t evt;
   evt.track_instance   = slotContainer->track.instance;
   memcpy(evt.track_owner, slotContainer->track.owner.addr_64b, 8);
   evt.slotOffset      = slotContainer->slotOffset;
   evt.type            = slotContainer->type;
   evt.shared          = slotContainer->shared;
   evt.channelOffset   = slotContainer->channelOffset;
   memcpy(evt.neighbor,      slotContainer->neighbor.addr_64b, 8);
   openserial_printStat(SERTYPE_CELL_ADD, COMPONENT_IEEE802154E, (uint8_t*)&evt, sizeof(evt));
   #endif
}

//a cell was removed in the schedule
void openserial_statCellremove(scheduleEntry_t* slotContainer){

   #ifdef OPENSERIAL_STAT

   evtCellRem_t evt;
   evt.track_instance   = slotContainer->track.instance;
   memcpy(evt.track_owner, slotContainer->track.owner.addr_64b, 8);
   evt.slotOffset      = slotContainer->slotOffset;
   evt.type            = slotContainer->type;
   evt.shared          = slotContainer->shared;
   evt.channelOffset   = slotContainer->channelOffset;
   memcpy(evt.neighbor,      slotContainer->neighbor.addr_64b, 8);
   openserial_printStat(SERTYPE_CELL_REMOVE, COMPONENT_IEEE802154E, (uint8_t*)&evt, sizeof(evt));

   #endif
}


//a ack was txed
void openserial_statAckTx(void){

   #ifdef OPENSERIAL_STAT

   openserial_printStat(SERTYPE_ACK_TX, COMPONENT_IEEE802154E, NULL, 0);

   #endif
}

//a ack was received
void openserial_statAckRx(void){

    #ifdef OPENSERIAL_STAT

   openserial_printStat(SERTYPE_ACK_RX, COMPONENT_IEEE802154E, NULL, 0);

   #endif
}



//-------- FOR PKTS ------

//info for a transmitted packet
void openserial_fillPktTx(evtPktTx_t *evt, OpenQueueEntry_t* msg){
   evt->length           = msg->length;
   evt->txPower          = msg->l1_txPower;
   evt->track_instance   = msg->l2_track.instance;
   evt->numTxAttempts    = msg->l2_numTxAttempts;
   evt->frame_type       = msg->l2_frameType;
   evt->slotOffset       = schedule_getSlotOffset();
   evt->frequency        = calculateFrequency(schedule_getChannelOffset());
   evt->queuePos         = openqueue_getPos(msg);
   memcpy(evt->track_owner, msg->l2_track.owner.addr_64b, 8);
   memcpy(evt->l2Dest,      msg->l2_nextORpreviousHop.addr_64b, 8);
}

//info for a received packet
void openserial_fillPktRx(evtPktRx_t *evt, OpenQueueEntry_t* msg){
   evt->length           = msg->length;
   evt->rssi             = msg->l1_rssi;
   evt->lqi              = msg->l1_lqi;
   evt->crc              = msg->l1_crc;
   evt->track_instance   = msg->l2_track.instance;
   evt->frame_type       = msg->l2_frameType;
   evt->slotOffset       = schedule_getSlotOffset();
   evt->frequency        = calculateFrequency(schedule_getChannelOffset());
   evt->queuePos         = openqueue_getPos(msg);
   memcpy(evt->track_owner, msg->l2_track.owner.addr_64b, 8);
   memcpy(evt->l2Src, msg->l2_nextORpreviousHop.addr_64b, 8);

}



//push an event to track received frames (with a bad crc)
void openserial_statRxCrcFalse(OpenQueueEntry_t* msg){

   #if defined(OPENSERIAL_STAT) && defined(OPENSERIAL_STAT_FALSECRC_)
      evtPktRx_t evt;
      openserial_fillPktRx(&evt, msg);
      openserial_printStat(SERTYPE_PKT_RX, COMPONENT_IEEE802154E, (uint8_t*)&evt, sizeof(evtPktRx_t));
  #endif
}


//push an event to track received frames
void openserial_statRx(OpenQueueEntry_t* msg){

   #ifdef OPENSERIAL_STAT
      evtPktRx_t evt;
      openserial_fillPktRx(&evt, msg);
      openserial_printStat(SERTYPE_PKT_RX, COMPONENT_IEEE802154E, (uint8_t*)&evt, sizeof(evtPktRx_t));
  #endif
}

//push an event to track transmitted frames
void openserial_statTx(OpenQueueEntry_t* msg){

   #ifdef OPENSERIAL_STAT
      evtPktTx_t evt;
      openserial_fillPktTx(&evt, msg);
      openserial_printStat(SERTYPE_PKT_TX, msg->creator, (uint8_t*)&evt, sizeof(evtPktTx_t));
   #endif

}


//a ack has timeouted in openqueue
void openserial_statPktTimeout(OpenQueueEntry_t* msg){

   #ifdef OPENSERIAL_STAT
      evtPktTx_t evt;
      openserial_fillPktTx(&evt, msg);
      openserial_printStat(SERTYPE_PKT_TIMEOUT, msg->creator, (uint8_t*)&evt, sizeof(evtPktTx_t));
   #endif
}



//not enough space in openqueue for this data packet
void openserial_statPktBufferOverflow(OpenQueueEntry_t* msg){


   #if defined(OPENSERIAL_STAT) && defined(OPENSERIAL_STAT_OVERFLOW_)
      evtPktRx_t evt;
      openserial_fillPktRx(&evt, msg);
      openserial_printStat(SERTYPE_PKT_BUFFEROVERFLOW, msg->creator, (uint8_t*)&evt, sizeof(evtPktRx_t));
   #endif
}


//push an event to track an erroneous frame
void openserial_statPktError(OpenQueueEntry_t* msg){

   #ifdef OPENSERIAL_STAT
      evtPktTx_t evt;
      openserial_fillPktTx(&evt, msg);
      openserial_printStat(SERTYPE_PKT_ERROR, COMPONENT_IEEE802154E, (uint8_t*)&evt, sizeof(evtPktTx_t));
   #endif
}


//push an event to track generated frames
void openserial_statDataGen(uint32_t seqnum, track_t *track, open_addr_t *l3_destinationAdd, uint8_t queuePos){

#ifdef OPENSERIAL_STAT
   evtPktData_t   dataGen;
   open_addr_t    addr_64b, prefix;

   //error
   if (l3_destinationAdd->type != ADDR_128B){
       openserial_printError(COMPONENT_OPENSERIAL, ERR_WRONG_ADDR_TYPE, l3_destinationAdd->type, 12);
       return;
    }

   //info
   dataGen.seqnum          = seqnum ;
   dataGen.queuePos        = queuePos;
   dataGen.track_instance  =  track->instance;
   memcpy(dataGen.track_owner,   track->owner.addr_64b, 8);
   packetfunctions_ip128bToMac64b(l3_destinationAdd, &prefix, &addr_64b);
   memcpy(dataGen.l3Dest, addr_64b.addr_64b, 8);
   memcpy(&addr_64b, idmanager_getMyID(ADDR_64B), sizeof(open_addr_t));
   memcpy(dataGen.l3Source, addr_64b.addr_64b, 8);

   openserial_printStat(SERTYPE_DATA_GENERATION, COMPONENT_CEXAMPLE, (uint8_t*)&dataGen, sizeof(dataGen));
#endif
   /*
   dataGen.seqnum          = seqnum ;
   dataGen.queuePos        = openqueue_getPos(msg);
   dataGen.track_instance  =  msg->l2_track.instance;
   memcpy(dataGen.track_owner,   msg->l2_track.owner.addr_64b, 8);
   packetfunctions_ip128bToMac64b(&(msg->l3_destinationAdd), &prefix, &addr_64b);
   memcpy(dataGen.l3Dest, addr_64b.addr_64b, 8);
   memcpy(&addr_64b, idmanager_getMyID(ADDR_64B), sizeof(open_addr_t));
   memcpy(dataGen.l3Source, addr_64b.addr_64b, 8);
*/

}


//push an event to track generated frames
void openserial_statDataRx(uint32_t seqnum, track_t *track, open_addr_t *src, open_addr_t *dest){

   #ifdef OPENSERIAL_STAT
      evtPktData_t          dataRx;

      //wrong arguments
      if (src->type != ADDR_64B){
          openserial_printError(COMPONENT_OPENSERIAL, ERR_WRONG_ADDR_TYPE, src->type, 7);
          return;
       }
      if (dest->type != ADDR_64B){
          openserial_printError(COMPONENT_OPENSERIAL, ERR_WRONG_ADDR_TYPE, dest->type, 8);
          return;
       }

      //info
      dataRx.seqnum          = seqnum ;
      dataRx.track_instance  = track->instance;
      memcpy(dataRx.track_owner, track->owner.addr_64b, 8);
      memcpy(dataRx.l3Source, src->addr_64b, 8);
      memcpy(dataRx.l3Dest, dest->addr_64b, 8);

      openserial_printStat(SERTYPE_DATA_RX, COMPONENT_CEXAMPLE, (uint8_t*)&dataRx, sizeof(dataRx));
   #endif

}


//push an event to track DIO transmissions
void openserial_statDIOtx(void){

   #ifdef OPENSERIAL_STAT
       openserial_printStat(SERTYPE_DIOTX, COMPONENT_ICMPv6RPL, NULL, 0);
   #endif

}

//push an event to track DAO transmissions
void openserial_statDAOtx(uint8_t *parent){

   #ifdef OPENSERIAL_STAT
      evtDaOTx_t          evt;
      //info
      memcpy(evt.parent, parent, 8);

      openserial_printStat(SERTYPE_DAOTX, COMPONENT_ICMPv6RPL, (uint8_t*)&evt, sizeof(evt));
   #endif

}



/***********************************************
 *    FUNCTION TOOLS
 **********************************************/



//append a uint8_t at the end of a string
char *openserial_ncat_uint8_t(char *str, uint8_t val, uint8_t length){
   uint8_t l = strlen(str);

   if (l + 3 > length)
      return(str);

   uint8_t a, b, c;

   a = val / 100;
   b = (val - a * 100)/10;
   c = val - a * 100 - b * 10;

   if (a != 0)
       str[l++] = '0' + a;
   if (b != 0 || a != 0)
       str[l++] = '0' + b;
   str[l++] = '0' + c;
   str[l++] = '\0';
   return(str);
}


//append a uint32_t at the end of a string (without the non significant zeros)
char *openserial_ncat_uint32_t(char *str, uint32_t val, uint8_t length){
   uint8_t l = strlen(str);

   if (l + 10 > length) //at most 10 digits
      return(str);

   uint8_t  digit, shift, i;
   uint32_t power;
   bool     nonzero = FALSE;


   power = 1000000000;
   shift = 0;           // to avoid non significant zeros
   for(i=0; i<10; i++){
      digit = val / power;
      if (digit != 0 || i == 9 || nonzero){
         nonzero = TRUE;
         str[l + shift] = '0' + digit;
         shift++;
      }
      val = val - power * digit;
      power = power / 10;
   }
   str[l+shift] = '\0';


   return(str);
}




//append a uint32_t at the end of a string (without the non significant zeros)
char *openserial_ncat_uint8_t_hex(char *str, uint8_t val, uint8_t length){
   uint8_t  l = strlen(str);
   uint8_t  c, shift;

   if (l + 2 > length) //at most 2 digits
      return(str);


   shift = 0;

   //first digit
   c = (val & 0xf0)  >> 4;


   if (c < 10)
      str[l+shift] = '0' + c;
   else if (c < 16)
      str[l+shift] = (uint8_t)'a' + c  - 10;
   else
      str[l+shift] = 'z';
   shift++;


   //second digit
   c = val & 0x0f;
   if (c < 10)
      str[l+shift] = '0' + c;
   else if (c < 16)
      str[l+shift] = 'a' + c  - 9;
   else
      str[l+shift] = 'z';
   shift++;


   str[l+shift] = '\0';

   return(str);
}
