#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>
#define debug(...) \
        { \
          if(_debug_enabled) \
             fprintf(stderr, __VA_ARGS__); \
        }
#define DEBUG_BEGIN if(_debug_enabled){ 

#define DEBUG_END }
extern int _debug_enabled;
void debug_on();
void debug_off();

#endif
