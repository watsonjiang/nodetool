#ifndef __DMPFILEPARSER_H__
#define __DMPFILEPARSER_H__

#include "dmpfileparser.yacc.h"
#include <string>
#include <vector>
struct pass_to_bison {
   PyObject * cb_handler;
   std::vector<std::string> buf;
   void * scanner;   
};
#endif
