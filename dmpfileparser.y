%{
#include <stdio.h>
#include <Python.h>
#include <string>
#include <vector>
#include "dmpfileparser.h"
extern int yylex(YYSTYPE * lvalp, void* scanner);
extern void yyerror(struct pass_to_bison* x, char*);
#define scanner x->scanner
%}
%pure-parser
%lex-param {void * scanner}
%parse-param {struct pass_to_bison * x}
%union
{
   char *sval;
}
%token <sval> STRING
%token <sval> NUMBER
%token NEWLINE
%token EMPTY
%token COMMA
%start lines
%%
lines : line
      | lines line
;

line: NEWLINE          
    | fields NEWLINE {
                      PyObject * list = PyList_New(0);
                      std::vector<std::string>::iterator it;
                      for(it = x->buf.begin(); it != x->buf.end(); ++it)
                      {
                         PyObject * v = PyString_FromString(it->c_str());
                         PyList_Append(list, v);
                         Py_DECREF(v);
                      }
                      if(PyCallable_Check(x->cb_handler))
                      {
                         PyObject* args = Py_BuildValue("(O)", list);
                         PyObject * rst = PyEval_CallObject(x->cb_handler, args);
                         Py_DECREF(args);
                      }
                      x->buf.clear();
                     }
;

fields: field 
     | fields COMMA field 
;

field: STRING {x->buf.push_back($1);free($1);}
     | NUMBER {x->buf.push_back($1);free($1);}
     | EMPTY  {x->buf.push_back("");}
;
%%
void 
yyerror(struct pass_to_bison* x, char *s)
{
   printf("error : %s\n", s);
}
