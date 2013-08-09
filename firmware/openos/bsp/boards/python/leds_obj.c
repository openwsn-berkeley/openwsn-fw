/**
\brief Python-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include <stdio.h>
#include "leds_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_init()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void leds_error_on(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_error_on()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_error_on],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_error_on() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_error_off(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_error_off()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_error_off],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_error_off() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_error_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_error_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_error_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_error_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
uint8_t leds_error_isOn(OpenMote* self) {
   PyObject*  result;
   uint8_t    returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_error_isOn()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_error_isOn],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_error_isOn() returned NULL\r\n");
      return 0;
   }
   returnVal = (uint8_t)PyInt_AsLong(result);
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}
void leds_error_blink(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_error_blink()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_error_blink],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_error_blink() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void leds_radio_on(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_radio_on()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_radio_on],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_radio_on() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_radio_off(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_radio_off()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_radio_off],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_radio_off() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_radio_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_radio_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_radio_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_radio_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
uint8_t leds_radio_isOn(OpenMote* self) {
   PyObject*  result;
   uint8_t    returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_radio_isOn()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_radio_isOn],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_radio_isOn() returned NULL\r\n");
      return 0;
   }
   returnVal = (uint8_t)PyInt_AsLong(result);
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

// green
void leds_sync_on(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_sync_on()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_sync_on],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_sync_on() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_sync_off(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_sync_off()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_sync_off],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_sync_off() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_sync_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_sync_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_sync_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_sync_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
uint8_t leds_sync_isOn(OpenMote* self) {
   PyObject*  result;
   uint8_t    returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_sync_isOn()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_sync_isOn],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_sync_isOn() returned NULL\r\n");
      return 0;
   }
   returnVal = (uint8_t)PyInt_AsLong(result);
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

// yellow
void leds_debug_on(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_debug_on()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_debug_on],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_debug_on() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_debug_off(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_debug_off()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_debug_off],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_debug_off() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_debug_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_debug_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_debug_toggle],NULL);
    if (result == NULL) {
      printf("[CRITICAL] leds_debug_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
uint8_t leds_debug_isOn(OpenMote* self) {
   PyObject*  result;
   uint8_t    returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_debug_isOn()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_debug_isOn],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_debug_isOn() returned NULL\r\n");
      return 0;
   }
   returnVal = (uint8_t)PyInt_AsLong(result);
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

void leds_all_on(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_all_on()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_all_on],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_all_on() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_all_off(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_all_off()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_all_off],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_all_off() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
void leds_all_toggle(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_all_toggle()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_all_toggle],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_all_toggle() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void leds_circular_shift(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_circular_shift()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_circular_shift],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_circular_shift() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void leds_increment(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: leds_increment()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_leds_increment],NULL);
   if (result == NULL) {
      printf("[CRITICAL] leds_increment() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== private =========================================