#ifndef  ANAL_H_
#define  ANAL_H_

#include  "tictac.h"

//-----------------------------------------------------------------------------
// The x coordinate of option #N
#define OPTX(n)  ( ((n) * g.optW) + 2 )

//=============================================================================
int*  lookahead (board_s* bp,  int depth) ;

void  analyse   (board_s* bp,  int st,  int nd) ;

void  oxoAnal   (int id,  board_s* bp,  int x) ;
void  analClr   (int x) ;

#endif //ANAL_H_

