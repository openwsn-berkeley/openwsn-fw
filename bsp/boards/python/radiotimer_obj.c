/**
\brief Python-specific definition of the "radiotimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "radiotimer_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== callback ========================================

void radiotimer_setOverflowCb(OpenMote* self, radiotimer_compare_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_setOverflowCb(cb=0x%x)... \n",self,cb);
#endif
   
   self->radiotimer_icb.overflow_cb = cb;
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radiotimer_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_setCompareCb(cb=0x%x)... \n",self,cb);
#endif
   
   self->radiotimer_icb.compare_cb = cb;
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== public ==========================================

//===== admin

void radiotimer_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_init()... \n",self,self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radiotimer_start(OpenMote* self, PORT_RADIOTIMER_WIDTH period) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_start(period=%d)... \n",self,period);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_start],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_start() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== direct access

PORT_RADIOTIMER_WIDTH radiotimer_getValue(OpenMote* self) {
   PyObject*  result;
   PORT_RADIOTIMER_WIDTH   returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_getValue()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_getValue],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_getValue() returned NULL\r\n");
      return 0;
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radiotimer_getValue() returned NULL\r\n");
      return 0;
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

void radiotimer_setPeriod(OpenMote* self, PORT_RADIOTIMER_WIDTH period) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_setPeriod(period=%d)... \n",self,period);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",period);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_setPeriod],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_setPeriod() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

PORT_RADIOTIMER_WIDTH radiotimer_getPeriod(OpenMote* self) {
   PyObject*  result;
   PORT_RADIOTIMER_WIDTH   returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_getPeriod()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_getPeriod],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_getPeriod() returned NULL\r\n");
      return 0;
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radiotimer_getPeriod() returned NULL\r\n");
      return 0;
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

//===== compare

void radiotimer_schedule(OpenMote* self, PORT_RADIOTIMER_WIDTH offset) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_schedule(offset=%d)... \n",self,offset);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",offset);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_schedule],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_schedule() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radiotimer_cancel(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_cancel()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_cancel],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_cancel() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== capture

PORT_RADIOTIMER_WIDTH radiotimer_getCapturedTime(OpenMote* self) {
   PyObject*  result;
   PORT_RADIOTIMER_WIDTH   returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_getCapturedTime()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radiotimer_getCapturedTime],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radiotimer_getCapturedTime() returned NULL\r\n");
      return 0;
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radiotimer_getCapturedTime() returned NULL\r\n");
      return 0;
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

//=========================== interrupt handlers ==============================

void radiotimer_intr_compare(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_intr_compare(), calling 0x%x... \n",self,self->radiotimer_icb.compare_cb);
#endif
   
   self->radiotimer_icb.compare_cb(self);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radiotimer_intr_overflow(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radiotimer_intr_overflow(), calling 0x%x... \n",self,self->radiotimer_icb.overflow_cb);
#endif
   
   self->radiotimer_icb.overflow_cb(self);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== private =========================================