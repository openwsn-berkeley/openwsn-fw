/**
\brief Python-specific definition of the "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "openwsnmodule.h"
#include "eui64.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t *addressToWrite) {
    PyObject *result;
    PyObject *item;
    uint8_t i;

#ifdef TRACE_ON
    printf("eui64_get()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_eui64_get], NULL);
    if (result == NULL) {
        printf("[CRITICAL] eui64_get() returned NULL\r\n");
        return;
    }

    // verify
    if (!PySequence_Check(result)) {
        printf("[CRITICAL] eui64_get() did not return a list\r\n");
        return;
    }
    if (PyList_Size(result) != 8) {
        printf("[CRITICAL] eui64_get() did not return a list of exactly 8 elements\r\n");
        return;
    }

#ifdef TRACE_ON
    printf("got ");
#endif
    // store retrieved information
    for (i = 0; i < 8; i++) {
        item = PyList_GetItem(result, i);
        addressToWrite[i] = (uint8_t) PyLong_AsLong(item);
#ifdef TRACE_ON
        printf("%02x", addressToWrite[i]);
#endif
    }
#ifdef TRACE_ON
    printf("\n");
#endif

    // dispose of returned value
    Py_DECREF(result);
}

//=========================== private =========================================