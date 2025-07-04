#ifndef  GFX_H_
#define  GFX_H_

#include  "tictac.h"
#include  "ansi.h"

//----------------------------------------------------------------------------- ---------------------------------------
#define  C_INVALID  (DBLU)

#define  C_LOSE     (BRED)
#define  C_LOSE2    ( MAG)

#define  C_WEAK     ( CYN)
#define  C_FAIR     ( WHT)
#define  C_STRONG   (BYEL)

#define  C_WIN      (BGRN)
#define  C_WIN2     (DGRN)

#define  C_GAME     (BWHT)

//----------------------------------------------------------------------------- ---------------------------------------
typedef
	enum mnuOpt {
		MNU_NONE  = 0,
		MNU_UNDO,
		MNU_AGAIN,
		MNU_ANAL,
		MNU_REDO,
		MNU_QUIT,
	}
mnuOpt_e;

//----------------------------------------------------------------------------- ---------------------------------------
mnuOpt_e  menuChk   (int y,  int x) ;
void      menuShow  (void) ;
void      menuClear (void) ;

void      oxoBig    (board_s* bp) ;
int       oxoChk    (void) ;

void      oxo       (int id,  board_s* bp,  int x) ;

void      seqShow   (int y,  int cnt) ;
void      seqClear  (void) ;

void      optShow   (board_s* bp) ;
int       optChk    (int* in) ;

void      modeShow  (int y,  int x) ;
void      modeClear (void) ;
int       modeChk   (void) ;

void      overkill  (board_s* bp) ;

void      botShow   (void) ;
bot_e     botChk    (void) ;

void      analClr   (int x) ;

#endif //GFX_H_

