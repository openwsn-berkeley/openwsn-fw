/**
\brief PC-specific definition of the "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "eui64_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(OpenMote* self, uint8_t* addressToWrite) {
   printf("eui64_get\r\n");
   PyObject*  result;
   PyObject*  euiList;
   PyObject*  item;
   uint8_t    i;
   
#ifdef TRACE_ON
   printf("C: eui64_get()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_eui64_get],NULL);
   if (result == NULL) {
      printf("[CRITICAL] eui64_get() returned NULL\r\n");
   }
   
   // verify 
   PyArg_ParseTuple(result, "O", &euiList);
   if (!PySequence_Check(euiList)) {
      printf("[CRITICAL] eui64_get() did not return a list\r\n");
   }
   if (PyList_Size(euiList)!=8) {
      printf("[CRITICAL] eui64_get() did not return a list of exactly 8 elements\r\n");
   }
   
   for (i=0;i<8;i++) {
      item = PyList_GetItem(euiList, i);
      addressToWrite[i] = (uint8_t)PyInt_AsLong(item);
   }
   Py_DECREF(result);
   Py_DECREF(euiList);
}

//=========================== private =========================================