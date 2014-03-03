#ifndef __TEST_H__
#define __TEST_H__
#include <stdlib.h>
#define TEST_ASSERT(a, b) \
   if(!(a)) \
   { \
      printf("%s:%d - %s\n", __FILE__, __LINE__, b); \
      exit(1); \
   }

#endif
