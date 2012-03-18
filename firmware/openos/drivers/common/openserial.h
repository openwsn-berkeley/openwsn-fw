#ifndef __OPENSERIAL_H
#define __OPENSERIAL_H

/**
\addtogroup cross-layers
\{
\addtogroup OpenSerial
\{
*/

//=========================== define ==========================================

#define SERIAL_OUTPUT_BUFFER_SIZE 300
//not more than 255 (length encoded in 1B)
#define SERIAL_INPUT_BUFFER_SIZE  200

enum {
   MODE_OFF    = 0,
   MODE_INPUT  = 1,
   MODE_OUTPUT = 2
};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void    openserial_init();
error_t openserial_printStatus(uint8_t statusElement, uint8_t* buffer, uint16_t length);
error_t openserial_printError(uint8_t calling_component, uint8_t error_code,
                              errorparameter_t arg1,
                              errorparameter_t arg2);
error_t openserial_printData(uint8_t* buffer, uint8_t length);
uint8_t openserial_getNumDataBytes();
uint8_t openserial_getInputBuffer(uint8_t* bufferToWrite, uint8_t maxNumBytes);
void    openserial_startInput();
void    openserial_startOutput();
void    openserial_stop();
bool    debugPrint_outBufferIndexes();

// interrupt handlers
void    isr_openserial_rx();
void    isr_openserial_tx();

/**
\}
\}
*/

#endif