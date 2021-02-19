/**
\brief Python-specific definition of the "sctimer" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "interface.h"
#include "sctimer.h"

//=========================== typedefs =======================================

typedef struct {
    sctimer_cbt compare_cb;
} sctimer_icb_t;

//=========================== variables =======================================

sctimer_icb_t sctimer_icb;

//=========================== prototypes ======================================

//=========================== callback ========================================

void sctimer_set_callback(sctimer_cbt cb) {

#ifdef TRACE_ON
    printf("sctimer_set_callback (cb = %p) ... \n", cb);
#endif

    sctimer_icb.compare_cb = cb;

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== public ==========================================

//===== admin

void sctimer_init(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("sctimer_init()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_sctimer_init], NULL);
    if (result == NULL) {
        printf("[CRITICAL] sctimer_init() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//===== direct access

PORT_RADIOTIMER_WIDTH sctimer_readCounter(void) {
    PyObject *result;
    PORT_RADIOTIMER_WIDTH returnVal;

#ifdef TRACE_ON
    printf("sctimer_readCounter()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_sctimer_readCounter], NULL);
    if (result == NULL) {
        printf("[CRITICAL] sctimer_readCounter() returned NULL\r\n");
        return 0;
    }
    returnVal = (PORT_TIMER_WIDTH) PyLong_AsLong(result);
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("returnVal=%d.\n", returnVal);
#endif

    return returnVal;
}

//===== compare

void sctimer_setCompare(PORT_RADIOTIMER_WIDTH value) {
    PyObject *result;
    PyObject *arglist;

#ifdef TRACE_ON
    printf("sctimer_setCompare(value=%d)... \n", value);
#endif

    // forward to Python
    arglist = Py_BuildValue("(i)", value);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_sctimer_setCompare], arglist);
    if (result == NULL) {
        printf("[CRITICAL] sctimer_setCompare() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void sctimer_enable(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("sctimer_enable()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_sctimer_enable], NULL);
    if (result == NULL) {
        printf("[CRITICAL] sctimer_enable() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void sctimer_disable(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("sctimer_disable()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_sctimer_disable], NULL);
    if (result == NULL) {
        printf("[CRITICAL] sctimer_disable() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== interrupt handlers ==============================

void sctimer_intr_compare(void) {

#ifdef TRACE_ON
    printf("sctimer_intr_compare(), calling %p... \n", sctimer_icb.compare_cb);
#endif

    sctimer_icb.compare_cb();

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== private =========================================