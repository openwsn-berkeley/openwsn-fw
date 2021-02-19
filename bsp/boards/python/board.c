/**
\brief Python-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include <Python.h>
#include "interface.h"

// bsp modules
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "uart.h"
#include "radio.h"
#include "sctimer.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("board_init() ...\n");
#endif

    // initialize bsp modules
    debugpins_init();
    leds_init();
    sctimer_init();
    uart_init();
    radio_init();

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_board_init], NULL);
    if (result == NULL) {
        printf("[CRITICAL] board_init() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("... done.\n");
#endif
}

void board_sleep(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("board_sleep()...\n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_board_sleep], NULL);
    if (result == NULL) {
        printf("[CRITICAL] board_sleep() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf(" ...done.\n");
#endif
}

void board_reset(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("board_reset()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_board_reset], NULL);
    if (result == NULL) {
        printf("[CRITICAL] board_reset() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void board_barrier_slot_sync(void) {

    PyObject *result;

#ifdef TRACE_ON
    printf("board_barrier_slot_sync()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_board_slot_sync], NULL);
    if (result == NULL) {
        printf("[CRITICAL] board_barrier_slot_sync() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}


void board_barrier_msg_sync(void) {

    PyObject *result;

#ifdef TRACE_ON
    printf("board_barrier_msg_sync()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_board_msg_sync], NULL);
    if (result == NULL) {
        printf("[CRITICAL] board_barrier_msg_sync() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void board_barrier_ack_sync(void) {

    PyObject *result;

#ifdef TRACE_ON
    printf("board_barrier_ack_sync()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_board_ack_sync], NULL);
    if (result == NULL) {
        printf("[CRITICAL] board_barrier_ack_sync() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== private =========================================
