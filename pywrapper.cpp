#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include "hashtree.h"
#include "ht.h"

typedef struct py_hashtree_st* py_hashtree_t;
struct py_hashtree_st
{
   PyObject_HEAD
   hashtree_t t;
};

static
void
py_hashtree_dealloc(py_hashtree_t t)
{
   if(NULL != t->t)
   {
      hashtree_destroy(t->t);
   }
   t->ob_type->tp_free((PyObject*)t);
}

static
PyObject *
py_hashtree_new(PyTypeObject *type, 
                PyObject *args,
                PyObject *kwds)
{
   py_hashtree_t self;
   self = (py_hashtree_t)type->tp_alloc(type, 0);
   if(NULL != self)
   {
      self->t = NULL;  
   }
   return (PyObject*)self;
}

static
int
py_hashtree_init(py_hashtree_t self, 
                 PyObject *args,
                 PyObject *kwds)
{
   char * dbname;
   if(!PyArg_ParseTuple(args, "s", &dbname))
      return -1;
   self->t = hashtree_new(dbname);
   if(NULL == self->t)
   {
      Py_INCREF(PyExc_RuntimeError);
      PyErr_SetString(PyExc_RuntimeError, "Fail to create hashtree");
      return -1;
   }
   return 0;
}

static
PyObject*
py_hashtree_insert(PyObject* self, PyObject* args)
{
   py_hashtree_t t = (py_hashtree_t)self;
   char * tname;
   unsigned int ksize;
   char * key;
   char * digest;
   Py_ssize_t dsize;
   if(!PyArg_ParseTuple(args, "sss#", 
                        &tname,
                        &key,
                        &digest,
                        &dsize))
      return NULL;
   if(dsize != 20)
   {
      Py_INCREF(PyExc_TypeError);
      PyErr_SetString(PyExc_TypeError, "digest length != 20");
      return NULL;
   }
   hashtree_insert(t->t, tname, key, (hashtree_digest_t)digest);
   Py_RETURN_NONE;
}

static
PyObject*
py_hashtree_remove(PyObject* self, PyObject* args)
{
   py_hashtree_t t = (py_hashtree_t)self;
   char * tname;
    
   Py_RETURN_NONE;
}

static
PyObject*
py_hashtree_update(PyObject* self, PyObject* args)
{
   Py_RETURN_NONE;
}

static
PyObject*
py_hashtree_get_digest(PyObject* self, PyObject* args)
{
   Py_RETURN_NONE;
}

static
PyObject*
py_hashtree_get_segment(PyObject* self, PyObject* args)
{
   Py_RETURN_NONE;
}


static
PyMethodDef py_hashtree_methods[] = {
   {"insert", (PyCFunction)py_hashtree_insert, METH_VARARGS, NULL},
   {"remove", (PyCFunction)py_hashtree_remove, METH_VARARGS, NULL},
   {"update", (PyCFunction)py_hashtree_update, METH_VARARGS, NULL},
   {"get_digest", (PyCFunction)py_hashtree_get_digest, METH_VARARGS, NULL},
   {"get_segment", (PyCFunction)py_hashtree_get_segment, METH_VARARGS, NULL},
   {NULL}
};

static
PyTypeObject py_hashtree_type_obj = {
   PyObject_HEAD_INIT(NULL)
   0,                         /*ob_size*/
   "pyaae.Hashtree",          /*tp_name*/
   sizeof(py_hashtree_st),    /*tp_basicsize*/
   0,                         /*tp_itemsize*/
   (destructor)py_hashtree_dealloc,  /*tp_dealloc*/
   0,                         /*tp_print*/
   0,                         /*tp_getattr*/
   0,                         /*tp_setattr*/
   0,                         /*tp_compare*/
   0,                         /*tp_repr*/
   0,                         /*tp_as_number*/
   0,                         /*tp_as_sequence*/
   0,                         /*tp_as_mapping*/
   0,                         /*tp_hash */
   0,                         /*tp_call*/
   0,                         /*tp_str*/
   0,                         /*tp_getattro*/
   0,                         /*tp_setattro*/
   0,                         /*tp_as_buffer*/
   Py_TPFLAGS_DEFAULT,        /*tp_flags*/
   "pyaae Hashtree type objects",  /* tp_doc */
   0,                         /* tp_traverse */
   0,                         /* tp_clear */
   0,                         /* tp_richcompare */
   0,                         /* tp_weaklistoffset */
   0,                         /* tp_iter */
   0,                         /* tp_iternext */
   py_hashtree_methods,       /* tp_methods */
   0,                         /* tp_members */
   0,                         /* tp_getset */
   0,                         /* tp_base */
   0,                         /* tp_dict */
   0,                         /* tp_descr_get */
   0,                         /* tp_descr_set */
   0,                         /* tp_dictoffset */
   (initproc)py_hashtree_init,/* tp_init */
   0,                         /* tp_alloc */
   py_hashtree_new,           /* tp_new */
};


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
   {"msgpuller_start", (PyCFunction)py_msg_puller_start, METH_VARARGS, NULL},
   {"console_start", (PyCFunction)py_console_start, METH_VARARGS, NULL},
   {"ht_init", (PyCFunction)py_ht_init, METH_NOARGS, NULL},
   {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initpyaae() {
   PyObject *m;
   if(PyType_Ready(&py_hashtree_type_obj) < 0)
      return;
   m = Py_InitModule3("pyaae", module_methods, 
                      "a module contains myshard active anti-entropy functions.");
   if(NULL == m)
      return;
   Py_INCREF(&py_hashtree_type_obj);
   PyModule_AddObject(m, "Hashtree", (PyObject*)&py_hashtree_type_obj);
}
