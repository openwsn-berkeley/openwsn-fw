/**
\brief Definition of the "openserial" driver.

\author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
*/

#include "board.h"
#include "openwsn.h"
#include "IEEE802154E.h"
#include "openserial.h"
#include "neighbors.h"
#include "res.h"
#include "iphc.h"
#include "forwarding.h"
#include "icmpv6.h"
#include "icmpv6echo.h"
#include "icmpv6router.h"
#include "icmpv6rpl.h"
#include "idmanager.h"
#include "openqueue.h"
#include "tcpinject.h"
#include "udpinject.h"
#include "openbridge.h"
#include "leds.h"
#include "schedule.h"
#include "uart.h"

//=========================== variables =======================================

typedef struct {
   uint8_t    output_buffer[SERIAL_OUTPUT_BUFFER_SIZE];
   uint16_t   output_buffer_index_write;
   uint16_t   output_buffer_index_read;
   bool       somethingInOutputBuffer;

   uint8_t    input_buffer[SERIAL_INPUT_BUFFER_SIZE];
   uint16_t   input_buffer_fill_level;
   uint8_t    input_buffer_bytes_still_to_come;
   uint8_t    received_command;

   bool       ready_receive_command;
   bool       ready_receive_length;
   uint8_t    input_command[8];
   uint8_t    input_command_index;

   uint8_t    mode;
   uint8_t    debugPrintCounter;
} openserial_vars_t;

openserial_vars_t openserial_vars;

//=========================== prototypes ======================================

uint16_t output_buffer_index_write_increment();
uint16_t output_buffer_index_read_increment();

//=========================== public ==========================================

void openserial_init() {
   //initialize variables
   openserial_vars.input_command[0] = (uint8_t)'^';
   openserial_vars.input_command[1] = (uint8_t)'^';
   openserial_vars.input_command[2] = (uint8_t)'^';
   openserial_vars.input_command[3] = (uint8_t)'R';
   openserial_vars.input_command[4] = 0;//to be filled out later
   openserial_vars.input_command[5] = (uint8_t)'$';
   openserial_vars.input_command[6] = (uint8_t)'$';
   openserial_vars.input_command[7] = (uint8_t)'$';
   openserial_vars.output_buffer_index_read  = 0;
   openserial_vars.output_buffer_index_write = 0;
   openserial_vars.somethingInOutputBuffer   = FALSE;
   openserial_vars.mode = MODE_OFF;
   
   // set callbacks
   uart_setCallbacks(isr_openserial_tx,
                     isr_openserial_rx);
}

error_t openserial_printStatus(uint8_t statusElement,uint8_t* buffer, uint16_t length) {
   uint8_t counter;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   openserial_vars.somethingInOutputBuffer=TRUE;
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';                  //preamble
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'S';                  //this is an status update
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[1]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[0]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)statusElement;        //type of element
   for (counter=0;counter<length;counter++){
      openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)buffer[counter];
   }
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';                  //postamble
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';
   ENABLE_INTERRUPTS();

   return E_SUCCESS;
}

error_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   leds_error_toggle();
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   openserial_vars.somethingInOutputBuffer=TRUE;
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';                  //preamble
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'E';                  //this is an error
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[1]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[0]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)calling_component;    //component generating error
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)error_code;           //error_code
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((arg1 & 0xff00)>>8); //arg1
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t) (arg1 & 0x00ff);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((arg2 & 0xff00)>>8); //arg2
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t) (arg2 & 0x00ff);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';                  //postamble
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';
   ENABLE_INTERRUPTS();

   return E_SUCCESS;
}

error_t openserial_printData(uint8_t* buffer, uint8_t length) {
   uint8_t counter;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   openserial_vars.somethingInOutputBuffer=TRUE;
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';                  //preamble
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'^';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'D';                  //this is data
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[1]);
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)((idmanager_getMyID(ADDR_16B))->addr_16b[0]);
   for (counter=0;counter<length;counter++){
      openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)buffer[counter];
   }
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';                  //postamble
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';
   openserial_vars.output_buffer[output_buffer_index_write_increment()]=(uint8_t)'$';
   ENABLE_INTERRUPTS();

   return E_SUCCESS;
}

uint8_t openserial_getNumDataBytes() {
   uint16_t temp_openserial_input_buffer_fill_level;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   temp_openserial_input_buffer_fill_level = openserial_vars.input_buffer_fill_level;
   ENABLE_INTERRUPTS();

   return temp_openserial_input_buffer_fill_level;
}

uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes) {
   uint8_t numBytesWritten;
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
   if (openserial_vars.input_buffer_fill_level>0) {
      openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)openserial_vars.input_buffer_fill_level,
                            (errorparameter_t)0);
      openserial_vars.input_buffer_fill_level = 0;
   }
   openserial_vars.input_command[4] = SERIAL_INPUT_BUFFER_SIZE;
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();          // clear possible pending interrupts
   uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   openserial_vars.mode                  = MODE_INPUT;
   openserial_vars.input_command_index   = 0;
   openserial_vars.ready_receive_command = FALSE;
   openserial_vars.ready_receive_length  = FALSE;
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
         if(debugPrint_outBufferIndexes()==TRUE) {
            break;
         }
      case STATUS_ASN:
         if(debugPrint_asn()==TRUE) {
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
      case STATUS_QUEUE:
         if(debugPrint_queue()==TRUE) {
            break;
         }
      case STATUS_NEIGHBORS:
         if(debugPrint_neighbors()==TRUE) {
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
         case 'O': //Trigger ICMPv6Router
            icmpv6router_trigger();
            break;
         case 'P': //Trigger ICMPv6RPL
            icmpv6rpl_trigger();
            break;
         case 'D': //Trigger OpenBridge (called only by moteProbe)
            openbridge_trigger();
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
   uint16_t temp_openserial_output_buffer_index_read;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   openserial_vars.output_buffer_index_read=(openserial_vars.output_buffer_index_read+1)%SERIAL_OUTPUT_BUFFER_SIZE;
   temp_openserial_output_buffer_index_read = openserial_vars.output_buffer_index_read;
   ENABLE_INTERRUPTS();
   return temp_openserial_output_buffer_index_read;
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
void isr_openserial_rx() {
   if (openserial_vars.mode==MODE_INPUT) {
      if (openserial_vars.ready_receive_command==TRUE) {
         openserial_vars.ready_receive_command=FALSE;
         openserial_vars.received_command=uart_readByte();
         openserial_vars.ready_receive_length=TRUE;
      } else if (openserial_vars.ready_receive_length==TRUE) {
         openserial_vars.ready_receive_length=FALSE;
         openserial_vars.input_buffer_bytes_still_to_come=uart_readByte();
      } else {
         openserial_vars.input_buffer[openserial_vars.input_buffer_fill_level++]=uart_readByte();
         if (openserial_vars.input_buffer_fill_level+1>SERIAL_INPUT_BUFFER_SIZE){
            openserial_printError(COMPONENT_OPENSERIAL,ERR_INPUT_BUFFER_OVERFLOW,
                                  (errorparameter_t)0,
                                  (errorparameter_t)0);
            openserial_vars.input_buffer_fill_level=0;
            openserial_stop();
         }
         openserial_vars.input_buffer_bytes_still_to_come--;
         if (openserial_vars.input_buffer_bytes_still_to_come==0) {
            openserial_stop();
         }
      }
   }
}
