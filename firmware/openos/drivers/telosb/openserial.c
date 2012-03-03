#include "openwsn.h"
#include "openserial.h"
#include "IEEE802154E.h"
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

//=========================== public ==========================================

void openserial_init() {
   //poipoi
}
error_t openserial_printStatus(uint8_t statusElement,uint8_t* buffer, uint16_t length) {
   //poipoi
   return 0;
}
error_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2) {
   //poipoi
   return E_SUCCESS;
}
error_t openserial_printData(uint8_t* buffer, uint8_t length) {
   //poipoi
   return E_SUCCESS;
}
uint8_t openserial_getNumDataBytes() {
   return 0;
   //poipoi
}
uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes) {
   // poipoi
   return 0;
}

void openserial_startInput() {
   // poipoi
}

void openserial_startOutput() {
   // poipoi
}

void openserial_stop() {
   // poipoi
}

bool debugPrint_outBufferIndexes() {
   // poipoi
   return TRUE;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void isr_openserial_tx() {
}

void isr_openserial_rx() {
}