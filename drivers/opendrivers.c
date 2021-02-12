#include "opentimers.h"
#include "openserial.h"

void opendrivers_init(void){

    //===== drivers
    opentimers_init();
    openserial_init();
}