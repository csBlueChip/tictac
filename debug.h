#ifndef  DEBUG_H_
#define  DEBUG_H_

//#define  NDEBUG
#include  <assert.h>

#ifdef  NDEBUG
#	define  DEBUGF(...)
#else
#	define  DEBUGF(...)  do {  fprintf(stderr, __VA_ARGS__);  fflush(stderr); }while(0)
#endif

#define  BPT  __asm__("int3");

#endif //DEBUG_H_

