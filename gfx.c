#include  <stdio.h>

#include  "anal.h"
#include  "ansi.h"
#include  "tictac.h"
#include  "macro.h"
#include  "gfx.h"
#include  "logic.h"


//+============================================================================
// g.prefs is an array of struct {0..8} are for each board option
//                               {9}    is the game board (no longer used)
// it is the result of the AI analysis
//
// assuming analysis results are not hidden:
//  this function will use .ink to choose the colour of the grid
//  oxoStats will use other values in that struct [qv. tictac.h]
//
void  oxo (int id,  board_s* bp,  int x)
{
	ink(g.pref[id].ink);

	if ((g.hide) && (g.pref[id].ink != C_INVALID))  ink(BWHT) ;

	goyx(g.yOpt+0,x+3);  printf("[-%d-]", id);

	goyx(g.yOpt+1,x  );  printf("   |   |   ");
	goyx(g.yOpt+2,x  );  printf("---|---|---");
	goyx(g.yOpt+3,x  );  printf("   |   |   ");
	goyx(g.yOpt+4,x  );  printf("---|---|---");
	goyx(g.yOpt+5,x  );  printf("   |   |   ");

	// clear analysis results
	for (int yy = g.yOpt +7;  yy <= g.yOpt +18;  yy++) {
		goyx(yy,x);
		printf("           ");
	}

	// optionally display new results
	if (!g.hide && (id != 9))  oxoAnal(id, bp, x) ;

	// 9 pieces
	goyx(g.yOpt+1,x+1);  printf(who(bp, POS_TL));
	goyx(g.yOpt+1,x+5);  printf(who(bp, POS_TC));
	goyx(g.yOpt+1,x+9);  printf(who(bp, POS_TR));

	goyx(g.yOpt+3,x+1);  printf(who(bp, POS_ML));
	goyx(g.yOpt+3,x+5);  printf(who(bp, POS_MC));
	goyx(g.yOpt+3,x+9);  printf(who(bp, POS_MR));

	goyx(g.yOpt+5,x+1);  printf(who(bp, POS_BL));
	goyx(g.yOpt+5,x+5);  printf(who(bp, POS_BC));
	goyx(g.yOpt+5,x+9);  printf(who(bp, POS_BR));

	ink(NORM);

	fflush(stdout);
}

//+============================================================================ =======================================
// This comment will stop `grep` from working! (cos unicode)
//
//	  ▄   ▄  ║   ▄▄▄   ║
//	  ▀▄ ▄▀  ║  █   █  ║
//	    █    ║  ▌   ▐  ║
//	  ▄▀ ▀▄  ║  █   █  ║
//	  ▀   ▀  ║   ▀▀▀   ║
//	═════════╬═════════╬═════════
//	         ║         ║
//	  ▀▄ ▄▀  ║  █▀▀▀█  ║
//	    █    ║  ▌   ▐  ║
//	  ▄▀ ▀▄  ║  █▄▄▄█  ║
//	         ║         ║
//	═════════╬═════════╬═════════
//	         ║         ║
//	   █ █   ║   █▀█   ║
//	    █    ║   █ █   ║
//	   █ █   ║   █▄█   ║
//	         ║         ║
//
//	═════════╬═════════╬═════════
//	         ║         ║
//	   ▄ ▄   ║   ▄▄▄   ║
//	    █    ║   █ █   ║
//	   ▀ ▀   ║   ▀▀▀   ║
//	         ║         ║
//
static
void  _grid (int y,  int x)
{
#	define  V  "\u2551" "\n\b"     // line : vertical
//#	define  V  "\u2551" "\e[1B\b"  // line : vertical
#	define  H  "\u2550"            // line : horizontal
#	define  P  "\u256C"            // intersection: plus

	ink(WHT);

	goyx(y   , x+ 9);  printf("%s", V V V V V  V  V V V V V  V  V V V V V );
	goyx(y   , x+19);  printf("%s", V V V V V  V  V V V V V  V  V V V V V );
	goyx(y+ 5, x   );  printf("%s", H H H H H H H H H  P  H H H H H H H H H  P  H H H H H H H H H);
	goyx(y+11, x   );  printf("%s", H H H H H H H H H  P  H H H H H H H H H  P  H H H H H H H H H);

#	undef  V
#	undef  H
#	undef  P
}

//-----------------------------------------------------------------------------
#	define  U  "\u2580"
#	define  D  "\u2584"
#	define  L  "\u258C"
#	define  R  "\u2590"
#	define  F  "\u2588"
#	define  N  " "

static  const char* const  icon[2][4][5] = {
	{	{ N D D D N,  F N N N F,  L N N N R,  F N N N F,  N U U U N },  // O : Big
		{ N N N N N,  F U U U F,  L N N N R,  F D D D F,  N N N N N },  //     Med
		{ N N N N N,  N F U F N,  N F N F N,  N F D F N,  N N N N N },  //     Sml
		{ N N N N N,  N D D D N,  N F N F N,  N U U U N,  N N N N N }   //     Tiny
	},
	{	{ D N N N D,  U D N D U,  N N F N N,  D U N U D,  U N N N U },  // X : Big
		{ N N N N N,  U D N D U,  N N F N N,  D U N U D,  N N N N N },  //     Med
		{ N N N N N,  N F N F N,  N N F N N,  N F N F N,  N N N N N },  //     Sml
		{ N N N N N,  N D N D N,  N N F N N,  N U N U N,  N N N N N }   //     Tiny
	}
};
static  const char* const empty[5] = 
		{ N N N N N,  N N N N N,  N N N N N,  N N N N N,  N N N N N };  // empty

static  int  iconClr[2][4] = {
	{ BRED, RED, RED, DRED },  // O colours
	{ BCYN, CYN, CYN, DCYN },  // X colours
};

#	undef  U
#	undef  D
#	undef  L
#	undef  R
#	undef  F
#	undef  N

//+============================================================================
static
void  _piece (int y,  int x,  board_s* bp,  int pos)
{
	int                 occ = occupier(bp, pos);    // original board occupier
	const char* const*  ico = empty;                // icon pointer

	if (occ) {
		occ = (occ ^ (g.par *3)) -1;    // adjust occupier against game parity (for looping games)

		if (g.hide || (g.loop == 9)) {  // All look same
			ink(iconClr[occ][0]);
			ico = icon[occ][0];

		} else {                        // Fade by sequence
			int  seq;
			for (seq = 0;  seq < 4;  seq++)
				if ( ( ((bp->seq >> (((seq*2)  ) *4)) & 0xF) == pos ) ||
				     ( ((bp->seq >> (((seq*2)+1) *4)) & 0xF) == pos )    )  break ;
			if ((g.loop <= 6) && (seq == 2))  seq = 3 ;
			ink(iconClr[occ][seq]);
			ico = icon[occ][seq];
		}
	}

	// Show
	for (int i = 0;  i < 5;  i++) {
		goyx(y+i, x);
		printf("%s", ico[i]);
	}
}

//+============================================================================
void  oxoBig (int y,  int x,  board_s* bp)
{
	_grid(y,x);

	_piece(y+ 0, x+ 2, bp, POS_TL);
	_piece(y+ 0, x+12, bp, POS_TC);
	_piece(y+ 0, x+22, bp, POS_TR);

	_piece(y+ 6, x+ 2, bp, POS_ML);
	_piece(y+ 6, x+12, bp, POS_MC);
	_piece(y+ 6, x+22, bp, POS_MR);

	_piece(y+12, x+ 2, bp, POS_BL);
	_piece(y+12, x+12, bp, POS_BC);
	_piece(y+12, x+22, bp, POS_BR);

	fflush(stdout);
}

//----------------------------------------------------------------------------- ---------------------------------------
static  int               mnuY       = -1;
static  int               mnuX       = -1;

static  const char const  menuStr1[] = "[UNDO]  [REDO]  [ANAL]";
                                   //   0123456789012345678901234567890
                                   //           ` 1  `  `   2`        3

static  const char const  menuStr2[] = "   [RESTART]  [QUIT]";
                                   //   0123456789012345678901234567890
                                   //             1`  `    `2         3

//+============================================================================
void  menuShow (int y,  int x)
{
	mnuY = y;
	mnuX = x;

	ink(BYEL);
	goyx(y  , x);  printf(menuStr1);
	goyx(y+2, x);  printf(menuStr2);

	if (!g.hide) {
		paper(ONRED);
		goyx(y, x+16);  printf("[ANAL]");
	}

	fflush(stdout);
}

//+============================================================================
void  menuClear (void)
{
	ink(NORM);
	goyx(mnuY  , mnuX);  printf("%*s", sizeof(menuStr1)-1, "");
	goyx(mnuY+2, mnuX);  printf("%*s", sizeof(menuStr2)-1, "");
	fflush(stdout);

	mnuY = -1;
	mnuX = -1;
}

//+============================================================================
mnuOpt_e  menuChk (int y,  int x)
{
	if        (y == mnuY) {
		if (INRANGE(x, mnuX+ 0, mnuX+ 5))  return MNU_UNDO  ;
		if (INRANGE(x, mnuX+ 8, mnuX+13))  return MNU_REDO  ;
		if (INRANGE(x, mnuX+16, mnuX+21))  return MNU_ANAL  ;
	} else if (y == mnuY+2) {
		if (INRANGE(x, mnuX+ 3, mnuX+11))  return MNU_AGAIN ;
		if (INRANGE(x, mnuX+14, mnuX+19))  return MNU_QUIT  ;
	}
	return MNU_NONE;
}

//----------------------------------------------------------------------------- ---------------------------------------
static  int  modeY = -1;
static  int  modeX = -1;

//+============================================================================
void  modeShow (int y,  int x)
{
	modeY = y;
	modeX = x;

	ink(BYEL);
	paper(ONBLK);
	goyx(y, x);
	for (int i = 5;  i <= 9;  i++) {
		if (g.loop == i)  paper(ONRED) ;
		printf("[%d]", i);
		paper(ONBLK);
		printf("  ");
	}

	fflush(stdout);
}

//+============================================================================
int  modeChk (void)
{
	if (modeY == -1)  return 0 ;

	if (g.my == modeY)
		for (int i = 5-5;  i <= 9-5;  i++)
			if ( INRANGE(g.mx, modeX+(i*5), mnuX+(i*5)+3) )  {
				modeShow(modeY, modeX);
				return i+5 ;
			}
	return 0;
}

//+============================================================================
void  modeClear (void)
{
	ink(NORM);
	goyx(modeY, modeX);
	printf("%*s", (5*5)-2, "");
	fflush(stdout);

	modeY = -1;
	modeX = -1;
}

//----------------------------------------------------------------------------- ---------------------------------------
static  int  seqY = -1;

//+============================================================================
void  seqShow (int y,  int cnt)
{
	int  i = 0;

	seqY = y;

	MSGFYX(y,1, "\e[K");  // clear old sequence
	goyx(y,1);

	ink(BWHT);  while (i < g.move)  printf("%d, ", g.play[i++].in) ;
	ink(DGRY);  while (i < g.last)  printf("%d, ", g.play[i++].in) ;
}

//+============================================================================
void  seqClear (void)
{
	if (seqY == -1)  return ;

	MSGFYX(seqY,1, "\e[K");

	seqY = -1;
}

//+============================================================================ =======================================
// draw ALL children (even greyed out moves)

int  wOpt = 13;  // width of an option

void  optShow (board_s* bp)
{
	int  cidx = 0;
	while (cidx < bp->cCnt)  oxo(cidx++, bp->chld[cidx], (cidx *wOpt) +1) ;
	while (cidx < 9       )  oxo(cidx++, &g.b[0],        (cidx *wOpt) +1) ;
}

//+============================================================================
int   optChk (int* in)
{
	int  h = g.hide ? 5 : (g.loop ==9) ? 8 : 18 ;

	// selecting an option?
	if (INRANGE(g.my, g.yOpt, g.yOpt +h)) {  // y coord for options
		int x = (g.mx -1) /wOpt;             // selection
		if (INRANGE(x, 0, 8)) {              // 0..8
			if ( INRANGE(g.mx, (x*wOpt)+1, (x*wOpt)+wOpt-2) )
				return (*in = x + '0');      // fake like we just pressed the number
		}
	}
	return -1;
}
