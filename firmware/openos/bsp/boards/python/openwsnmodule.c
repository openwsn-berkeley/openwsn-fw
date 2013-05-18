/**
\brief Source code of the Python openwsn module, written in C.
*/

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
