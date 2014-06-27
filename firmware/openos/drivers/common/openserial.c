/**
\brief Definition of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
*/

#include "openwsn.h"
#include "openserial.h"
#include "IEEE802154E.h"
#include "neighbors.h"
#include "res.h"
#include "icmpv6echo.h"
#include "idmanager.h"
#include "openqueue.h"
#include "tcpinject.h"
#include "udpinject.h"
#include "openbridge.h"
#include "leds.h"
#include "schedule.h"
#include "uart.h"
#include "opentimers.h"
#include "openhdlc.h"

//=========================== variables =======================================

openserial_vars_t openserial_vars;

//=========================== prototypes ======================================

owerror_t openserial_printInfoErrorCritical(
   char             severity,
   uint8_t          calling_component,
   uint8_t          error_code,
   errorparameter_t arg1,
   errorparameter_t arg2
);
// HDLC output
void outputHdlcOpen(void);
void outputHdlcWrite(uint8_t b);
void outputHdlcClose(void);
// HDLC input
void inputHdlcOpen(void);
void inputHdlcWrite(uint8_t b);
void inputHdlcClose(void);

//=========================== public ==========================================

void openserial_init() {
   uint16_t crc;
   
   // reset variable
   memset(&openserial_vars,0,sizeof(openserial_vars_t));
   
   // admin
   openserial_vars.mode                = MODE_OFF;
   openserial_vars.debugPrintCounter   = 0;
   
   // input
   openserial_vars.reqFrame[0]         = HDLC_FLAG;
   openserial_vars.reqFrame[1]         = SERFRAME_MOTE2PC_REQUEST;
   crc = HDLC_CRCINIT;
   crc = crcIteration(crc,openserial_vars.reqFrame[1]);
   crc = ~crc;
   openserial_vars.reqFrame[2]         = (crc>>0)&0xff;
   openserial_vars.reqFrame[3]         = (crc>>8)&0xff;
   openserial_vars.reqFrame[4]         = HDLC_FLAG;
   openserial_vars.reqFrameIdx         = 0;
   openserial_vars.lastRxByte          = HDLC_FLAG;
   openserial_vars.busyReceiving       = FALSE;
   openserial_vars.inputEscaping       = FALSE;
   openserial_vars.inputBufFill        = 0;
   
   // ouput
   openserial_vars.outputBufFilled     = FALSE;
   openserial_vars.outputBufIdxR       = 0;
   openserial_vars.outputBufIdxW       = 0;
   
   // set callbacks
   uart_setCallbacks(isr_openserial_tx,
                     isr_openserial_rx);
}

owerror_t openserial_printStatus(uint8_t statusElement,uint8_t* buffer, uint8_t length) {
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

owerror_t openserial_printData(uint8_t* buffer, uint8_t length) {
   uint8_t  i;
   uint8_t  asn[5];
   INTERRUPT_DECLARATION();
   
   // retrieve ASN
   ieee154e_getAsn(asn);// byte01,byte23,byte4
   
   DISABLE_INTERRUPTS();
   openserial_vars.outputBufFilled  = TRUE;
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
   ENABLE_INTERRUPTS();
   
   return E_SUCCESS;
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
   leds_error_toggle();
   
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
                    board_reset);
   
   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_CRITICAL,
      calling_component,
      error_code,
      arg1,
      arg2
   );
}

uint8_t openserial_getNumDataBytes() {
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
      openserial_printError(COMPONENT_OPENSERIAL,ERR_GETDATA_ASKS_TOO_FEW_BYTES,
                            (errorparameter_t)maxNumBytes,
                            (errorparameter_t)inputBufFill-1);
      numBytesWritten = 0;
   } else {
      numBytesWritten = inputBufFill-1;
      memcpy(bufferToWrite,&(openserial_vars.inputBuf[1]),numBytesWritten);
   }
   
   return numBytesWritten;
}

void openserial_startInput() {
   INTERRUPT_DECLARATION();
   
   if (openserial_vars.inputBufFill>0) {
      openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)openserial_vars.inputBufFill,
                            (errorparameter_t)0);
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

void openserial_startOutput() {
   //schedule a task to get new status in the output buffer
   uint8_t debugPrintCounter;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   openserial_vars.debugPrintCounter = (openserial_vars.debugPrintCounter+1)%STATUS_MAX;
   debugPrintCounter = openserial_vars.debugPrintCounter;
   ENABLE_INTERRUPTS();
   
   // print debug information
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
      default:
         DISABLE_INTERRUPTS();
         openserial_vars.debugPrintCounter=0;
         ENABLE_INTERRUPTS();
   }
   
   // flush buffer
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
      uart_writeByte(openserial_vars.outputBuf[openserial_vars.outputBufIdxR++]);
#endif
   } else {
      openserial_stop();
   }
   ENABLE_INTERRUPTS();
}

void openserial_stop() {
   uint8_t inputBufFill;
   uint8_t cmdByte;
   bool busyReceiving;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   busyReceiving = openserial_vars.busyReceiving;
   inputBufFill = openserial_vars.inputBufFill;
   ENABLE_INTERRUPTS();
   
   // disable USCI_A1 TX & RX interrupt
   uart_disableInterrupts();
   
   DISABLE_INTERRUPTS();
   openserial_vars.mode=MODE_OFF;
   ENABLE_INTERRUPTS();
   //the inputBuffer has to be reset if it is not reset where the data is read.
   //or the function openserial_getInputBuffer is called (which resets the buffer)
   if (busyReceiving==TRUE){
      openserial_printError(COMPONENT_OPENSERIAL,ERR_BUSY_RECEIVING,
                                  (errorparameter_t)0,
                                  (errorparameter_t)inputBufFill);
   }
   
   if (busyReceiving == FALSE && inputBufFill>0) {
      DISABLE_INTERRUPTS();
      cmdByte = openserial_vars.inputBuf[0];
      ENABLE_INTERRUPTS();
      switch (cmdByte) {
         case SERFRAME_PC2MOTE_SETROOT:
            idmanager_triggerAboutRoot();
            break;
         case SERFRAME_PC2MOTE_SETBRIDGE:
            idmanager_triggerAboutBridge();
            break;
         case SERFRAME_PC2MOTE_DATA:
            openbridge_triggerData();
            break;
         case SERFRAME_PC2MOTE_TRIGGERTCPINJECT:
            tcpinject_trigger();
            break;
         case SERFRAME_PC2MOTE_TRIGGERUDPINJECT:
            udpinject_trigger();
            break;
         case SERFRAME_PC2MOTE_TRIGGERICMPv6ECHO:
            icmpv6echo_trigger();
            break;
         case SERFRAME_PC2MOTE_TRIGGERSERIALECHO:
            //echo function must reset input buffer after reading the data.
            openserial_echo(&openserial_vars.inputBuf[1],inputBufFill-1);
            break;   
         default:
            openserial_printError(COMPONENT_OPENSERIAL,ERR_UNSUPPORTED_COMMAND,
                                  (errorparameter_t)cmdByte,
                                  (errorparameter_t)0);
            //reset here as it is not being reset in any other callback
            DISABLE_INTERRUPTS();
            openserial_vars.inputBufFill = 0;
            ENABLE_INTERRUPTS();
            break;
      }
   }
   
   DISABLE_INTERRUPTS();
   openserial_vars.inputBufFill  = 0;
   openserial_vars.busyReceiving = FALSE;
   ENABLE_INTERRUPTS();
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_outBufferIndexes() {
   uint16_t temp_buffer[2];
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   temp_buffer[0] = openserial_vars.outputBufIdxW;
   temp_buffer[1] = openserial_vars.outputBufIdxR;
   ENABLE_INTERRUPTS();
   openserial_printStatus(STATUS_OUTBUFFERINDEXES,(uint8_t*)temp_buffer,sizeof(temp_buffer));
   return TRUE;
}

//=========================== private =========================================

//===== hdlc (output)

/**
\brief Start an HDLC frame in the output buffer.
*/
inline void outputHdlcOpen() {
   // initialize the value of the CRC
   openserial_vars.outputCrc                          = HDLC_CRCINIT;
   
   // write the opening HDLC flag
   openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]     = HDLC_FLAG;
}
/**
\brief Add a byte to the outgoing HDLC frame being built.
*/
inline void outputHdlcWrite(uint8_t b) {
   
   // iterate through CRC calculator
   openserial_vars.outputCrc = crcIteration(openserial_vars.outputCrc,b);
   
   // add byte to buffer
   if (b==HDLC_FLAG || b==HDLC_ESCAPE) {
      openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]  = HDLC_ESCAPE;
      b                                               = b^HDLC_ESCAPE_MASK;
   }
   openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]     = b;
   
}
/**
\brief Finalize the outgoing HDLC frame.
*/
inline void outputHdlcClose() {
   uint16_t   finalCrc;
    
   // finalize the calculation of the CRC
   finalCrc   = ~openserial_vars.outputCrc;
   
   // write the CRC value
   outputHdlcWrite((finalCrc>>0)&0xff);
   outputHdlcWrite((finalCrc>>8)&0xff);
   
   // write the closing HDLC flag
   openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]   = HDLC_FLAG;
}

//===== hdlc (input)

/**
\brief Start an HDLC frame in the input buffer.
*/
inline void inputHdlcOpen() {
   // reset the input buffer index
   openserial_vars.inputBufFill                       = 0;
   
   // initialize the value of the CRC
   openserial_vars.inputCrc                           = HDLC_CRCINIT;
}
/**
\brief Add a byte to the incoming HDLC frame.
*/
inline void inputHdlcWrite(uint8_t b) {
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
inline void inputHdlcClose() {
   
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

//executed in ISR, called from scheduler.c
void isr_openserial_tx() {
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
            uart_writeByte(openserial_vars.outputBuf[openserial_vars.outputBufIdxR++]);
         }
         break;
      case MODE_OFF:
      default:
         break;
   }
}

// executed in ISR, called from scheduler.c
void isr_openserial_rx() {
   uint8_t rxbyte;
   uint8_t inputBufFill;
   
   // stop if I'm not in input mode
   if (openserial_vars.mode!=MODE_INPUT) {
      return;
   }
   
   // read byte just received
   rxbyte = uart_readByte();
   //keep lenght
   inputBufFill=openserial_vars.inputBufFill;
   
   if        (
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
         openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUT_BUFFER_OVERFLOW,
                               (errorparameter_t)0,
                               (errorparameter_t)0);
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
            openserial_printError(COMPONENT_OPENSERIAL,ERR_WRONG_CRC_INPUT,
                                  (errorparameter_t)inputBufFill,
                                  (errorparameter_t)0);
         
         }
         
         openserial_vars.busyReceiving      = FALSE;
         openserial_stop();
   }
   
   openserial_vars.lastRxByte = rxbyte;
}

//======== SERIAL ECHO =============

void openserial_echo(uint8_t* buf, uint8_t bufLen){
   INTERRUPT_DECLARATION();
   // echo back what you received
   openserial_printData(
      buf,
      bufLen
   );
   
    DISABLE_INTERRUPTS();
    openserial_vars.inputBufFill = 0;
    ENABLE_INTERRUPTS();
}
