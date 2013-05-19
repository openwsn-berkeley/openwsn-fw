/**
\brief Python-specific definition of the "bsp_timer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include <stdio.h>
#include "bsp_timer_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void bsp_timer_set_callback(OpenMote* self, bsp_timer_cbt cb) {
   self->bsp_timer_icb.cb   = cb;
}

//=========================== public ==========================================

void bsp_timer_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: bsp_timer_init()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_bsp_timer_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] bsp_timer_init() returned NULL\r\n");
   }
   Py_DECREF(result);
}

void bsp_timer_reset(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: bsp_timer_reset()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_bsp_timer_reset],NULL);
   if (result == NULL) {
      printf("[CRITICAL] bsp_timer_reset() returned NULL\r\n");
   }
   Py_DECREF(result);
}

void bsp_timer_scheduleIn(OpenMote* self, PORT_TIMER_WIDTH delayTicks) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C: bsp_timer_scheduleIn()\n");
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",delayTicks);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_bsp_timer_scheduleIn],arglist);
   if (result == NULL) {
      printf("[CRITICAL] bsp_timer_scheduleIn() returned NULL\r\n");
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
}

void bsp_timer_cancel_schedule(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: bsp_timer_cancel_schedule()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_bsp_timer_cancel_schedule],NULL);
   if (result == NULL) {
      printf("[CRITICAL] bsp_timer_cancel_schedule() returned NULL\r\n");
   }
   Py_DECREF(result);
}

PORT_TIMER_WIDTH bsp_timer_get_currentValue(OpenMote* self) {
   PyObject*            result;
   PORT_TIMER_WIDTH     returnVal;
   
#ifdef TRACE_ON
   printf("C: bsp_timer_get_currentValue()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_bsp_timer_get_currentValue],NULL);
   if (result == NULL) {
      printf("[CRITICAL] bsp_timer_get_currentValue() returned NULL\r\n");
   }
   returnVal  = (PORT_TIMER_WIDTH)PyInt_AsLong(result);
   Py_DECREF(result);
   
   return returnVal;
}
//=========================== private =========================================

//=========================== interrupt handlers ==============================

kick_scheduler_t bsp_timer_isr(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C: bsp_timer_isr()\n");
#endif
   
   self->bsp_timer_icb.cb(self);
   return 0;//poipoi
}