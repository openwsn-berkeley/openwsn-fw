/**
\brief PC-based emulation of the mote's power supply.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <stdio.h>
#include "supply.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void supply_init() {
   // Nothing to do
}

void supply_on() {
   mote_main();
}

void supply_off() {
   // TODO
}

//=========================== interrupt handlers ==============================