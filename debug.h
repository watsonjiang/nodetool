#ifndef __DEBUG_H__
#define __DEBUG_H__

#if _DEBUG_
#define debug(...) \
        fprintf(stderr, __VA_ARGS__);
#else
#define debug(...)   //NOP
#endif


#endif
