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
#include "icmpv6router.h"
#include "idmanager.h"
#include "openqueue.h"
#include "tcpinject.h"
#include "udpinject.h"
#include "openbridge.h"
#include "leds.h"
#include "schedule.h"
#include "uart.h"
#include "opentimers.h"
#include "serialecho.h"
#include "hdlcserial.h"

//=========================== variables =======================================

typedef struct {
   // admin
   uint8_t    mode;
   uint8_t    debugPrintCounter;
   // input
   uint8_t    input_command[1+1+2+1]; // flag (1B), command (2B), CRC (1B), flag (1B)
   uint8_t    input_command_index;
   uint8_t    input_buffer[SERIAL_INPUT_BUFFER_SIZE];
   uint16_t   input_buffer_fill_level;
   uint8_t    input_buffer_bytes_still_to_come;
   bool       receiving;
   bool       ready_receive_command;
   uint8_t    received_command;
   // output
   uint8_t    output_buffer[SERIAL_OUTPUT_BUFFER_SIZE];
   uint8_t    output_buffer_index_read;
   uint8_t    output_buffer_index_write;
   bool       somethingInOutputBuffer;
} openserial_vars_t;

openserial_vars_t openserial_vars;

//=========================== prototypes ======================================

uint16_t output_buffer_index_write_increment();
uint16_t output_buffer_index_read_increment();
error_t  openserial_printInfoErrorCritical(
   char             severity,
   uint8_t          calling_component,
   uint8_t          error_code,
   errorparameter_t arg1,
   errorparameter_t arg2
);

//=========================== public ==========================================

void openserial_init() {
   //initialize variables
   openserial_vars.input_command[0]              = SERFRAME_MOTE2PC_REQUEST;
   hdlcify(openserial_vars.input_command,1);

   openserial_vars.output_buffer_index_read      = -1;
   openserial_vars.output_buffer_index_write     = -1;
   openserial_vars.somethingInOutputBuffer       = FALSE;
   openserial_vars.mode                          = MODE_OFF;
   openserial_vars.receiving                     = FALSE;
   openserial_vars.input_buffer_fill_level       = 0;
   
   // set callbacks
   uart_setCallbacks(isr_openserial_tx,
                     isr_openserial_rx);
}

error_t openserial_printStatus(uint8_t statusElement,uint8_t* buffer, uint16_t length) {
   uint8_t counter;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   openserial_vars.somethingInOutputBuffer=TRUE;
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=SERFRAME_MOTE2PC_STATUS;
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[0]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[1]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)statusElement;
   for (counter=0;counter<length;counter++){
      openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)buffer[counter];
   }
   counter = hdlcify(openserial_vars.output_buffer,openserial_vars.output_buffer_index_write+1);
   openserial_vars.output_buffer_index_write = counter-1;
   ENABLE_INTERRUPTS();
   
   return E_SUCCESS;
}

error_t openserial_printInfo(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_INFO, // severity
      calling_component,
      error_code,
      arg1,
      arg2
   );
}

error_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   // blink error LED, this is serious
   leds_error_toggle();
   
   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_ERROR, // severity
      calling_component,
      error_code,
      arg1,
      arg2
   );
}

error_t openserial_printCritical(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   // blink error LED, this is serious
   leds_error_blink();
   
   // schedule for the mote to reboot in 10s
   opentimers_start(10000,
                    TIMER_ONESHOT,TIME_MS,
                    board_reset);
   
   return openserial_printInfoErrorCritical(
      SERFRAME_MOTE2PC_CRITICAL, // severity
      calling_component,
      error_code,
      arg1,
      arg2
   );
}

error_t openserial_printData(uint8_t* buffer, uint8_t length) {
   uint8_t counter;
   uint8_t asn[5];
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   openserial_vars.somethingInOutputBuffer=TRUE;
   
   // type
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=SERFRAME_MOTE2PC_DATA;
   // addr16b
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[1]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[0]);
   // asn
   asnWriteToSerial(asn);// byte01,byte23,byte4
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=asn[0];
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=asn[1];
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=asn[2];
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=asn[3];
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=asn[4];
   // payload
   for (counter=0;counter<length;counter++){
      openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)buffer[counter];
   }
   
   counter = hdlcify(openserial_vars.output_buffer,openserial_vars.output_buffer_index_write+1);
   openserial_vars.output_buffer_index_write = counter-1;
   ENABLE_INTERRUPTS();
   
   return E_SUCCESS;
}

uint8_t openserial_getNumDataBytes() {
   uint16_t temp_openserial_input_buffer_fill_level=0;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   temp_openserial_input_buffer_fill_level = openserial_vars.input_buffer_fill_level;
   ENABLE_INTERRUPTS();

   return temp_openserial_input_buffer_fill_level;
}

uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes) {
   uint8_t numBytesWritten=0;
   uint16_t temp_openserial_input_buffer_fill_level;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   temp_openserial_input_buffer_fill_level = openserial_vars.input_buffer_fill_level;
   ENABLE_INTERRUPTS();
   if (maxNumBytes<temp_openserial_input_buffer_fill_level) {
      openserial_printError(COMPONENT_OPENSERIAL,ERR_GETDATA_ASKS_TOO_FEW_BYTES,
                            (errorparameter_t)maxNumBytes,
                            (errorparameter_t)temp_openserial_input_buffer_fill_level);
      numBytesWritten = 0;
   } else {
      numBytesWritten = temp_openserial_input_buffer_fill_level;
      memcpy(bufferToWrite,&(openserial_vars.input_buffer[0]),numBytesWritten);
   }
   DISABLE_INTERRUPTS();
   openserial_vars.input_buffer_fill_level=0;
   ENABLE_INTERRUPTS();
   return numBytesWritten;
}

void openserial_startInput() {
   INTERRUPT_DECLARATION();
   if (openserial_vars.input_buffer_fill_level>0) {
      openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)openserial_vars.input_buffer_fill_level,
                            (errorparameter_t)0);
      openserial_vars.input_buffer_fill_level = 0;
   }
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();          // clear possible pending interrupts
   uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt
   DISABLE_INTERRUPTS();
   openserial_vars.mode                  = MODE_INPUT;
   openserial_vars.input_command_index   = 0;
   openserial_vars.ready_receive_command = FALSE;
   uart_writeByte(openserial_vars.input_command[openserial_vars.input_command_index]);
   ENABLE_INTERRUPTS();
}

void openserial_startOutput() {
   //schedule a task to get new status in the output buffer
   uint8_t temp_openserial_debugPrintCounter; //to avoid many atomics
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   openserial_vars.debugPrintCounter=(openserial_vars.debugPrintCounter+1)%STATUS_MAX;
   temp_openserial_debugPrintCounter = openserial_vars.debugPrintCounter;
   ENABLE_INTERRUPTS();
   switch (temp_openserial_debugPrintCounter) {
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
   //print out what's in the buffer now
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();          // clear possible pending interrupts
   uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt
   DISABLE_INTERRUPTS();
   openserial_vars.mode=MODE_OUTPUT;
   if (openserial_vars.somethingInOutputBuffer) {
      openserial_vars.output_buffer_index_read = -1;
      uart_writeByte(openserial_vars.output_buffer[output_buffer_index_read_increment()]);
   } else {
      openserial_stop();
   }
   ENABLE_INTERRUPTS();
}

void openserial_stop() {
   uint16_t temp_openserial_input_buffer_fill_level;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   temp_openserial_input_buffer_fill_level = openserial_vars.input_buffer_fill_level;
   ENABLE_INTERRUPTS();
   uart_disableInterrupts();              // disable USCI_A1 TX & RX interrupt
   DISABLE_INTERRUPTS();
   openserial_vars.mode=MODE_OFF;
   ENABLE_INTERRUPTS();
   if (temp_openserial_input_buffer_fill_level>0) {
      uint8_t temp_openserial_received_command;
      DISABLE_INTERRUPTS();
      temp_openserial_received_command = openserial_vars.received_command;
      ENABLE_INTERRUPTS();
      switch (temp_openserial_received_command) {
         case 'R': //Trigger IDManager about isRoot
            idmanager_triggerAboutRoot();
            break;
         case 'B': //Trigger IDManager about isBridge
            idmanager_triggerAboutBridge();
            break;
         case 'T': //Trigger TCPInject
            tcpinject_trigger();
            break;
         case 'U': //Trigger UDPInject
            udpinject_trigger();
            break;
         case 'E': //Trigger ICMPv6Echo
            icmpv6echo_trigger();
            break;
         case 'P': //Trigger ICMPv6RPL
            icmpv6rpl_trigger();
            break;
         case 'D': //Trigger OpenBridge (called only by moteProbe)
            openbridge_trigger();
            break;
         case 'H': //Trigger serial echo
            serialecho_echo();
            break;   
         default:
            openserial_printError(COMPONENT_OPENSERIAL,ERR_UNSUPPORTED_COMMAND,
                                  (errorparameter_t)temp_openserial_received_command,
                                  (errorparameter_t)0);
            DISABLE_INTERRUPTS();
            openserial_vars.input_buffer_fill_level = 0;
            ENABLE_INTERRUPTS();
            break;
      }
   }
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
   temp_buffer[0] = openserial_vars.output_buffer_index_write;
   temp_buffer[1] = openserial_vars.output_buffer_index_read;
   ENABLE_INTERRUPTS();
   openserial_printStatus(STATUS_OUTBUFFERINDEXES,(uint8_t*)temp_buffer,sizeof(temp_buffer));
   return TRUE;
}

//=========================== private =========================================

uint16_t output_buffer_index_write_increment() {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   openserial_vars.output_buffer_index_write=(openserial_vars.output_buffer_index_write+1)%SERIAL_OUTPUT_BUFFER_SIZE;
   ENABLE_INTERRUPTS();
   return openserial_vars.output_buffer_index_write;  
}

uint16_t output_buffer_index_read_increment() {
   uint16_t temp_openserial_output_buffer_index_read=0;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   openserial_vars.output_buffer_index_read=(openserial_vars.output_buffer_index_read+1)%SERIAL_OUTPUT_BUFFER_SIZE;
   temp_openserial_output_buffer_index_read = openserial_vars.output_buffer_index_read;
   ENABLE_INTERRUPTS();
   return temp_openserial_output_buffer_index_read;
}

error_t openserial_printInfoErrorCritical(
      char             severity,
      uint8_t          calling_component,
      uint8_t          error_code,
      errorparameter_t arg1,
      errorparameter_t arg2
   ) {
   uint8_t counter;
   
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   openserial_vars.somethingInOutputBuffer=TRUE;
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=severity;                      // severity indicator
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[0]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[1]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)calling_component;    // component generating error
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)error_code;           // error_code
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((arg1 & 0xff00)>>8); // arg1
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t) (arg1 & 0x00ff);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((arg2 & 0xff00)>>8); // arg2
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t) (arg2 & 0x00ff);
   counter = hdlcify(openserial_vars.output_buffer,openserial_vars.output_buffer_index_write+1);
   openserial_vars.output_buffer_index_write = counter-1;
   ENABLE_INTERRUPTS();
   
   return E_SUCCESS;
}


//=========================== interrupt handlers ==============================

//executed in ISR, called from scheduler.c
void isr_openserial_tx() {
   switch (openserial_vars.mode) {
      case MODE_INPUT:
         openserial_vars.input_command_index++;
         if (openserial_vars.input_command_index<sizeof(openserial_vars.input_command)) {
            uart_writeByte(openserial_vars.input_command[openserial_vars.input_command_index]);
         } else {
            openserial_vars.ready_receive_command = TRUE;
         }
         break;
      case MODE_OUTPUT:
         if (openserial_vars.output_buffer_index_write==openserial_vars.output_buffer_index_read) {
            openserial_vars.somethingInOutputBuffer=FALSE;
            openserial_vars.output_buffer_index_read   = -1;
            openserial_vars.output_buffer_index_write  = -1;
         }
         if (openserial_vars.somethingInOutputBuffer) {
            uart_writeByte(openserial_vars.output_buffer[output_buffer_index_read_increment()]);
         }
         break;
      case MODE_OFF:
      default:
         break;
   }
}

//executed in ISR, called from scheduler.c
//TODO make sure I'm initializing the buffer length
void isr_openserial_rx() {
   uint8_t temporary;
   uint8_t receivedLength;
   if (openserial_vars.mode==MODE_INPUT) {
     temporary=uart_readByte();
     if(openserial_vars.receiving==FALSE){
       if (temporary==HDLC_FLAG){//we just started receiving
         openserial_vars.receiving=TRUE;
         openserial_vars.input_buffer[openserial_vars.input_buffer_fill_level++]=temporary;
       }
     } else {
       if(temporary==HDLC_FLAG){//we're done receiving
         openserial_vars.input_buffer[openserial_vars.input_buffer_fill_level++]=temporary;
         receivedLength = dehdlcify(openserial_vars.input_buffer,openserial_vars.input_buffer_fill_level);
         if(receivedLength<255){//valid packet
           openserial_vars.receiving = FALSE;
           openserial_vars.received_command = openserial_vars.input_buffer[0];
           openserial_vars.input_buffer_fill_level = receivedLength-1;//minus the command byte
           memcpy(openserial_vars.input_buffer,openserial_vars.input_buffer+1,receivedLength-1);//removing the command byte
           openserial_stop();
         } else { //invalid packet
           openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUTBUFFER_BAD_CRC,
                                 (errorparameter_t)0,
                                 (errorparameter_t)0);
           openserial_vars.receiving = FALSE;
           openserial_vars.input_buffer_fill_level=0;
           openserial_vars.received_command = (uint8_t)'X';//invalid command
           openserial_stop();
           
         }
         //call dehdlcify
         //reset the buffer length
         //set receiving to FALSE
         //call stop
       } else { //we're in the middle of a packet
         openserial_vars.input_buffer[openserial_vars.input_buffer_fill_level++]=temporary;
         if (openserial_vars.input_buffer_fill_level+1>SERIAL_INPUT_BUFFER_SIZE){//make sure we're not overflowing
           openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUT_BUFFER_OVERFLOW,
                                 (errorparameter_t)0,
                                 (errorparameter_t)0);
           openserial_vars.receiving = FALSE;
           openserial_vars.input_buffer_fill_level=0;
           openserial_vars.received_command = (uint8_t)'X';//invalid command
           openserial_stop();
         }
       }
     }
   }
}