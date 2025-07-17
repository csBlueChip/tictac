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
void      oxoBig    (board_s* bp) ;
int       oxoChk    (void) ;
void      shadow    (board_s* bp,  int opt,  int pos) ;

void      oxo       (int id,  board_s* bp,  int x) ;
void      optShow   (board_s* bp) ;
int       optChk    (int* in) ;

void      overkill  (board_s* bp) ;

void      plmShow   (void) ;
int       plmChk    (int* in) ;

void      menuShow  (void) ;
void      menuClear (void) ;
mnuOpt_e  menuChk   (void) ;

void      seqShow   (int cnt) ;
void      seqClear  (void) ;

void      modeShow  (void) ;
void      modeClear (void) ;
int       modeChk   (void) ;

void      botShow   (void) ;
bot_e     botChk    (void) ;

#endif //GFX_H_

