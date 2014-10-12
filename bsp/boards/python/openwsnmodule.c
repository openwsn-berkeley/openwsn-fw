/**
\brief Source code of the Python openwsn module, written in C.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include <stdio.h>
#include "openwsnmodule.h"

//=========================== OpenMote Class ==================================

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
   if (cmdId<0 || cmdId>MOTE_NOTIF_LAST) {
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

static PyObject* OpenMote_getState(OpenMote* self) {
   PyObject* returnVal;
   PyObject* uart_icb_tx;
   PyObject* uart_icb_rx;
   PyObject* bsp_timer_icb_cb;
   PyObject* radio_icb_startFrame_cb;
   PyObject* radio_icb_endFrame_cb;
   PyObject* radiotimer_icb_overflow_cb;
   PyObject* radiotimer_icb_compare_cb;
   //PyObject* ohlone_vars;
   PyObject* tcpinject_vars;
   PyObject* icmpv6echo_vars;
   PyObject* icmpv6rpl_vars;
   PyObject* opencoap_vars;
   PyObject* tcp_vars;
   PyObject* neighbors_vars;
   PyObject* sixtop_vars;
   PyObject* schedule_vars;
   PyObject* schedule_dbg;
   PyObject* ieee154e_vars;
   PyObject* ieee154e_stats;
   PyObject* ieee154e_dbg;
   PyObject* idmanager_vars;
   PyObject* openqueue_vars;
   PyObject* opentimers_vars;
   PyObject* random_vars;
   PyObject* openserial_vars;
   PyObject* scheduler_vars;
   PyObject* scheduler_dbg;
   
   returnVal = PyDict_New();
   
   // callbacks
   uart_icb_tx                    = PyInt_FromLong((intptr_t)self->uart_icb.txCb);
   PyDict_SetItemString(returnVal, "uart_icb_tx", uart_icb_tx);
   uart_icb_rx                    = PyInt_FromLong((intptr_t)self->uart_icb.rxCb);
   PyDict_SetItemString(returnVal, "uart_icb_rx", uart_icb_rx);
   bsp_timer_icb_cb               = PyInt_FromLong((intptr_t)self->bsp_timer_icb.cb);
   PyDict_SetItemString(returnVal, "bsp_timer_icb_cb", bsp_timer_icb_cb);
   radio_icb_startFrame_cb        = PyInt_FromLong((intptr_t)self->radio_icb.startFrame_cb);
   PyDict_SetItemString(returnVal, "radio_icb_startFrame_cb", radio_icb_startFrame_cb);
   radio_icb_endFrame_cb          = PyInt_FromLong((intptr_t)self->radio_icb.endFrame_cb);
   PyDict_SetItemString(returnVal, "radio_icb_endFrame_cb", radio_icb_endFrame_cb   );
   radiotimer_icb_overflow_cb     = PyInt_FromLong((intptr_t)self->radiotimer_icb.overflow_cb);
   PyDict_SetItemString(returnVal, "radiotimer_icb_overflow_cb", radiotimer_icb_overflow_cb);
   radiotimer_icb_compare_cb      = PyInt_FromLong((intptr_t)self->radiotimer_icb.compare_cb);
   PyDict_SetItemString(returnVal, "radiotimer_icb_compare_cb", radiotimer_icb_compare_cb);
   
   // ohlone_vars
   /*
   ohlone_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "ohlone_vars", ohlone_vars);
   */
   
   // tcpinject_vars
   tcpinject_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "tcpinject_vars", tcpinject_vars);
   
   // icmpv6echo_vars
   icmpv6echo_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "icmpv6echo_vars", icmpv6echo_vars);
   
   // icmpv6rpl_vars
   icmpv6rpl_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "icmpv6rpl_vars", icmpv6rpl_vars);
   
   // opencoap_vars
   opencoap_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "opencoap_vars", opencoap_vars);
   
   // tcp_vars
   tcp_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "tcp_vars", tcp_vars);
   
   // neighbors_vars
   neighbors_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "neighbors_vars", neighbors_vars);
   
   // sixtop_vars
   sixtop_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "sixtop_vars", sixtop_vars);
   
   // schedule_vars
   schedule_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "schedule_vars", schedule_vars);
   
   // schedule_dbg
   schedule_dbg = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "schedule_dbg", schedule_dbg);
   
   // ieee154e_vars
   ieee154e_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "ieee154e_vars", ieee154e_vars);
   
   // ieee154e_stats
   ieee154e_stats = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "ieee154e_stats", ieee154e_stats);
   
   // ieee154e_dbg
   ieee154e_dbg = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "ieee154e_dbg", ieee154e_dbg);
   
   // idmanager_vars
   idmanager_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "idmanager_vars", idmanager_vars);
   
   // openqueue_vars
   openqueue_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "openqueue_vars", openqueue_vars);
   
   // opentimers_vars
   opentimers_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "opentimers_vars", opentimers_vars);
   
   // random_vars
   random_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "random_vars", random_vars);
   
   // openserial_vars
   openserial_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "openserial_vars", openserial_vars);
   
   // scheduler_vars
   scheduler_vars = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "scheduler_vars", scheduler_vars);
   
   // scheduler_dbg
   scheduler_dbg = PyDict_New();
   // TODO
   PyDict_SetItemString(returnVal, "scheduler_dbg", scheduler_dbg);
   
   return returnVal;
}

static PyObject* OpenMote_bsp_timer_isr(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   bsp_timer_isr(self);
   
   // return successfully
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
      // TODO raise exception
      return NULL;
   }
   
   // call the callback
   radio_intr_startOfFrame(
      self,
      (uint16_t)capturedTime
   );
   
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
      // TODO raise exception
      return NULL;
   }
   
   // call the callback
   radio_intr_endOfFrame(
      self,
      (uint16_t)capturedTime
   );
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_radiotimer_isr_compare(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   radiotimer_intr_compare(self);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_radiotimer_isr_overflow(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   radiotimer_intr_overflow(self);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_uart_isr_tx(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   uart_intr_tx(self);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_uart_isr_rx(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   uart_intr_rx(self);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_supply_on(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   supply_on(self);
   
   // return successfully
   Py_RETURN_NONE;
}

static PyObject* OpenMote_supply_off(OpenMote* self) {
   
   // no arguments
   
   // call the callback
   supply_off(self);
   
   // return successfully
   Py_RETURN_NONE;
}

//===== admin

/*
\brief List of methods of the OpenMote class.
*/
static PyMethodDef OpenMote_methods[] = {
   // name                        function                                          flags          doc
   //=== admin
   {  "set_callback",             (PyCFunction)OpenMote_set_callback,               METH_VARARGS,  ""},
   {  "getState",                 (PyCFunction)OpenMote_getState,                   METH_NOARGS,   ""},
   //=== BSP
   {  "bsp_timer_isr",            (PyCFunction)OpenMote_bsp_timer_isr,              METH_NOARGS,   ""},
   {  "radio_isr_startFrame",     (PyCFunction)OpenMote_radio_isr_startFrame,       METH_VARARGS,  ""},
   {  "radio_isr_endFrame",       (PyCFunction)OpenMote_radio_isr_endFrame,         METH_VARARGS,  ""},
   {  "radiotimer_isr_compare",   (PyCFunction)OpenMote_radiotimer_isr_compare,     METH_NOARGS,   ""},
   {  "radiotimer_isr_overflow",  (PyCFunction)OpenMote_radiotimer_isr_overflow,    METH_NOARGS,   ""},
   {  "uart_isr_tx",              (PyCFunction)OpenMote_uart_isr_tx,                METH_NOARGS,   ""},
   {  "uart_isr_rx",              (PyCFunction)OpenMote_uart_isr_rx,                METH_NOARGS,   ""},
   {  "supply_on",                (PyCFunction)OpenMote_supply_on,                  METH_NOARGS,   ""},
   {  "supply_off",               (PyCFunction)OpenMote_supply_off,                 METH_NOARGS,   ""},
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
   "openwsn_generic.OpenMote",         // tp_name
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

PyMODINIT_FUNC initopenwsn_generic(void) {
   PyObject* openwsn_module;
   
   // populate "new" method for OpenMote object
   openwsn_OpenMoteType.tp_new = PyType_GenericNew;
   if (PyType_Ready(&openwsn_OpenMoteType) < 0) {
      return;
   }
   
   // initialize the openwsn module
   openwsn_module = Py_InitModule3(
      "openwsn_generic",
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
