/**
\brief Python-based emulation of the mote's power supply.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include <stdio.h>
#include "supply_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void supply_init(OpenMote* self) {
   // Nothing to do
}

void supply_on(OpenMote* self) {
   mote_main(self);
}

void supply_off(OpenMote* self) {
   // TODO
}

//=========================== interrupt handlers ==============================