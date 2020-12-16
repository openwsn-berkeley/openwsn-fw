/**
\brief Python-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "openwsnmodule.h"
#include "debugpins.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_init()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_init], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_init() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_frame_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_frame_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_frame_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_frame_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_frame_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_frame_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_frame_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_frame_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_frame_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_frame_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_frame_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_frame_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_slot_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_slot_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_slot_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_slot_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_slot_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_slot_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_slot_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_slot_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_slot_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_slot_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_slot_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_slot_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_fsm_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_fsm_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_fsm_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_fsm_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_fsm_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_fsm_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_fsm_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_fsm_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_fsm_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_fsm_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_fsm_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_fsm_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_task_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_task_toggle(... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_task_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_task_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_task_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_task_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_task_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_task_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_task_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_task_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_task_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_task_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_isr_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_isr_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_isr_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_isr_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_isr_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_isr_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_isr_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_isr_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_isr_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_isr_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_isr_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_isr_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_radio_toggle(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_radio_toggle()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_radio_toggle], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_radio_toggle() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_radio_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_radio_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_radio_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_radio_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_radio_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_radio_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_radio_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_radio_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_ka_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_ka_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_ka_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_ka_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_ka_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_ka_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_ka_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_ka_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_syncPacket_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_syncPacket_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_syncPacket_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_syncPacket_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_syncPacket_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_syncPacket_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_syncPacket_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_syncPacket_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_syncAck_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_syncAck_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_syncAck_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_syncAck_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_syncAck_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_syncAck_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_syncAck_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_syncAck_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_debug_clr(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_debug_clr()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_debug_clr], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_debug_clr() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void debugpins_debug_set(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("debugpins_debug_set()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_debugpins_debug_set], NULL);
    if (result == NULL) {
        printf("[CRITICAL] debugpins_debug_set() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}