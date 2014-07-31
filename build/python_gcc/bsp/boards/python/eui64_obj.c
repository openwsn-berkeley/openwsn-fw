/**
\brief Python-specific definition of the "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "eui64_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(OpenMote* self, uint8_t* addressToWrite) {
   PyObject*  result;
   PyObject*  item;
   uint8_t    i;
   
#ifdef TRACE_ON
   printf("C@0x%x: eui64_get()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_eui64_get],NULL);
   if (result == NULL) {
      printf("[CRITICAL] eui64_get() returned NULL\r\n");
      return;
   }
   
   // verify
   if (!PySequence_Check(result)) {
      printf("[CRITICAL] eui64_get() did not return a list\r\n");
      return;
   }
   if (PyList_Size(result)!=8) {
      printf("[CRITICAL] eui64_get() did not return a list of exactly 8 elements\r\n");
      return;
   }

#ifdef TRACE_ON
   printf("C@0x%x: got ",self);
#endif
   // store retrieved information
   for (i=0;i<8;i++) {
      item = PyList_GetItem(result, i);
      addressToWrite[i] = (uint8_t)PyInt_AsLong(item);
#ifdef TRACE_ON
   printf("%02x",addressToWrite[i]);
#endif
   }
#ifdef TRACE_ON
   printf("\n");
#endif
   
   // dispose of returned value
   Py_DECREF(result);
}

//=========================== private =========================================