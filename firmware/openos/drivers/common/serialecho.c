/**
\brief Definition of the "serialecho" driver.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, January 2013.
*/

#include "serialecho.h"
#include "openserial.h"

//=========================== variables =======================================

typedef struct{
   uint8_t bufLen;
   uint8_t buf[SERIAL_INPUT_BUFFER_SIZE];
} serialecho_vars_t;

serialecho_vars_t serialecho_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void serialecho_init(){
   memset(&serialecho_vars,0,sizeof(serialecho_vars_t));
}

void serialecho_echo(uint8_t* buf, uint8_t bufLen){
   
   // read input
   serialecho_vars.bufLen = bufLen;
   memcpy(serialecho_vars.buf,buf,serialecho_vars.bufLen);
   
   // echo them
   openserial_printData(
      serialecho_vars.buf,
      serialecho_vars.bufLen
   );
}

//=========================== private =========================================