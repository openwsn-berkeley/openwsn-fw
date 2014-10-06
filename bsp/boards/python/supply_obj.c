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
   printf("C@0x%x: supply_init()... \n",self);
#endif
   
   // Nothing to do
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void supply_on(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C@0x%x: supply_on()... \n",self);
#endif
   
   // start the mote's execution
   mote_main(self);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void supply_off(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C@0x%x: supply_off()... \n",self);
#endif
   
   // TODO
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== interrupt handlers ==============================