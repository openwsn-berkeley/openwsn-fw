/**
\brief Python-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "radiotimer_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== callback ========================================

void radiotimer_setOverflowCb(OpenMote* self, radiotimer_compare_cbt cb) {
   self->radiotimer_icb.overflow_cb = cb;
}

void radiotimer_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb) {
   self->radiotimer_icb.compare_cb = cb;
}

//=========================== public ==========================================

//===== admin

void radiotimer_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radiotimer_init()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_init() returned NULL\r\n");
   }
   Py_DECREF(result);
}

void radiotimer_start(OpenMote* self, uint16_t period) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radiotimer_start()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_start],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_start() returned NULL\r\n");
   }
   Py_DECREF(result);
}

//===== direct access

uint16_t radiotimer_getValue(OpenMote* self) {
   PyObject*  result;
   uint16_t   returnVal;
   
#ifdef TRACE_ON
   printf("C: radiotimer_getValue()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_getValue],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_getValue() returned NULL\r\n");
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radiotimer_getValue() returned NULL\r\n");
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
   return returnVal;
}

void radiotimer_setPeriod(OpenMote* self, uint16_t period) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C: radiotimer_setPeriod()\n");
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",period);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_setPeriod],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_setPeriod() returned NULL\r\n");
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
}

uint16_t radiotimer_getPeriod(OpenMote* self) {
   PyObject*  result;
   uint16_t   returnVal;
   
#ifdef TRACE_ON
   printf("C: radiotimer_getPeriod()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_getPeriod],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_getPeriod() returned NULL\r\n");
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radiotimer_getPeriod() returned NULL\r\n");
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
   return returnVal;
}

//===== compare

void radiotimer_schedule(OpenMote* self, uint16_t offset) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C: radiotimer_schedule()\n");
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",offset);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_schedule],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_schedule() returned NULL\r\n");
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
}

void radiotimer_cancel(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radiotimer_cancel()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_cancel],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_cancel() returned NULL\r\n");
   }
   Py_DECREF(result);
}

//===== capture

uint16_t radiotimer_getCapturedTime(OpenMote* self) {
   PyObject*  result;
   uint16_t   returnVal;
   
#ifdef TRACE_ON
   printf("C: radiotimer_getCapturedTime()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_getCapturedTime],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_getCapturedTime() returned NULL\r\n");
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radiotimer_getCapturedTime() returned NULL\r\n");
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
   return returnVal;
}

//=========================== interrupt handlers ==============================

void radiotimer_intr_compare(OpenMote* self) {
   self->radiotimer_icb.compare_cb(self);
}

void radiotimer_intr_overflow(OpenMote* self) {
   self->radiotimer_icb.overflow_cb(self);
}

//=========================== private =========================================