/**
\brief Python-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "interface.h"
#include "leds.h"

//=========================== defines =========================================


//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_init()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_init], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_init() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_error_on(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_error_on()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_error_on], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_error_on() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_error_off(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_error_off()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_error_off], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_error_off() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_error_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_error_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_error_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_error_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

uint8_t leds_error_isOn(void) {
    PyObject *result;
    uint8_t returnVal;

#ifdef TRACE_ON
    printf("leds_error_isOn()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_error_isOn], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_error_isOn() returned NULL\r\n");
        return 0;
    }
    returnVal = (uint8_t) PyLong_AsLong(result);
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...got %d.\n",returnVal);
#endif

    return returnVal;
}

void leds_error_blink(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_error_blink()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_error_blink], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_error_blink() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_radio_on(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_radio_on()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_radio_on], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_radio_on() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_radio_off(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_radio_off()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_radio_off], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_radio_off() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_radio_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_radio_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_radio_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_radio_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

uint8_t leds_radio_isOn(void) {
    PyObject *result;
    uint8_t returnVal;

#ifdef TRACE_ON
    printf("leds_radio_isOn()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_radio_isOn], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_radio_isOn() returned NULL\r\n");
        return 0;
    }
    returnVal = (uint8_t) PyLong_AsLong(result);
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...got %d.\n",returnVal);
#endif

    return returnVal;
}

// green
void leds_sync_on(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_sync_on()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_sync_on], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_sync_on() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_sync_off(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_sync_off()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_sync_off], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_sync_off() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_sync_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_sync_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_sync_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_sync_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

uint8_t leds_sync_isOn(void) {
    PyObject *result;
    uint8_t returnVal;

#ifdef TRACE_ON
    printf("leds_sync_isOn()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_sync_isOn], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_sync_isOn() returned NULL\r\n");
        return 0;
    }
    returnVal = (uint8_t) PyLong_AsLong(result);
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...got %d.\n",returnVal);
#endif

    return returnVal;
}

// yellow
void leds_debug_on(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_debug_on()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_debug_on], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_debug_on() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_debug_off(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_debug_off()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_debug_off], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_debug_off() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_debug_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_debug_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_debug_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_debug_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

uint8_t leds_debug_isOn(void) {
    PyObject *result;
    uint8_t returnVal;

#ifdef TRACE_ON
    printf("leds_debug_isOn()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_debug_isOn], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_debug_isOn() returned NULL\r\n");
        return 0;
    }
    returnVal = (uint8_t) PyLong_AsLong(result);
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...got %d.\n",returnVal);
#endif

    return returnVal;
}

void leds_all_on(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_all_on()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_all_on], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_all_on() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_all_off(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_all_off()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_all_off], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_all_off() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_all_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_all_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_all_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_all_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_circular_shift(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_circular_shift()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_circular_shift], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_circular_shift() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void leds_increment(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("leds_increment()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_leds_increment], NULL);
    if (result == NULL) {
        printf("[CRITICAL] leds_increment() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== private =========================================