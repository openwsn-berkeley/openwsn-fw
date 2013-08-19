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
#include "xbee_app.h"

#define XBEE_API_INIT 0x7e
#define XBEE_API_ESCAPE 0x7d
#define XBEE_API_ESCAPE_MASK     0x20

// XBEE definitions
enum {
  API_MODEM_STATUS = 0x8A,
  API_AT_COMMAND = 0x08,
  API_AT_RESPONSE = 0x88,
  API_TX_REQUEST_64 = 0x00,
  API_TX_REQUEST_16 = 0x01,
  API_TX_STATUS = 0x89,
  API_RX_PACKET_64 = 0x80,
  API_RX_PACKET_16 = 0x81
} ;


/// Modes of the openserial module.
enum { 
  MODE_OFF    = 0, ///< The module is off, no serial activity.
  MODE_INPUT  = 1, ///< The serial is listening or receiving bytes.
  MODE_OUTPUT = 2  ///< The serial is transmitting bytes.
};


//=========================== module variables ================================

typedef struct {
  // admin
  uint8_t    mode;
  // input
  bool       busyReceiving;
  bool       inputEscaping;
  uint16_t   inputChecksum;
  uint16_t    inputBufFill;
  uint16_t     inputLengthExpected;
  uint8_t     lengthBytesRecieved;
  uint8_t    inputBuf[SERIAL_INPUT_BUFFER_SIZE];
  // output
  bool       outputBufFilled;
  uint8_t   outputChecksum;
  uint16_t    outputBufIdxW;
  uint16_t    outputBufIdxR;
  uint8_t    outputBuf[SERIAL_OUTPUT_BUFFER_SIZE];
} openserial_vars_t;


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
// API output
void outputAPIOpen();
void outputAPIWrite(uint8_t b);
void outputAPIClose();
// API input
void inputAPIWrite(uint8_t b);

//=========================== public ==========================================

void openserial_init() {
  // reset variable
  memset(&openserial_vars,0,sizeof(openserial_vars_t));
  
  // admin
  openserial_vars.mode                = MODE_OFF;
  
  // input
  openserial_vars.busyReceiving       = FALSE;
  openserial_vars.inputEscaping       = FALSE;
  openserial_vars.inputBufFill        = 0;
  
  // ouput
  openserial_vars.outputBufFilled     = FALSE;
  openserial_vars.outputBufIdxR       = 0;
  openserial_vars.outputBufIdxW       = 0;
  
#ifdef UART_USE_CTS
  uart_cts_set(); // not ready to recieve
#endif
  
  // set callbacks
  uart_setCallbacks(isr_openserial_tx,
                    isr_openserial_rx);
}

owerror_t openserial_printStatus(uint8_t statusElement,uint8_t* buffer, uint8_t length) {
  return E_SUCCESS;
}

owerror_t openserial_printData(uint8_t* buffer, uint8_t length) {
  return E_SUCCESS;
}

owerror_t openserial_printInfo(uint8_t calling_component, uint8_t error_code,
                               errorparameter_t arg1,
                               errorparameter_t arg2) {
                                 return E_SUCCESS;
                               }

owerror_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                                errorparameter_t arg1,
                                errorparameter_t arg2) {
                                  // blink error LED, this is serious
                                  leds_error_toggle();
                                  
                                  return E_SUCCESS;
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
                                     
                                     return E_SUCCESS;
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
  ENABLE_INTERRUPTS();
  
#ifdef UART_USE_CTS
  uart_cts_clr(); // indicate ready to recieve
#endif
}

void openserial_startOutput() {
  //schedule a task to get new status in the output buffer
  
  // flush buffer
  uart_clearTxInterrupts();
  uart_clearRxInterrupts();          // clear possible pending interrupts
  uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt
  
  // start pointers at 0
  openserial_vars.outputBufIdxR = openserial_vars.outputBufIdxW = 0;
  
  // fill the output buffer now
  
  
  DISABLE_INTERRUPTS();
  openserial_vars.mode=MODE_OUTPUT;
  if (openserial_vars.outputBufFilled) {
    uart_writeByte(openserial_vars.outputBuf[openserial_vars.outputBufIdxR++]);
  } else {
    openserial_stop();
  }
  ENABLE_INTERRUPTS();
}

void openserial_stop() {
  uint8_t inputBufFill;
  uint8_t cmdByte;
  uint8_t frameID;
  uint8_t txoptions;
  uint8_t AT_Cmd[2];
  uint16_t cmdlen;
  
  bool busyReceiving;
  INTERRUPT_DECLARATION();
  
#ifdef UART_USE_CTS
  uart_cts_set(); // no longer can recieve data
#endif  
  
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
    // interpret packet
    DISABLE_INTERRUPTS();
    cmdByte = openserial_vars.inputBuf[0];
    ENABLE_INTERRUPTS();
    
    switch(cmdByte) {
    case API_AT_COMMAND:
      DISABLE_INTERRUPTS();
      frameID = openserial_vars.inputBuf[1];
      AT_Cmd[0] = openserial_vars.inputBuf[2];
      AT_Cmd[1] = openserial_vars.inputBuf[3];
      cmdlen = openserial_vars.inputLengthExpected - 3;
      ENABLE_INTERRUPTS();
      if (cmdlen == 0 ) {
          at_command_get(AT_Cmd);
      } else {
          at_command_set(AT_Cmd,&(openserial_vars.inputBuf[4]),cmdlen);
      }
      break;
    case API_TX_REQUEST_64:
      DISABLE_INTERRUPTS();
      frameID = openserial_vars.inputBuf[1];
      txoptions = openserial_vars.inputBuf[10];
      ENABLE_INTERRUPTS();
      break;
    case API_TX_REQUEST_16:
      break;
    default: // not supported        
    }
    
  }
  
  DISABLE_INTERRUPTS();
  openserial_vars.inputBufFill  = 0;
  openserial_vars.busyReceiving = FALSE;
  ENABLE_INTERRUPTS();
}


//=========================== private =========================================

//===== api (output)

/**
\brief Start an API frame in the output buffer.
*/
inline void outputAPIOpen() {
  // initialize the value of the checksum
  openserial_vars.outputChecksum                          = 0xFF;
  
  // write the opening API flag
  openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]     = XBEE_API_INIT;
  
  // reserve space for length vars, not included in checksum
  openserial_vars.outputBufIdxW += 2;
}
/**
\brief Add a byte to the outgoing API frame being built.

\todo escape 0x7e and 0x7d.
*/
inline void outputAPIWrite(uint8_t b) {
  if ( openserial_vars.outputBufIdxW >= SERIAL_OUTPUT_BUFFER_SIZE - 1) 
  {
    leds_error_toggle();
    return;// overflow!
  }
  
  // iterate through checksum calculator
  openserial_vars.outputChecksum  -= b;
  
  // add byte to buffer
  if (b==XBEE_API_INIT || b==XBEE_API_ESCAPE || b == 0x11 || b == 0x13) {
    openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]  = XBEE_API_ESCAPE;
    b                                               = b^XBEE_API_ESCAPE_MASK;
  }
  openserial_vars.outputBuf[openserial_vars.outputBufIdxW++]     = b;
  
  openserial_vars.outputBufFilled = TRUE;
}
/**
\brief Finalize the outgoing API frame.
*/
inline void outputAPIClose() {
  uint16_t  temp_len;
  // write the current checksum value
  outputAPIWrite((openserial_vars.outputChecksum)&0xff);
  // outputBufIdxW will be size of the buf including checksum, length, and start delimeter, a total of 4 bytes
  
  temp_len = openserial_vars.outputBufIdxW - 4;
  openserial_vars.outputBufIdxW = 1; // write to length fields
  outputAPIWrite((temp_len>>8)&0xff);
  outputAPIWrite(temp_len&0xff);
  openserial_vars.outputBufIdxW = temp_len + 4;
}

//===== api (input)

/**
\brief Add a byte to the incoming API frame.

\todo escape 0x7e and 0x7d.
*/
inline void inputAPIWrite(uint8_t b) {
  if (b==XBEE_API_ESCAPE) {
    openserial_vars.inputEscaping = TRUE;
  } else {
    if (openserial_vars.inputEscaping==TRUE) {
      b                             = b^XBEE_API_ESCAPE_MASK;
      openserial_vars.inputEscaping = FALSE;
    }
    
    // add byte to input buffer
    openserial_vars.inputBuf[openserial_vars.inputBufFill] = b;
    openserial_vars.inputBufFill++;
    
    // iterate through CRC calculator
    openserial_vars.inputChecksum += b;
  }
}

//=========================== interrupt handlers ==============================

//executed in ISR, called from scheduler.c
void isr_openserial_tx() {
  switch (openserial_vars.mode) {
  case MODE_OUTPUT:
    if (openserial_vars.outputBufIdxW==openserial_vars.outputBufIdxR) {
      openserial_vars.outputBufFilled = FALSE;
    }
    if (openserial_vars.outputBufFilled) {
      uart_writeByte(openserial_vars.outputBuf[openserial_vars.outputBufIdxR++]);
    }
    break;
  case MODE_OFF:
  case MODE_INPUT:
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
  //keep length
  inputBufFill=openserial_vars.inputBufFill;
  
  // start of frame no matter what (discard partial frame if necessary)
  if  ( rxbyte == XBEE_API_INIT ) {
    // start of frame
    // I'm now receiving
    openserial_vars.busyReceiving         = TRUE;
    
    // create the API frame
    // reset the input buffer index
    openserial_vars.inputBufFill                       = 0;
    openserial_vars.lengthBytesRecieved                = 0;
    openserial_vars.inputLengthExpected = 0;
    
    // initialize the value of the CRC
    openserial_vars.inputChecksum                           = 0;
  } else if ( openserial_vars.busyReceiving == TRUE ) {
    if ( openserial_vars.lengthBytesRecieved < 2 ) { // length bytes recieving
      openserial_vars.inputLengthExpected |= ((uint16_t)rxbyte << (8*(0==openserial_vars.lengthBytesRecieved)));
      openserial_vars.lengthBytesRecieved++;
    } else if ( openserial_vars.inputBufFill < openserial_vars.inputLengthExpected - 1) { // normal bytes
      // middle of frame
      
      // add the byte just received
      inputAPIWrite(rxbyte);
      if (openserial_vars.inputBufFill+1>SERIAL_INPUT_BUFFER_SIZE){
        // input buffer overflow
        openserial_vars.inputBufFill       = 0;
        openserial_vars.busyReceiving      = FALSE;
        openserial_stop();
      }
    } else {   // end of frame, rxbyte contains CRC
      
      // finalize the API frame
      
      // verify the validity of the frame
      if (openserial_vars.inputChecksum + rxbyte != 0xFF) {         // the CRC is incorrect, drop the incoming frame
        openserial_vars.inputBufFill     = 0;
        leds_error_toggle();
      } 
      
      openserial_vars.busyReceiving      = FALSE;
      openserial_stop();
    }
  }
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
