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

//=========================== OpenMote Class ==================================

//===== members

//===== methods

//===== admin

/*
\brief Memory footprint of an OpenMote instance.
*/
typedef struct {
   PyObject_HEAD // No ';' allows since in macro
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

/*
\brief List of methods of the OpenMote class.
*/
static PyMethodDef OpenMote_methods[] = {
   // name      function                   flags        doc
   //{"name",   (PyCFunction)Noddy_name,   METH_NOARGS, "Return the name, combining the first and last name"},
   //{"getNum", (PyCFunction)Noddy_getNum, METH_NOARGS, "" },
   {NULL} // sentinel
};

/*
\brief List of members of the OpenMote class.
*/
static PyMemberDef OpenMote_members[] = {
   // name      type           offset                             flags   doc
   //{"first",  T_OBJECT_EX,   offsetof(Noddy, first),            0,      "first name"},
   //{"last",   T_OBJECT_EX,   offsetof(Noddy, last),             0,      "last name"},
   //{"number", T_INT,         offsetof(Noddy, number),           0,      "noddy number"},
   //{"hC",     T_INT,         offsetof(Noddy, hiddenCounter),    0,      ""},
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

static PyObject* say_hello(PyObject* self, PyObject* args) {
   const char* name;
   int         a;
   int         b;
   int         sum;
   PyObject*   arglist;
   PyObject*   result;
   
   // parse the arguments
   if (!PyArg_ParseTuple(args, "s", &name)) {
      return NULL;
   }
   
   printf("Hello %s!\n", name);
   
   // call the Python function
   a          = 1;
   b          = 2;
   arglist    = Py_BuildValue("(i,i)",a,b);
   result     = PyObject_CallObject(my_callback, arglist);
   Py_DECREF(arglist);
   if (result == NULL) {
      return NULL;
   }
   if (!PyInt_Check(result)) {
      return NULL;
   }
   sum = PyInt_AsLong(result);
   Py_DECREF(result);
   printf("sum %d\n", sum);
   
   Py_RETURN_NONE;
}

static PyObject* my_set_callback(PyObject* self, PyObject *args) {
   PyObject* result = NULL;
   PyObject* temp;
   
   if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
      if (!PyCallable_Check(temp)) {
         PyErr_SetString(PyExc_TypeError, "parameter must be callable");
         return NULL;
      }
      Py_XINCREF(temp);           // Add a reference to new callback.
      Py_XDECREF(my_callback);    // Dispose of previous callback.
      my_callback = temp;         // Remember new callback.
      
      // Prepare to return "None".
      Py_INCREF(Py_None);
      result = Py_None;
   }
   return result;
}

//===== admin

static PyMethodDef openwsn_methods[] = {
   {"say_hello",        say_hello,          METH_VARARGS,  "Greet somebody."},
   {"my_set_callback",  my_set_callback,    METH_VARARGS,  "Set a callback."},
   {NULL,               NULL,               0,             NULL} // sentinel
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initopenwsn(void) {
   PyObject* openwsn_module;
   
   openwsn_OpenMoteType.tp_new = PyType_GenericNew;
   if (PyType_Ready(&openwsn_OpenMoteType) < 0) {
      return;
   }
   
   openwsn_module = Py_InitModule3(
      "openwsn",
      openwsn_methods,
      "Module which declares the OpenMote class."
   );
   
   
   
   Py_INCREF(&openwsn_OpenMoteType);
   PyModule_AddObject(
      openwsn_module,
      "OpenMote",
      (PyObject*)&openwsn_OpenMoteType
   );
}
