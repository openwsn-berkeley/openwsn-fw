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
   
#ifdef TRACE_ON
   printf("C: supply_init()... \n");
#endif
   
   // Nothing to do
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void supply_on(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C: supply_on()... \n");
#endif
   
   // start the mote's execution
   mote_main(self);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void supply_off(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C: supply_off()... \n");
#endif
   
   // TODO
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

//=========================== interrupt handlers ==============================