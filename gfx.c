#include  <stdio.h>

#include  "ansi.h"
#include  "anal.h"
#include  "tictac.h"
#include  "macro.h"
#include  "gfx.h"
#include  "logic.h"
#include  "conio.h"
#include  "bot.h"

int  wOpt = 13;  // width of an option

#define OPTX(n)  ( ((n) * g.optW) + 2 )

//+============================================================================ =======================================
// clear old analysis results
//
void  _analClr (int x)
{
	for (int y = g.optY +5 +2;  y <= g.optY +5 +g.analH;  y++) {
		goyx(y,x);
		printf("           ");
	}
}

//+============================================================================
// -1 will clear ALL analysis blocks
//
void  analClr (int x)
{
	if (x != -1)  _analClr(x);
	else          for (int i = 0;  i < 9;  i++)  _analClr(OPTX(i)) ;
}

//+============================================================================ =======================================
// Draw a SMALL oxo grid
//
// g.prefs is an array of struct {0..8} are for each board option
//                               {9}    is the game board [no longer used]
// it is the result of the Bot analysis
//
// assuming analysis results are not hidden:
//  this function will use .ink to choose the colour of the grid
//  oxoStats will use other values in that struct [qv. tictac.h]
//
void  oxo (int id,  board_s* bp,  int x)
{
	ink(g.pref[id].ink);

	if ((g.hide) && (g.pref[id].ink != C_INVALID))  ink(BWHT) ;

	goyx(g.optY+0,x+3);  printf("[-%d-]", id);

	goyx(g.optY+1,x  );  printf("   |   |   ");
	goyx(g.optY+2,x  );  printf("---|---|---");
	goyx(g.optY+3,x  );  printf("   |   |   ");
	goyx(g.optY+4,x  );  printf("---|---|---");
	goyx(g.optY+5,x  );  printf("   |   |   ");

	_analClr(x);

	// optionally display new analysis results
	if (!g.hide && (id != 9))  oxoAnal(id, bp, x) ;

	// 9 pieces
	goyx(g.optY+1,x+1);  printf(who(bp, POS_TL));
	goyx(g.optY+1,x+5);  printf(who(bp, POS_TC));
	goyx(g.optY+1,x+9);  printf(who(bp, POS_TR));

	goyx(g.optY+3,x+1);  printf(who(bp, POS_ML));
	goyx(g.optY+3,x+5);  printf(who(bp, POS_MC));
	goyx(g.optY+3,x+9);  printf(who(bp, POS_MR));

	goyx(g.optY+5,x+1);  printf(who(bp, POS_BL));
	goyx(g.optY+5,x+5);  printf(who(bp, POS_BC));
	goyx(g.optY+5,x+9);  printf(who(bp, POS_BR));

	ink(NORM);

	fflush(stdout);
}

//+============================================================================ =======================================
// Draw the BIG grid
//
// This comment MAY stop `grep` from working! (cos unicode)
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
#	define  U  "\u2580"  // Up (top half)
#	define  D  "\u2584"  // Down
#	define  L  "\u258C"  // Left
#	define  R  "\u2590"  // Right
#	define  F  "\u2588"  // Full
#	define  N  " "       // None

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

static  int  iconClr[2][5] = {
	{ BRED, RED, RED, DRED, DGRY },  // O colours
	{ BCYN, CYN, CYN, DCYN, DGRY },  // X colours
};

#	undef  U
#	undef  D
#	undef  L
#	undef  R
#	undef  F
#	undef  N

//+============================================================================
// Draw BIG piece
//
static
void  _piece (int y,  int x,  board_s* bp,  int pos)
{
	int                 occ = (pos == -1) ? 0 : occupier(bp, pos%10) ;    // original board occupier
	const char* const*  ico = empty;                // icon pointer

	if (occ) {
		occ = (occ ^ (g.par *3)) -1;    // adjust occupier against game parity (for looping games)

		if (pos >= 10) {          // shadow bodge
			ink(iconClr[occ][4]);
			ico = icon[occ][1];

		} else if (( g.bot[g.botID].fn && !g.hint) ||
		           (!g.bot[g.botID].fn &&  g.hide) || (g.loop == 9)) {  // All look same
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
// Draw BIG board
//
void  oxoBig (board_s* bp)
{
	_grid(g.oxoY, g.oxoX);

	_piece(g.oxoY+ 0, g.oxoX+ 2, bp, POS_TL);
	_piece(g.oxoY+ 0, g.oxoX+12, bp, POS_TC);
	_piece(g.oxoY+ 0, g.oxoX+22, bp, POS_TR);

	_piece(g.oxoY+ 6, g.oxoX+ 2, bp, POS_ML);
	_piece(g.oxoY+ 6, g.oxoX+12, bp, POS_MC);
	_piece(g.oxoY+ 6, g.oxoX+22, bp, POS_MR);

	_piece(g.oxoY+12, g.oxoX+ 2, bp, POS_BL);
	_piece(g.oxoY+12, g.oxoX+12, bp, POS_BC);
	_piece(g.oxoY+12, g.oxoX+22, bp, POS_BR);

	fflush(stdout);
}

//+============================================================================
// Are mouse coordinates over a grid position?
//
int  oxoChk (void)
{
	if        (INRANGE(g.my, g.oxoY+ 0, g.oxoY+ 4)) {                   // top
		if      (INRANGE(g.mx, g.oxoX+ 0, g.oxoX+ 8))  return POS_TL ;  //   left
		else if (INRANGE(g.mx, g.oxoX+10, g.oxoX+18))  return POS_TC ;  //   centre
		else if (INRANGE(g.mx, g.oxoX+20, g.oxoX+28))  return POS_TR ;  //   right

	} else if (INRANGE(g.my, g.oxoY+ 6, g.oxoY+10)) {                   // middle
		if      (INRANGE(g.mx, g.oxoX+ 0, g.oxoX+ 8))  return POS_ML ;  //   left
		else if (INRANGE(g.mx, g.oxoX+10, g.oxoX+18))  return POS_MC ;  //   centre
		else if (INRANGE(g.mx, g.oxoX+20, g.oxoX+28))  return POS_MR ;  //   right

	} else if (INRANGE(g.my, g.oxoY+12, g.oxoY+16)) {                   // bottom
		if      (INRANGE(g.mx, g.oxoX+ 0, g.oxoX+ 8))  return POS_BL ;  //   left
		else if (INRANGE(g.mx, g.oxoX+10, g.oxoX+18))  return POS_BC ;  //   centre
		else if (INRANGE(g.mx, g.oxoX+20, g.oxoX+28))  return POS_BR ;  //   right
	}
	return -1;
}

//+============================================================================ =======================================
// Show Bot Menu
//
#define  BOTY(i)  (g.botY +(((i)-1)*2))

void  botShow (void)
{
	for (int i = 1;  i < BOT_CNT;  i++) {
		goyx(BOTY(i), g.botX);
		ink(BYEL);
		if (i == g.botID) {
			paper(ONRED);
			printf(" > %s < ", g.bot[i].name);
		} else {
			paper(ONBLK);
			printf("   %s   ", g.bot[i].name);
		}
	}
	fflush(stdout);
}

//+============================================================================
// Are mouse coordinates over a bot name
//
bot_e  botChk (void)
{
	if (!INRANGE(g.mx, g.botX, g.botX+10))  return BOT_NONE ;

	int i;
	for (i = BOT_CNT -1;  i > 0 && (g.my != BOTY(i));  i--) ;
	return i;
}

//----------------------------------------------------------------------------- ---------------------------------------
static  const char const  menuStr1[] = "[UNDO]  [REDO]  [ANAL]";
                                   //   0123456789012345678901234567890
                                   //             1         2         3
                                   //   0123456789012345678901234567890
static  const char const  menuStr2[] = "   [RESTART]  [QUIT]";

//+============================================================================
// Show Main Menu
// ANAL is for non-Bot play - and shows full game analysis
// HINT is for     Bot play - and, in looping games, shows pieces fading
//
void  menuShow (void)
{
	ink(BYEL);
	goyx(g.mnuY  , g.mnuX);  printf(menuStr1);
	goyx(g.mnuY+2, g.mnuX);  printf(menuStr2);

	// analysis NOT hidden
	if (!g.hide) {
		paper(ONRED);
		goyx(g.mnuY, g.mnuX+16);  printf("[ANAL]");

	// Bot is active
	} else if (g.bot[g.botID].fn) {
		ink(DGRY);
		goyx(g.mnuY, g.mnuX);  printf(menuStr1);
		ink(BYEL);

		// show HINT button
		if (g.hint)  paper(ONRED) ;
		goyx(g.mnuY, g.mnuX+16);  printf("[HINT]");
	}

	fflush(stdout);
}

//+============================================================================
// erase main menu [never used]
//
void  menuClear (void)
{
	ink(NORM);
	goyx(g.mnuY  , g.mnuX);  printf("%*s", sizeof(menuStr1)-1, "");
	goyx(g.mnuY+2, g.mnuX);  printf("%*s", sizeof(menuStr2)-1, "");
	fflush(stdout);
}

//+============================================================================
// is mouse over a main menu option
//
mnuOpt_e  menuChk (void)
{
	if        (g.my == g.mnuY) {
		if (INRANGE(g.mx, g.mnuX+ 0, g.mnuX+ 5))  return MNU_UNDO  ;
		if (INRANGE(g.mx, g.mnuX+ 8, g.mnuX+13))  return MNU_REDO  ;
		if (INRANGE(g.mx, g.mnuX+16, g.mnuX+21))  return MNU_ANAL  ;
	} else if (g.my == g.mnuY+2) {
		if (INRANGE(g.mx, g.mnuX+ 3, g.mnuX+11))  return MNU_AGAIN ;
		if (INRANGE(g.mx, g.mnuX+14, g.mnuX+19))  return MNU_QUIT  ;
	}
	return MNU_NONE;
}

//+============================================================================ =======================================
// show Game Mode menu {6..9}
//
void  modeShow (void)
{
	paper(ONBLK);
	goyx(g.modeY, g.modeX);
	for (int i = 5;  i <= 9;  i++) {
		ink(BYEL);
		if (g.loop == i)  paper(ONRED) ;
		else if (g.bot[g.botID].loop != -1)  ink(DGRY) ;
		printf("[%d]", i);
		paper(ONBLK);
		printf("  ");
	}

	fflush(stdout);
}

//+============================================================================
// is mouse over a mode option
//
int  modeChk (void)
{
	if (g.modeY == -1)  return 0 ;

	if (g.my == g.modeY)
		for (int i = 5-5;  i <= 9-5;  i++)
			if ( INRANGE(g.mx, g.modeX+(i*5), g.mnuX+(i*5)+3) )  {
				modeShow();
				return i+5 ;
			}
	return 0;
}

//+============================================================================
// erase mode menu [never used]
//
void  modeClear (void)
{
	ink(NORM);
	goyx(g.modeY, g.modeX);
	printf("%*s", (5*5)-2, "");
	fflush(stdout);
}

//+============================================================================ =======================================
// Show sequence .. moves that are available to 'redo' are in dark grey
//
void  seqShow (int cnt)
{
	ink(NORM);
	MSGFYX(g.seqY,1, "\e[K");  // clear old sequence
	goyx(g.seqY,1);

	int  i = 0;
	ink(BWHT);  while (i < g.move)  printf("%d, ", g.play[i++].in) ;
	ink(DGRY);  while (i < g.last)  printf("%d, ", g.play[i++].in) ;
}

//+============================================================================
// erase sequnce line
//
void  seqClear (void)
{
	MSGFYX(g.seqY,1, "\e[K");
}

//+============================================================================ =======================================
// draw ALL children (even greyed out moves)
//
void  optShow (board_s* bp)
{
	int  cidx = 0;
	while (cidx < bp->cCnt)  oxo(cidx++, bp->chld[cidx], OPTX(cidx)) ;
	while (cidx < 9       )  oxo(cidx++, &g.b[0],        OPTX(cidx)) ;
}

//+============================================================================
// is the mouse over an option ?
//
int   optChk (int* in)
{
	int  h = g.hide ? 5 : (g.loop ==9) ? 8 : 18 ;

	// selecting an option?
	if (INRANGE(g.my, g.optY, g.optY +h)) {  // y coord for options
		int x = (g.mx -1) / g.optW;          // selection
		if (INRANGE(x, 0, 8)) {              // 0..8
			if ( INRANGE(g.mx, OPTX(x), OPTX(x+1)-1) )
				return (*in = x + '0');      // fake like we just pressed the number
		}
	}
	return -1;
}

//+============================================================================ =======================================
// I got annoyed at ther beign no correlation between the BIG oxo and the analysis
// so now, whichever one you highlight, the other is also highlighted
// the stats get a box, the BIG oxo gets a shadow piece
//
void  shadow (board_s* bp,  int opt,  int pos)
{
	static  int  pos_ = -1;

	if (pos == pos_)  return ;

	switch (pos_) {
		case POS_TL :  _piece(g.oxoY+ 0, g.oxoX+ 2, bp, -1);  break ;
		case POS_TC :  _piece(g.oxoY+ 0, g.oxoX+12, bp, -1);  break ;
		case POS_TR :  _piece(g.oxoY+ 0, g.oxoX+22, bp, -1);  break ;

		case POS_ML :  _piece(g.oxoY+ 6, g.oxoX+ 2, bp, -1);  break ;
		case POS_MC :  _piece(g.oxoY+ 6, g.oxoX+12, bp, -1);  break ;
		case POS_MR :  _piece(g.oxoY+ 6, g.oxoX+22, bp, -1);  break ;

		case POS_BL :  _piece(g.oxoY+12, g.oxoX+ 2, bp, -1);  break ;
		case POS_BC :  _piece(g.oxoY+12, g.oxoX+12, bp, -1);  break ;
		case POS_BR :  _piece(g.oxoY+12, g.oxoX+22, bp, -1);  break ;

		default:
		case -1     : break ;
	}

	pos_ = pos;
	if (opt < 0)  return ;

	switch (pos) {
		case POS_TL :  _piece(g.oxoY+ 0, g.oxoX+ 2, bp->chld[opt], pos+10);  break ;
		case POS_TC :  _piece(g.oxoY+ 0, g.oxoX+12, bp->chld[opt], pos+10);  break ;
		case POS_TR :  _piece(g.oxoY+ 0, g.oxoX+22, bp->chld[opt], pos+10);  break ;

		case POS_ML :  _piece(g.oxoY+ 6, g.oxoX+ 2, bp->chld[opt], pos+10);  break ;
		case POS_MC :  _piece(g.oxoY+ 6, g.oxoX+12, bp->chld[opt], pos+10);  break ;
		case POS_MR :  _piece(g.oxoY+ 6, g.oxoX+22, bp->chld[opt], pos+10);  break ;

		case POS_BL :  _piece(g.oxoY+12, g.oxoX+ 2, bp->chld[opt], pos+10);  break ;
		case POS_BC :  _piece(g.oxoY+12, g.oxoX+12, bp->chld[opt], pos+10);  break ;
		case POS_BR :  _piece(g.oxoY+12, g.oxoX+22, bp->chld[opt], pos+10);  break ;

		default:
		case -1     : break ;
	}
}

//+============================================================================
// Draw a box
//
#define  TL  "\u250C"   // box characters (single line)
#define  TR  "\u2510"
#define  BL  "\u2514"
#define  BR  "\u2518"
#define  H   "\u2500"
#define  V   "\u2502"
#define  D   "\b\e[1B"

void box_ (int opt,  char* tl, char* tr, char* bl, char* br, char* h, char* v)
{
	int  hh = 5 + g.analH;

	goyx(g.optY-1, OPTX(opt)-1);
	printf(tl);
	for (int i = 1;  i < g.optW-1;  i++)  printf(h);
	printf("%s%s", tr, D);

	for (int i = 0;  i <= hh;  i++)  printf("%s%s", v, D);
	goyx(g.optY, (opt*g.optW)+1);
	for (int i = 0;  i <= hh;  i++)  printf("%s%s", v, D);

	printf(bl);
	for (int i = 1;  i < g.optW-1;  i++)  printf(h);
	printf(br);
}

//+============================================================================
// (un)draw a box
//
void  box (int opt)
{
	static  int opt_ = -1;

	if (opt == opt_)  return ;
	ink(LGRY);
	if (opt_ >= 0)  box_(opt_, " ", " ", " ", " ", " ", " ") ;
	if (opt  >= 0)  box_(opt, TL, TR, BL, BR, H, V) ;

	opt_ = opt;
}

#undef  TL
#undef  TR
#undef  BL
#undef  BR
#undef  H
#undef  V
#undef  D

//+============================================================================
// this is totally overkill for a O's & X's game - LOL !
//
// if you're pointing to an option, draw a shadow piece
// if you're pointing to an empty grid location, highlight the option
// NULL = UNdraw both
//
void  overkill (board_s* bp)
{
	int  in;
	int  opt = -1;
	int  pos = -1;

	// stop the cursor flickering
	curoff();

	// undraw everything
	if (!bp) {
		box(-1);
		shadow(NULL, 0, -1);
		goto done;
	}

	// option highlighted
	if ((opt = optChk(&in)) >= 0) {
		// which grid position is it?
		opt -= '0';
		if (g.pref[opt].ink != C_INVALID)  pos = bp->chld[opt]->seq &0xF ;

	// grid location highlighted
	} else if ((pos = oxoChk()) >= 0) {
		// which option is it?
		for (opt = 8;  opt >= 0;  opt--)
			if (g.pref[opt].ink == C_INVALID)           continue ;
			else if (pos == (bp->chld[opt]->seq &0xF))  break ;
		if (opt == -1)  pos = -1 ;
	}

	// draw them
	shadow(bp, opt, pos);
	box(opt);

done:
	// reset the mouse pointer
	goyx(g.my, g.mx);
	curon();

	return;
}
