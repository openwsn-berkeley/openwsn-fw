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
   printf("C@0x%x: debugpins_init()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_frame_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_frame_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_frame_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_frame_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_frame_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_frame_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_frame_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_frame_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_frame_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_frame_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_frame_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_frame_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_slot_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_slot_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_slot_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_slot_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_slot_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_slot_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_slot_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_slot_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_slot_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_slot_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_slot_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_slot_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_fsm_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_fsm_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_fsm_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_fsm_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_fsm_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_fsm_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_fsm_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_fsm_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_fsm_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_fsm_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_fsm_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_fsm_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_task_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_task_toggle(... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_task_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_task_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_task_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_task_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_task_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_task_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_task_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_task_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_task_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_task_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_isr_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_isr_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_isr_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_isr_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_isr_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_isr_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_isr_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_isr_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_isr_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_isr_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_isr_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_isr_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_radio_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_radio_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_radio_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_radio_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_radio_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_radio_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_radio_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_radio_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_radio_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_radio_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_radio_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_radio_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_ka_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_ka_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_ka_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_ka_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_ka_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_ka_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_ka_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_ka_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_syncPacket_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_syncPacket_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_syncPacket_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_syncPacket_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_syncPacket_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_syncPacket_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_syncPacket_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_syncPacket_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_syncAck_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_syncAck_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_syncAck_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_syncAck_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_syncAck_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_syncAck_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_syncAck_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_syncAck_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void debugpins_debug_clr(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_debug_clr()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_debug_clr],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_debug_clr() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void debugpins_debug_set(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: debugpins_debug_set()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_debugpins_debug_set],NULL);
   if (result == NULL) {
      printf("[CRITICAL] debugpins_debug_set() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}