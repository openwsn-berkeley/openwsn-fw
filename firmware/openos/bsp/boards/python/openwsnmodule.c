/**
\brief Source code of the Python openwsn module, written in C.
*/

#include <Python.h>

static PyObject *my_callback  = NULL;

//=========================== public ==========================================

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

//=========================== mote object =====================================

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} openwsn_OpenMoteObject;

static PyTypeObject openwsn_OpenMoteType = {
   PyObject_HEAD_INIT(NULL)
   0,                                  /*ob_size*/
   "openwsn.OpenMote",                 /*tp_name*/
   sizeof(openwsn_OpenMoteObject),     /*tp_basicsize*/
   0,                                  /*tp_itemsize*/
   0,                                  /*tp_dealloc*/
   0,                                  /*tp_print*/
   0,                                  /*tp_getattr*/
   0,                                  /*tp_setattr*/
   0,                                  /*tp_compare*/
   0,                                  /*tp_repr*/
   0,                                  /*tp_as_number*/
   0,                                  /*tp_as_sequence*/
   0,                                  /*tp_as_mapping*/
   0,                                  /*tp_hash */
   0,                                  /*tp_call*/
   0,                                  /*tp_str*/
   0,                                  /*tp_getattro*/
   0,                                  /*tp_setattro*/
   0,                                  /*tp_as_buffer*/
   Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
   "Emulated OpenWSN mote",            /* tp_doc */
};

//=========================== initialize module ===============================

static PyMethodDef openwsn_methods[] = {
   {"say_hello",        say_hello,          METH_VARARGS,  "Greet somebody."},
   {"my_set_callback",  my_set_callback,    METH_VARARGS,  "Set a callback."},
   {NULL,               NULL,               0,             NULL} // sentinel
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initopenwsn(void) {
   PyObject* openmote_object;
   
   openwsn_OpenMoteType.tp_new = PyType_GenericNew;
   if (PyType_Ready(&openwsn_OpenMoteType) < 0) {
      return;
   }
   
   openmote_object = Py_InitModule3(
      "openwsn",
      openwsn_methods,
      "Module which declares the OpenMote class."
   );
   
   Py_INCREF(&openwsn_OpenMoteType);
   PyModule_AddObject(
      openmote_object,
      "OpenMote",
      (PyObject*)&openwsn_OpenMoteType
   );
}
