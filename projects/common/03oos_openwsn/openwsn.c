/**
   \brief This project runs the full OpenWSN stack.

   \author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
 */

#include "config.h"

#if PYTHON_BOARD

#include "Python.h"

#include <signal.h>
#include "interface.h"

#endif

#include "openserial.h"
#include "board.h"
#include "scheduler.h"
#include "opendrivers.h"
#include "openstack.h"
#include "openweb.h"
#include "openapps.h"


// ========================= variables =========================

#if PYTHON_BOARD
#define PY_SSIZE_T_CLEAN
#endif

#if PYTHON_BOARD

void INThandler(int sig) {
    signal(sig, SIG_IGN);
    scheduler_stop();
}

#endif


int mote_main(void) {

    // initialize
    board_init();
    scheduler_init();

    opendrivers_init();

    openstack_init();
    openweb_init();
    openapps_init();

    LOG_SUCCESS(COMPONENT_OPENWSN, ERR_BOOTED, (errorparameter_t) 0, (errorparameter_t) 0);

    // start
    scheduler_start();

    return 0; // this line should never be reached
}

#if PYTHON_BOARD

static PyObject *set_callback(PyObject *self, PyObject *args) {
    (void) self;

    int cmdId;
    PyObject *tempCallback;

    // parse arguments
    if (!PyArg_ParseTuple(args, "iO:set_callback", &cmdId, &tempCallback)) {
        return NULL;
    }

    // make sure cmdId is plausible
    if (cmdId < 0 || cmdId > MOTE_NOTIF_LAST) {
        PyErr_SetString(PyExc_TypeError, "wrong cmdId");
        return NULL;
    }

    // make sure tempCallback is callable
    if (!PyCallable_Check(tempCallback)) {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }


    // record the callback
    Py_XINCREF(tempCallback);                       // add a reference to new callback
    Py_XDECREF(callbacks[cmdId]);              // dispose of previous callback
    callbacks[cmdId] = tempCallback;                // remember new callback

    // printf("callbacks: %d - %p \n", cmdId, tempCallback);

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *radio_isr_startFrame(PyObject *self, PyObject *args) {
    (void) self;
    long capturedTime;

    // parse the arguments
    if (!PyArg_ParseTuple(args, "l", &capturedTime)) {
        return NULL;
    }
    if (capturedTime > 0xffffffff) {
        printf("[radio_isr_startFrame] FATAL: capturedTime larger than 0xffffffff\n");
        // TODO raise exception
        return NULL;
    }

    // call the callback
    radio_intr_startOfFrame((uint32_t) capturedTime);

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *radio_isr_endFrame(PyObject *self, PyObject *args) {
    (void) self;
    long capturedTime;

    // parse the arguments
    if (!PyArg_ParseTuple(args, "l", &capturedTime)) {
        return NULL;
    }
    if (capturedTime > 0xffffffff) {
        fprintf(stderr, "[radio_isr_endFrame] FATAL: capturedTime larger than 0xffffffff\n");
        // TODO raise exception
        return NULL;
    }

    // call the callback
    radio_intr_endOfFrame((uint32_t) capturedTime);

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *sctimer_isr(PyObject *self) {
    (void) self;

    // call the callback
    sctimer_intr_compare();

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *uart_isr_tx(PyObject *self) {
    (void) self;

    // call the callback
    uart_intr_tx();

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *uart_isr_rx(PyObject *self) {
    (void) self;

    // call the callback
    uart_intr_rx();

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *supply_on(PyObject *self) {
    (void) self;

    // start the mote's execution

    signal(SIGINT, INThandler);
    mote_main();

    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *supply_init(PyObject *self) {
    (void) self;

    // TODO
    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *supply_off(PyObject *self) {
    (void) self;

    // TODO
    // return successfully
    Py_INCREF(Py_None);
    return Py_None;
}

//===== admin

/*
\brief List of methods of the openwsn module class.
*/
static PyMethodDef OpenWSN_Methods[] = {
        {"set_callback",         (PyCFunction) set_callback,         METH_VARARGS, ""},
        {"radio_isr_startFrame", (PyCFunction) radio_isr_startFrame, METH_VARARGS, ""},
        {"radio_isr_endFrame",   (PyCFunction) radio_isr_endFrame,   METH_VARARGS, ""},
        {"sctimer_isr",          (PyCFunction) sctimer_isr,          METH_NOARGS,  ""},
        {"uart_isr_tx",          (PyCFunction) uart_isr_tx,          METH_NOARGS,  ""},
        {"uart_isr_rx",          (PyCFunction) uart_isr_rx,          METH_NOARGS,  ""},
        {"supply_init",          (PyCFunction) supply_init,          METH_NOARGS,  ""},
        {"supply_on",            (PyCFunction) supply_on,            METH_NOARGS,  ""},
        {"supply_off",           (PyCFunction) supply_off,           METH_NOARGS,  ""},
        {NULL, NULL,                                                 METH_NOARGS,  ""}
};
//=========================== openwsn module ==================================

//===== members

//===== methods

//===== admin

static PyModuleDef openmotemodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "openwsn",
        .m_doc = "Python/C extension for the OpenWSN-firmware",
        .m_size = -1,
        .m_methods = OpenWSN_Methods
};

PyMODINIT_FUNC PyInit_openmote(void) {
    PyObject *m;

    m = PyModule_Create(&openmotemodule);
    if (m == NULL)
        return NULL;

    PyModule_AddIntMacro(m, MOTE_NOTIF_board_init);
    PyModule_AddIntMacro(m, MOTE_NOTIF_board_sleep);
    PyModule_AddIntMacro(m, MOTE_NOTIF_board_reset);
    PyModule_AddIntMacro(m, MOTE_NOTIF_board_slot_sync);
    PyModule_AddIntMacro(m, MOTE_NOTIF_board_msg_sync);
    PyModule_AddIntMacro(m, MOTE_NOTIF_board_ack_sync);
    PyModule_AddIntMacro(m, MOTE_NOTIF_sctimer_init);
    PyModule_AddIntMacro(m, MOTE_NOTIF_sctimer_setCompare);
    PyModule_AddIntMacro(m, MOTE_NOTIF_sctimer_set_callback);
    PyModule_AddIntMacro(m, MOTE_NOTIF_sctimer_readCounter);
    PyModule_AddIntMacro(m, MOTE_NOTIF_sctimer_enable);
    PyModule_AddIntMacro(m, MOTE_NOTIF_sctimer_disable);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_init);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_frame_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_frame_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_frame_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_slot_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_slot_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_slot_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_fsm_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_fsm_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_fsm_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_task_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_task_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_task_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_isr_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_isr_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_isr_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_radio_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_radio_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_radio_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_ka_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_ka_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_syncPacket_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_syncPacket_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_syncAck_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_syncAck_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_debug_clr);
    PyModule_AddIntMacro(m, MOTE_NOTIF_debugpins_debug_set);
    PyModule_AddIntMacro(m, MOTE_NOTIF_eui64_get);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_init);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_error_on);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_error_off);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_error_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_error_isOn);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_error_blink);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_radio_on);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_radio_off);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_radio_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_radio_isOn);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_sync_on);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_sync_off);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_sync_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_sync_isOn);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_debug_on);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_debug_off);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_debug_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_debug_isOn);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_all_on);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_all_off);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_all_toggle);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_circular_shift);
    PyModule_AddIntMacro(m, MOTE_NOTIF_leds_increment);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_init);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_reset);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_setFrequency);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_rfOn);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_rfOff);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_loadPacket);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_txEnable);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_txNow);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_rxEnable);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_rxNow);
    PyModule_AddIntMacro(m, MOTE_NOTIF_radio_getReceivedFrame);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_init);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_enableInterrupts);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_disableInterrupts);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_clearRxInterrupts);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_clearTxInterrupts);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_writeByte);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_writeCircularBuffer_FASTSIM);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_writeBufferByLen_FASTSIM);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_readByte);
    PyModule_AddIntMacro(m, MOTE_NOTIF_uart_setCTS);
    PyModule_AddIntMacro(m, MOTE_NOTIF_LAST);

    return m;
}

#endif