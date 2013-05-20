/**
\brief Python-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "debugpins_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_init()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_init() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void debugpins_frame_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_frame_toggle()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_frame_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_frame_toggle() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_frame_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_frame_clr()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_frame_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_frame_clr() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_frame_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_frame_set()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_frame_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_frame_set() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void debugpins_slot_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_slot_toggle()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_slot_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_slot_toggle() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_slot_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_slot_clr()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_slot_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_slot_clr() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_slot_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_slot_set()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_slot_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_slot_set() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void debugpins_fsm_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_fsm_toggle()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_fsm_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_fsm_toggle() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_fsm_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_fsm_clr()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_fsm_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_fsm_clr() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_fsm_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_fsm_set()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_fsm_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_fsm_set() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void debugpins_task_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_task_toggle(... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_task_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_task_toggle() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_task_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_task_clr()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_task_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_task_clr() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_task_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_task_set()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_task_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_task_set() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void debugpins_isr_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_isr_toggle()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_isr_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_isr_toggle() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_isr_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_isr_clr()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_isr_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_isr_clr() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_isr_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_isr_set()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_isr_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_isr_set() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void debugpins_radio_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_radio_toggle()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_radio_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_radio_toggle() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_radio_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_radio_clr()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_radio_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_radio_clr() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}
void debugpins_radio_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: debugpins_radio_set()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_radio_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_radio_set() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}