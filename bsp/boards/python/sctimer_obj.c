/**
\brief Python-specific definition of the "sctimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "sctimer_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== callback ========================================

void sctimer_set_callback(OpenMote* self, sctimer_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_set_callback(cb=0x%x)... \n",self,cb);
#endif
   
   self->sctimer_icb.compare_cb = cb;
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== public ==========================================

//===== admin

void sctimer_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_init()... \n",self,self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_sctimer_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] sctimer_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== direct access

PORT_RADIOTIMER_WIDTH sctimer_readCounter(OpenMote* self) {
   PyObject*  result;
   PORT_RADIOTIMER_WIDTH   returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_readCounter()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_sctimer_readCounter],NULL);
   if (result == NULL) {
      printf("[CRITICAL] sctimer_readCounter() returned NULL\r\n");
      return 0;
   }
   returnVal  = (PORT_TIMER_WIDTH)PyInt_AsLong(result);
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("returnVal=%d.\n",returnVal);
#endif
   
   return returnVal;
}

//===== compare

void sctimer_setCompare(OpenMote* self, PORT_RADIOTIMER_WIDTH value) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_setCompare(value=%d)... \n",self,value);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",value);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_sctimer_setCompare],arglist);
   if (result == NULL) {
      printf("[CRITICAL] sctimer_setCompare() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void sctimer_enable(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_enable()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_sctimer_enable],NULL);
   if (result == NULL) {
      printf("[CRITICAL] sctimer_enable() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void sctimer_disable(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_disable()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_sctimer_disable],NULL);
   if (result == NULL) {
      printf("[CRITICAL] sctimer_disable() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== interrupt handlers ==============================

void sctimer_intr_compare(OpenMote* self) {
   
#ifdef TRACE_ON
   printf("C@0x%x: sctimer_intr_compare(), calling 0x%x... \n",self,self->sctimer_icb.compare_cb);
#endif
   
   self->sctimer_icb.compare_cb(self);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== private =========================================