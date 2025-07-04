#ifndef  MACRO_H_
#define  MACRO_H_

#include  <stdio.h>

#include  "tictac.h"

#define  ARRCNT(a)  ( sizeof(a) / sizeof(*(a)) )

#define  INRANGE(x,min,max)  ({      \
	typeof(x)   _x   = x;            \
	typeof(min) _min = min;          \
	typeof(max) _max = max;          \
	((_x >= _min) && (_x <= _max));  \
})

#define  MSGF(...)  do { \
	fprintf(stdout, __VA_ARGS__);  \
	goyx(g.my, g.mx);              \
	fflush(stdout);                \
}while(0)

#define  MSGFYX(y,x,...)  do {  goyx(y,x); MSGF(__VA_ARGS__);  }while(0)

#endif //MACRO_H_
