/**
\brief Definition of the "serialecho" driver.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, January 2013.
*/

#include "serialecho.h"
#include "openserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void serialecho_init(){
}

void serialecho_echo(uint8_t* buf, uint8_t bufLen){
   // echo back what you received
   openserial_printData(
      buf,
      bufLen
   );
}

//=========================== private =========================================