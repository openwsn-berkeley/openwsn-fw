/**
\brief Source code of the Python openwsn module, written in C.
*/

// Python
#include <Python.h>
#include "structmember.h"
// OpenWSN
#include "openserial.h"
#include "openserial.h"
#include "opentimers.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "neighbors.h"
#include "res.h"
#include "schedule.h"
#include "icmpv6echo.h"
#include "icmpv6rpl.h"
#include "opencoap.h"
#include "opentcp.h"
#include "ohlone.h"
#include "tcpinject.h"
#include "idmanager.h"
#include "openqueue.h"
#include "openrandom.h"

enum {
   //===== from client to server
   // board
   OPENSIM_CMD_board_init = 0,
   OPENSIM_CMD_board_sleep,
   OPENSIM_CMD_board_reset,
   // bsp_timer
   OPENSIM_CMD_bsp_timer_init,
   OPENSIM_CMD_bsp_timer_reset,
   OPENSIM_CMD_bsp_timer_scheduleIn,
   OPENSIM_CMD_bsp_timer_cancel_schedule,
   OPENSIM_CMD_bsp_timer_get_currentValue,
   // debugpins
   OPENSIM_CMD_debugpins_init,
   OPENSIM_CMD_debugpins_frame_toggle,
   OPENSIM_CMD_debugpins_frame_clr,
   OPENSIM_CMD_debugpins_frame_set,
   OPENSIM_CMD_debugpins_slot_toggle,
   OPENSIM_CMD_debugpins_slot_clr,
   OPENSIM_CMD_debugpins_slot_set,
   OPENSIM_CMD_debugpins_fsm_toggle,
   OPENSIM_CMD_debugpins_fsm_clr,
   OPENSIM_CMD_debugpins_fsm_set,
   OPENSIM_CMD_debugpins_task_toggle,
   OPENSIM_CMD_debugpins_task_clr,
   OPENSIM_CMD_debugpins_task_set,
   OPENSIM_CMD_debugpins_isr_toggle,
   OPENSIM_CMD_debugpins_isr_clr,
   OPENSIM_CMD_debugpins_isr_set,
   OPENSIM_CMD_debugpins_radio_toggle,
   OPENSIM_CMD_debugpins_radio_clr,
   OPENSIM_CMD_debugpins_radio_set,
   // eui64
   OPENSIM_CMD_eui64_get,
   // leds
   OPENSIM_CMD_leds_init,
   OPENSIM_CMD_leds_error_on,
   OPENSIM_CMD_leds_error_off,
   OPENSIM_CMD_leds_error_toggle,
   OPENSIM_CMD_leds_error_isOn,
   OPENSIM_CMD_leds_error_blink,
   OPENSIM_CMD_leds_radio_on,
   OPENSIM_CMD_leds_radio_off,
   OPENSIM_CMD_leds_radio_toggle,
   OPENSIM_CMD_leds_radio_isOn,
   OPENSIM_CMD_leds_sync_on,
   OPENSIM_CMD_leds_sync_off,
   OPENSIM_CMD_leds_sync_toggle,
   OPENSIM_CMD_leds_sync_isOn,
   OPENSIM_CMD_leds_debug_on,
   OPENSIM_CMD_leds_debug_off,
   OPENSIM_CMD_leds_debug_toggle,
   OPENSIM_CMD_leds_debug_isOn,
   OPENSIM_CMD_leds_all_on,
   OPENSIM_CMD_leds_all_off,
   OPENSIM_CMD_leds_all_toggle,
   OPENSIM_CMD_leds_circular_shift,
   OPENSIM_CMD_leds_increment,
   // radio
   OPENSIM_CMD_radio_init,
   OPENSIM_CMD_radio_reset,
   OPENSIM_CMD_radio_startTimer,
   OPENSIM_CMD_radio_getTimerValue,
   OPENSIM_CMD_radio_setTimerPeriod,
   OPENSIM_CMD_radio_getTimerPeriod,
   OPENSIM_CMD_radio_setFrequency,
   OPENSIM_CMD_radio_rfOn,
   OPENSIM_CMD_radio_rfOff,
   OPENSIM_CMD_radio_loadPacket,
   OPENSIM_CMD_radio_txEnable,
   OPENSIM_CMD_radio_txNow,
   OPENSIM_CMD_radio_rxEnable,
   OPENSIM_CMD_radio_rxNow,
   OPENSIM_CMD_radio_getReceivedFrame,
   // radiotimer
   OPENSIM_CMD_radiotimer_init,
   OPENSIM_CMD_radiotimer_start,
   OPENSIM_CMD_radiotimer_getValue,
   OPENSIM_CMD_radiotimer_setPeriod,
   OPENSIM_CMD_radiotimer_getPeriod,
   OPENSIM_CMD_radiotimer_schedule,
   OPENSIM_CMD_radiotimer_cancel,
   OPENSIM_CMD_radiotimer_getCapturedTime,
   // uart
   OPENSIM_CMD_uart_init,
   OPENSIM_CMD_uart_enableInterrupts,
   OPENSIM_CMD_uart_disableInterrupts,
   OPENSIM_CMD_uart_clearRxInterrupts,
   OPENSIM_CMD_uart_clearTxInterrupts,
   OPENSIM_CMD_uart_writeByte,
   OPENSIM_CMD_uart_readByte,
   // last
   OPENSIM_CMD_LAST
};

//=========================== OpenMote Class ==================================

/*
\brief Memory footprint of an OpenMote instance.
*/
typedef struct {
   PyObject_HEAD // No ';' allows since in macro
   //===== callbacks
   PyObject*            callback[OPENSIM_CMD_LAST];
   //===== state
   // l7
   ohlone_vars_t        ohlone_vars;
   tcpinject_vars_t     tcpinject_vars;
   // l4
   icmpv6echo_vars_t    icmpv6echo_vars;
   icmpv6rpl_vars_t     icmpv6rpl_vars;
   opencoap_vars_t      opencoap_vars;
   tcp_vars_t           tcp_vars;
   // l3
   // l2b
   neighbors_vars_t     neighbors_vars;
   res_vars_t           res_vars;
   schedule_vars_t      schedule_vars;
   schedule_dbg_t       schedule_dbg;
   // l2a
   ieee154e_vars_t      ieee154e_vars;
   ieee154e_stats_t     ieee154e_stats;
   ieee154e_dbg_t       ieee154e_dbg;
   // cross-layer
   idmanager_vars_t     idmanager_vars;
   openqueue_vars_t     openqueue_vars;
   // drivers
   opentimers_vars_t    opentimers_vars;
   random_vars_t        random_vars;
   // kernel
   scheduler_vars_t     scheduler_vars;
   scheduler_dbg_t      scheduler_dbg;
} OpenMote;

//===== members

//===== methods

static PyObject* OpenMote_set_callback(OpenMote* self, PyObject* args) {
   int       cmdId;
   PyObject* tempCallback;
   
   // parse arguments
   if (!PyArg_ParseTuple(args, "iO:set_callback", &cmdId, &tempCallback)) {
      return NULL;
   }
   
   // make sure cmdId is plausible
   if (cmdId<0 || cmdId>OPENSIM_CMD_LAST) {
      PyErr_SetString(PyExc_TypeError, "wrong cmdId");
      return NULL;
   }
   
   // make sure tempCallback is callable
   if (!PyCallable_Check(tempCallback)) {
      PyErr_SetString(PyExc_TypeError, "parameter must be callable");
      return NULL;
   }
   
   // record the callback
   Py_XINCREF(tempCallback);                // add a reference to new callback
   Py_XDECREF(self->callback[cmdId]);       // dispose of previous callback
   self->callback[cmdId] = tempCallback;    // remember new callback
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_bsp_timer_isr(OpenMote* self) {
   //poipoi bsp_timer_isr();
   Py_RETURN_NONE;
}

static PyObject* OpenMote_radio_isr_startFrame(OpenMote* self, PyObject* args) {
   int capturedTime;
   
   // parse the arguments
   if (!PyArg_ParseTuple(args, "i", &capturedTime)) {
      return NULL;
   }
   if (capturedTime>0xffff) {
      fprintf(stderr,"[OpenMote_radio_isr_startFrame] FATAL: capturedTime larger than 0xffff\n");
      return NULL;
   }
   
   // call the callback
   //poipoi radio_intr_startOfFrame((uint16_t)radio_startOfFrame->capturedTime);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_radio_isr_endFrame(OpenMote* self, PyObject* args) {
   int capturedTime;
   
   // parse the arguments
   if (!PyArg_ParseTuple(args, "i", &capturedTime)) {
      return NULL;
   }
   if (capturedTime>0xffff) {
      fprintf(stderr,"[OpenMote_radio_isr_startFrame] FATAL: capturedTime larger than 0xffff\n");
      return NULL;
   }
   
   // call the callback
   //poipoi radio_intr_endOfFrame((uint16_t)radio_startOfFrame->capturedTime);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_radiotimer_isr_compare(OpenMote* self) {
   //poipoi radiotimer_intr_compare();
   Py_RETURN_NONE;
}

static PyObject* OpenMote_radiotimer_isr_overflow(OpenMote* self) {
   //poipoi radiotimer_intr_overflow();
   Py_RETURN_NONE;
}

//===== admin

/*
\brief List of methods of the OpenMote class.
*/
static PyMethodDef OpenMote_methods[] = {
   // name                        function                                          flags          doc
   {  "set_callback",             (PyCFunction)OpenMote_set_callback,               METH_VARARGS,  ""},
   {  "bsp_timer_isr",            (PyCFunction)OpenMote_bsp_timer_isr,              METH_NOARGS,   ""},
   {  "radio_isr_startFrame",     (PyCFunction)OpenMote_radio_isr_startFrame,       METH_VARARGS,  ""},
   {  "radio_isr_endFrame",       (PyCFunction)OpenMote_radio_isr_endFrame,         METH_VARARGS,  ""},
   {  "radiotimer_isr_compare",   (PyCFunction)OpenMote_radiotimer_isr_compare,     METH_NOARGS,   ""},
   {  "radiotimer_isr_overflow",  (PyCFunction)OpenMote_radiotimer_isr_overflow,    METH_NOARGS,   ""},
   {NULL} // sentinel
};

/*
\brief List of members of the OpenMote class.
*/
static PyMemberDef OpenMote_members[] = {
   // name      type           offset                             flags   doc
   //{"first",  T_OBJECT_EX,   offsetof(Noddy, first),            0,      "first name"},
   //{"number", T_INT,         offsetof(Noddy, number),           0,      "noddy number"},
   {NULL} // sentinel
};

/*
\brief Declaration of the OpenMote type.
*/
static PyTypeObject openwsn_OpenMoteType = {
   PyObject_HEAD_INIT(NULL)
   0,                                  // ob_size
   "openwsn.OpenMote",                 // tp_name
   sizeof(OpenMote),                   // tp_basicsize
   0,                                  // tp_itemsize
   0,                                  // tp_dealloc
   0,                                  // tp_print
   0,                                  // tp_getattr
   0,                                  // tp_setattr
   0,                                  // tp_compare
   0,                                  // tp_repr
   0,                                  // tp_as_number
   0,                                  // tp_as_sequence
   0,                                  // tp_as_mapping
   0,                                  // tp_hash
   0,                                  // tp_call
   0,                                  // tp_str
   0,                                  // tp_getattro
   0,                                  // tp_setattro
   0,                                  // tp_as_buffer
   Py_TPFLAGS_DEFAULT,                 // tp_flags
   "Emulated OpenWSN mote",            // tp_doc
   0,                                  // tp_traverse
   0,                                  // tp_clear
   0,                                  // tp_richcompare
   0,                                  // tp_weaklistoffset
   0,                                  // tp_iter
   0,                                  // tp_iternext
   OpenMote_methods,                   // tp_methods
   OpenMote_members,                   // tp_member
   0,                                  // tp_getset
   0,                                  // tp_base
   0,                                  // tp_dict
   0,                                  // tp_descr_get
   0,                                  // tp_descr_set
   0,                                  // tp_dictoffset
   0,                                  // tp_init
   0,                                  // tp_alloc
   0,                                  // tp_new (populated at module initialization)
};

//=========================== openwsn module ==================================

//===== members

static PyObject* my_callback  = NULL;

//===== methods

//===== admin

static PyMethodDef openwsn_methods[] = {
   {NULL, NULL, 0, NULL} // sentinel
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initopenwsn(void) {
   PyObject* openwsn_module;
   
   // populate "new" method for OpenMote object
   openwsn_OpenMoteType.tp_new = PyType_GenericNew;
   if (PyType_Ready(&openwsn_OpenMoteType) < 0) {
      return;
   }
   
   // initialize the openwsn module
   openwsn_module = Py_InitModule3(
      "openwsn",
      openwsn_methods,
      "Module which declares the OpenMote class."
   );
   
   // create OpenMote class
   Py_INCREF(&openwsn_OpenMoteType);
   PyModule_AddObject(
      openwsn_module,
      "OpenMote",
      (PyObject*)&openwsn_OpenMoteType
   );
}
