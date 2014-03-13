#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include "hashtree.h"
#include "filtermsg.h"
#include "dmpfileparser.h"
/* Hashtree */
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
      PyErr_SetString(PyExc_RuntimeError, "digest length != 20");
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
   char * key;
   if(!PyArg_ParseTuple(args, "ss",
                        &tname,
                        &key))
      return NULL;
   hashtree_remove(t->t, tname, key);
   Py_RETURN_NONE;
}

static
PyObject*
py_hashtree_update(PyObject* self, PyObject* args)
{
   py_hashtree_t t = (py_hashtree_t)self;
   char * tname;
   if(!PyArg_ParseTuple(args, "s",
                        &tname))
      return NULL;
   hashtree_update(t->t, tname);
   Py_RETURN_NONE;
}

static
PyObject*
py_hashtree_get_digest(PyObject* self, PyObject* args)
{
   py_hashtree_t t = (py_hashtree_t)self;
   char * tname = NULL;
   unsigned int lv = -1;
   unsigned int start = -1;
   unsigned int length = -1;
   if(!PyArg_ParseTuple(args, "sI|II",
                     &tname,
                     &lv,
                     &start,
                     &length))
      return NULL;
   if(start == -1)
      start = 0;
   if(length == -1)
      length = 1;
   hashtree_digest_t* rst = 
      (hashtree_digest_t*)malloc(sizeof(hashtree_digest_t) * length);
   hashtree_get_digest(rst, t->t, tname, lv, start, length);
   PyObject *pylist = PyList_New(0);
   for(int i = 0; i < length; i++)
   {
      PyObject *digest = PyString_FromStringAndSize(rst[i], 
                                 sizeof(hashtree_digest_t));  
      PyList_Append(pylist, digest);
   }
   free(rst);         
   return pylist;
}

static
PyObject*
py_hashtree_get_segment(PyObject* self, PyObject* args)
{
   py_hashtree_t t = (py_hashtree_t)self;
   char * tname = NULL;
   unsigned int idx = -1;
   if(!PyArg_ParseTuple(args, "sI",
                     &tname,
                     &idx))
      return NULL;
   hashtree_segment_t seg = hashtree_get_segment(t->t, tname, idx);
   hashtree_segment_entry_t* s_it = (hashtree_segment_entry_t*)seg;
   int ENTRY_PREFIX_LEN = ((char*)&s_it->kstart - (char*)s_it);
   PyObject * dict = PyDict_New();
   while(s_it->ksize != 0)
   {
      PyObject * digest = PyString_FromStringAndSize(s_it->digest, 
                                 sizeof(hashtree_digest_t));
      PyObject * row_key = PyString_FromStringAndSize((char*)&s_it->kstart,
                                 s_it->ksize);
      PyDict_SetItem(dict, row_key, digest);
      s_it =(hashtree_segment_entry_t*) ((char*)s_it + ENTRY_PREFIX_LEN + s_it->ksize);
   }
   
   return dict;
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

/*  Filterlist */
typedef struct py_filterlist_st* py_filterlist_t;
struct py_filterlist_st
{
   PyObject_HEAD
   filter_list_t t;
};

static
void
py_filterlist_dealloc(py_filterlist_t t)
{
   if(NULL != t->t)
   {
      filter_list_destroy(t->t);
   }
   t->ob_type->tp_free((PyObject*)t);
}

static
PyObject *
py_filterlist_new(PyTypeObject *type, 
                PyObject *args,
                PyObject *kwds)
{
   py_filterlist_t self;
   self = (py_filterlist_t)type->tp_alloc(type, 0);
   if(NULL != self)
   {
      self->t = NULL;  
   }
   return (PyObject*)self;
}

static
int
py_filterlist_init(py_filterlist_t self, 
                 PyObject *args,
                 PyObject *kwds)
{
   if(!PyArg_ParseTuple(args, ""))
      return -1;
   self->t = filter_list_new();
   if(NULL == self->t)
   {
      PyErr_SetString(PyExc_RuntimeError, "Fail to create filter list.");
      return -1;
   }
   return 0;
}

static
PyObject*
py_filterlist_add(py_filterlist_t self, PyObject* args)
{
   char * tname;
   char * colname;
   unsigned int func_id;
   if(!PyArg_ParseTuple(args, "ssI", 
                              &tname,
                              &colname,
                              &func_id))
      return NULL;
   filter_func_t f = NULL;
   switch(func_id)
   {
      case 1:
         f = filter_func_datetime;
         break;
      case 2:
         f = filter_func_number;
         break;
   }

   filter_list_add(self->t, tname, colname, f);
   Py_RETURN_NONE;
}

static
PyObject*
py_filterlist_rm(py_filterlist_t self, PyObject *args)
{
   char * tname;
   char * colname;
   if(!PyArg_ParseTuple(args, "ss", 
                              &tname,
                              &colname))
      return NULL;
   filter_list_rm(self->t, tname, colname); 
   Py_RETURN_NONE;
}

static
PyMethodDef py_filterlist_methods[] = {
   {"add", (PyCFunction)py_filterlist_add, METH_VARARGS, NULL},
   {"remove", (PyCFunction)py_filterlist_rm, METH_VARARGS, NULL},
   {NULL}
};


static
PyTypeObject py_filterlist_type_obj = {
   PyObject_HEAD_INIT(NULL)
   0,                         /*ob_size*/
   "pyaae._Filterlist",          /*tp_name*/
   sizeof(py_filterlist_st),    /*tp_basicsize*/
   0,                         /*tp_itemsize*/
   (destructor)py_filterlist_dealloc,  /*tp_dealloc*/
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
   "pyaae _Filterlist type objects",  /* tp_doc */
   0,                         /* tp_traverse */
   0,                         /* tp_clear */
   0,                         /* tp_richcompare */
   0,                         /* tp_weaklistoffset */
   0,                         /* tp_iter */
   0,                         /* tp_iternext */
   py_filterlist_methods,       /* tp_methods */
   0,                         /* tp_members */
   0,                         /* tp_getset */
   0,                         /* tp_base */
   0,                         /* tp_dict */
   0,                         /* tp_descr_get */
   0,                         /* tp_descr_set */
   0,                         /* tp_dictoffset */
   (initproc)py_filterlist_init,/* tp_init */
   0,                         /* tp_alloc */
   py_filterlist_new,           /* tp_new */
};

/* msg puller */
typedef struct py_mp_st* py_mp_t;
struct py_mp_st
{
   PyObject_HEAD
   PyObject * xmlfile;
   py_filterlist_t filter;
   py_hashtree_t hashtree;
};

static
void
py_mp_dealloc(py_mp_t t)
{
   Py_XDECREF(t->filter);
   Py_XDECREF(t->hashtree);
   t->ob_type->tp_free((PyObject*)t);
}

static
PyObject *
py_mp_new(PyTypeObject *type, 
                      PyObject *args,
                      PyObject *kwds)
{
   py_mp_t self;
   self = (py_mp_t)type->tp_alloc(type, 0);
   if(NULL != self)
   {
      self->xmlfile = NULL;  
      self->hashtree = NULL;  
      self->filter = NULL;  
   }
   return (PyObject*)self;
}

static
int
py_mp_init(py_mp_t self, 
                       PyObject *args,
                       PyObject *kwds)
{
   PyObject * xmlfile;
   PyObject * hashtree;
   PyObject * filter;
   if(!PyArg_ParseTuple(args, "OOO",
                        &xmlfile,
                        &hashtree,
                        &filter))
      return -1;
    
   Py_XDECREF(self->xmlfile);
   Py_XINCREF(xmlfile);
   self->xmlfile = xmlfile;

   Py_XDECREF(self->hashtree);
   Py_XINCREF(hashtree);
   self->hashtree = (py_hashtree_t)hashtree;

   Py_XDECREF(self->filter);
   Py_XINCREF(filter);
   self->filter = (py_filterlist_t)filter;

   return 0;
}

extern void msg_puller_start(const char * xmlfile, hashtree_t t, filter_list_t f);
static
PyObject*
py_mp_start(py_mp_t self, PyObject* args)
{
   //msg_puller_start(PyString_AsString(self->xmlfile), self->hashtree->t, self->filter->t);
   Py_RETURN_NONE; 
}

static
PyMethodDef py_mp_methods[] = {
   {"start", (PyCFunction)py_mp_start, METH_NOARGS, NULL},
   {NULL}
};


static
PyTypeObject py_mp_type_obj = {
   PyObject_HEAD_INIT(NULL)
   0,                         /*ob_size*/
   "pyaae.Msgpuller",          /*tp_name*/
   sizeof(py_mp_st),    /*tp_basicsize*/
   0,                         /*tp_itemsize*/
   (destructor)py_mp_dealloc,  /*tp_dealloc*/
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
   "pyaae MsgPuller type objects",  /* tp_doc */
   0,                         /* tp_traverse */
   0,                         /* tp_clear */
   0,                         /* tp_richcompare */
   0,                         /* tp_weaklistoffset */
   0,                         /* tp_iter */
   0,                         /* tp_iternext */
   py_mp_methods,             /* tp_methods */
   0,                         /* tp_members */
   0,                         /* tp_getset */
   0,                         /* tp_base */
   0,                         /* tp_dict */
   0,                         /* tp_descr_get */
   0,                         /* tp_descr_set */
   0,                         /* tp_dictoffset */
   (initproc)py_mp_init,/* tp_init */
   0,                         /* tp_alloc */
   py_mp_new,           /* tp_new */
};

typedef void* yyscan_t;
extern int yylex_init (yyscan_t* scanner);
extern int yylex_destroy (yyscan_t yyscanner );
extern void yyset_in  (FILE * in_str ,yyscan_t yyscanner );
static
PyObject*
py_parse_dumpfile(PyObject* self, PyObject *args)
{
   char * filename;
   PyObject * cb_handler;
   if(!PyArg_ParseTuple(args, "sO", 
                        &filename,
                        &cb_handler))
      return NULL;
   FILE * fp = fopen(filename, "r");
   if(!fp)
   {
      char buf[100] = {0};
      snprintf(sizeof(buf), buf, "fail to open file [%s]", filename);
      PyErr_SetString(PyExc_RuntimeError, buf);
      return NULL;
   }
   //yydebug=1;
   struct pass_to_bison x;
   Py_INCREF(cb_handler);
   x.cb_handler = cb_handler;
   yylex_init(&x.scanner);
   yyset_in(fp, x.scanner);
   yyparse(&x);
   yylex_destroy(x.scanner);
   fclose(fp);
   Py_DECREF(cb_handler);
   Py_RETURN_NONE;
}

static
PyMethodDef module_methods[] = {
   {"parse_dumpfile", (PyCFunction)py_parse_dumpfile, METH_VARARGS, "parse the myshard data dump file."},
   {NULL}
};

PyMODINIT_FUNC initpyaae() {
   PyObject *m;
   if(PyType_Ready(&py_hashtree_type_obj) < 0)
      return;
   if(PyType_Ready(&py_filterlist_type_obj) < 0)
      return;
   if(PyType_Ready(&py_mp_type_obj) < 0)
      return;
   m = Py_InitModule3("pyaae", module_methods, 
                      "a module contains myshard active anti-entropy functions.");
   if(NULL == m)
      return;

   Py_INCREF(&py_hashtree_type_obj);
   PyModule_AddObject(m, "Hashtree", (PyObject*)&py_hashtree_type_obj);
   Py_INCREF(&py_filterlist_type_obj);
   PyModule_AddObject(m, "_Filterlist", (PyObject*)&py_filterlist_type_obj);
   Py_INCREF(&py_mp_type_obj);
   PyModule_AddObject(m, "Msgpuller", (PyObject*)&py_mp_type_obj);
}
