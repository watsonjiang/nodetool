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

extern void msg_puller_start(const char * xmlfile);
static
PyObject *
py_msg_puller_start(PyObject *self, PyObject *args)
{
   char * xmlfile;
   if(!PyArg_ParseTuple(args, "s", &xmlfile))
      return NULL;
   msg_puller_start(xmlfile);
   Py_RETURN_NONE; 
}

extern void console_start(char *ip, unsigned int port);
static
PyObject *
py_console_start(PyObject *self, PyObject *args)
{
   char * ip;
   unsigned int port; 
   if(!PyArg_ParseTuple(args, "si", &ip, &port))
      return NULL;
 
   console_start(ip, port);
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
   {"msgpuller_start", (PyCFunction)py_msg_puller_start, METH_VARARGS, NULL},
   {"console_start", (PyCFunction)py_console_start, METH_VARARGS, NULL},
   {"ht_init", (PyCFunction)py_ht_init, METH_NOARGS, NULL},
   {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initpyaae() {
   Py_InitModule3("pyaae", module_methods, "a module contains myshard functions.");
}
