/**
\brief Definition of the serialEcho driver.

\author Xavire Vilajosana  <xvilajosana@eecs.berkeley.edu>, January 2013.
*/



#include "serialecho.h"
#include "openserial.h"

typedef struct{
    uint8_t buffer[SERIAL_INPUT_BUFFER_SIZE];
    uint8_t numDataBytes;
}serialecho_vars_t;

serialecho_vars_t serialecho_vars;

//============= public ==================
void serialecho_init(){
   memset(&serialecho_vars,0,sizeof(serialecho_vars_t));
}

void serialecho_echo(){
   //read input
   serialecho_vars.numDataBytes = openserial_getNumDataBytes();
   openserial_getInputBuffer(&(serialecho_vars.buffer[0]),serialecho_vars.numDataBytes);
   //echo them
   openserial_printData(serialecho_vars.buffer, serialecho_vars.numDataBytes);
}