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

//=========================== private =========================================

//=========================== initialize module ===============================

static PyMethodDef OpenWSNMethods[] = {
   {"say_hello",        say_hello,          METH_VARARGS,  "Greet somebody."},
   {"my_set_callback",  my_set_callback,    METH_VARARGS,  "Set a callback."},
   {NULL,               NULL,               0,             NULL}
};

PyMODINIT_FUNC initopenwsn(void) {
   (void)Py_InitModule("openwsn", OpenWSNMethods);
}
