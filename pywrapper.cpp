#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include "hashtree.h"
#include "ht.h"

static
PyObject *
py_hashtree_init(PyObject *self, PyObject *args)
{
   hashtree_init();
   Py_RETURN_NONE;
}

static
PyObject *
py_hashtree_destroy(PyObject *self, PyObject *args)
{
   hashtree_destroy();
}

extern void msg_puller_start();
static
PyObject *
py_msg_puller_start(PyObject *self, PyObject *args)
{
   msg_puller_start();
   Py_RETURN_NONE; 
}

extern void console_start();
static
PyObject *
py_console_start(PyObject *self, PyObject *args)
{
   console_start();
   Py_RETURN_NONE; 
}

static
PyObject *
py_ht_init(PyObject *self, PyObject *args)
{
   ht_init();
   Py_RETURN_NONE; 
}



static PyMethodDef module_methods[] = {
   {"hashtree_init", (PyCFunction)py_hashtree_init, METH_NOARGS, NULL},
   {"hashtree_destroy", (PyCFunction)py_hashtree_destroy, METH_NOARGS, NULL},
   {"msgpuller_start", (PyCFunction)py_msg_puller_start, METH_NOARGS, NULL},
   {"console_start", (PyCFunction)py_console_start, METH_NOARGS, NULL},
   {"ht_init", (PyCFunction)py_ht_init, METH_NOARGS, NULL},
   {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initpyaae() {
   Py_InitModule3("pyaae", module_methods, "a module contains myshard functions.");
}
