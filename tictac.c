#include  <stdint.h>
#include  <stdbool.h>
#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <unistd.h>  // usleep

#include  "macro.h"
#include  "ansi.h"
#include  "conio.h"

#include  "tictac.h"
#include  "gfx.h"
#include  "build.h"
#include  "logic.h"
#include  "anal.h"
#include  "version.h"
#include  "bot.h"

#define  NDEBUG
#include  "debug.h"

#include  <time.h>
#include  <stdlib.h>

//----------------------------------------------------------------------------- ---------------------------------------
// global variables - extern'ed
//
global_s  g;

//+============================================================================ =======================================
int  tictac (void)
{
	int       cidx = 0;        // child index (used by loop-pairs)
	board_s*  bp   = &g.b[0];  // board #0 is the empty board

	// In a normal game, the 6th move is ALWAYS player by player-2 ("X")
	// When a board loops (eg. move-6 -> move-6), we need to invert the roles of "player-1" and "player-2"
	// ie. if parity == 0, then when this board refers to a player as "player-1", it is referring to: "O"
	//     if parity == 1,  "    "    "     "      "   "  "    "   "  "player-1", "  "      "     " : "X"
	//
	// Similar is true for all looping games
	g.par = 0;

	// 'play' acts as the undo stack
	g.play[(g.move = 0, g.last = 0)].bp = bp;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// GAME ON
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// === let's get this starty parted... ===
	int in;
	for (g.move++;  ;  g.move++) {

		oxoBig(bp);  // Draw the main board

		// which children are in play ?   (standard, or loop)
		// {0..cnt-1} or {cnt..cCnt-1} ... the -1 is affected with `< nd` in the loops
		int  st = (bp->cnt == g.loop) ? (9 - bp->cnt) : 0 ;
		int  nd = (bp->cnt != g.loop) ? (9 - bp->cnt) : bp->cCnt ;

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Opinion time...
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		in  = -1;  // input / option choice

		// no preferred move
		memset(g.pref, 0, sizeof(g.pref));
		int  cidx = 0;
		for (  ;  cidx < st;  g.pref[cidx++].ink = C_INVALID ) ;
		for (  ;  cidx < nd;  g.pref[cidx++].ink = bp->win ? C_INVALID : C_FAIR) ;
		for (  ;  cidx < 9 ;  g.pref[cidx++].ink = C_INVALID ) ;
		g.pref[cidx].ink = C_GAME;  // {9} is the game board  (unused, for now)

		if (!bp->win) {  // Winners don't have children
			analyse(bp, st, nd);
			if (!(g.move & 1) && g.bot[g.botID].fn)  (void)g.bot[g.botID].fn(&in, st, nd) ;
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// User/Bot Input handling
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		// when the current board contains `loop` pieces, we need to flip the parity
		// the first time this will happen will be AFTER move #loop (eg. move #6) has been played
		g.par ^= (bp->cnt == g.loop);

		// draw ALL children (even greyed out moves)
		if (in == -1)  optShow(bp) ;

		// input move
		for (;;) {
			int  over = 0;  // game over status

			//----------------------------------------------
			// Evaluate game state and draw prompt
			//----------------------------------------------
			const char* const  icon1 = (g.move & 1) ? plo[0] : plx[0] ;  // this player
			const char* const  icon2 = (g.move & 1) ? plx[0] : plo[0] ;  // other player
			goyx(g.yy+2,0);
			ink(NORM);
			if      (bp->win)                { MSGF("Player %s WINS!\e[K", icon2);                       over = 1; }
			else if (bp->cnt == 9)           { MSGF("It's a draw!\e[K");                                 over = 1; }
			else if (g.move +1 >= MOVE_MAX)  { MSGF("%d moves made - I declare a draw!\e[K", MOVE_MAX);  over = 1; }
			else                             { MSGF("Move %d: %s >\e[K", g.move, icon1);                           }

			//----------------------------------------------
			// Get user input
			//----------------------------------------------
			goyx(g.my, g.mx);  // position mouse
			if (in == -1) {
				do {
					while (!kbhit())  usleep(1000) ;   // wait for a key (1mS == 1'000uS)
					in = getchw();                     // get the (wide-)key
				} while ((in == ',') || (in == ' ') || (in == '\r'));  // allow paste of previous game
			}
			//----------------------------------------------
			// Mouse input will simulate keystrokes
			//----------------------------------------------
			if (MOUSE_ISRPT(in)) {
				int  pos;

				// decode the event
				g.mev = MOUSE_EVENT(in);
				g.my  = MOUSE_Y(in);
				g.mx  = MOUSE_X(in);
//				MSGFYX(g.yy+4,0, "Mouse: %3d @ %3d,%-3d", g.mev, g.my, g.mx);

				if (g.mev == MEV_POS) {
					if (!bp->win)  overkill(bp) ;

				} else if ((g.mev == MEV_BTN_L) && MOUSE_ISDOWN(in)) {
					// game mode {5..9}
					int  m = modeChk();
					if (m) {
						if (g.bot[g.botID].loop == -1) {
							in     = KEY_CTRL_R;
							g.loop = m;
							modeShow(13, 5);  //!
						}

					// bots
					} else if ((m = botChk()) != BOT_NONE) {
						// in/out of pvp - restart game
						if        ((m == BOT_PVP) && (g.botID != BOT_PVP)) {
							in = KEY_CTRL_R;

						} else if ((m != BOT_PVP) && (g.botID == BOT_PVP)) {
							in = KEY_CTRL_R;
						}
						botSet(m);
						menuShow(16, 5);  //!
						modeShow(13, 5);  //!
						botShow();

					// option {0..8}
					} else if (optChk(&in) >= 0) {
						(void)NULL;

					// option {0..8}
					} else if ((pos = oxoChk()) >= 0) {
						for (in = 8;  in >= 0;  in--)
							if (g.pref[in].ink == C_INVALID)           continue ;
							else if (pos == (bp->chld[in]->seq &0xF))  break ;
						in += '0';

					// menu options
					} else {
						switch (menuChk(g.my, g.mx)) {
							case MNU_UNDO  :  in = KEY_LEFT;    break ;
							case MNU_REDO  :  in = KEY_RIGHT;   break ;
							case MNU_ANAL  :  in = g.bot[g.botID].fn ? KEY_CTRL_H : KEY_CTRL_A;  break ;
							case MNU_AGAIN :  in = KEY_CTRL_R;  break ;
							case MNU_QUIT  :  in = KEY_CTRL_C;  break ;
							default :  break ;
						}
					}

				} else if ((g.mev == MEV_BTN_R) && MOUSE_ISDOWN(in)) {
					in = KEY_LEFT ;
				}
			}

			//----------------------------------------------
			// Special keys
			//----------------------------------------------
			if        (in == '`') {                             // ` (bactick) -> '0'
				in = '0';

			} else if (in == KEY_CTRL_H) {                      // ^H hint show/hide
				if (g.bot[g.botID].fn) {
					g.hint ^= 1;
					menuShow(16, 5);  //!
				}

			} else if (in == KEY_CTRL_A) {                      // ^A analysis show/hide
				if (!g.bot[g.botID].fn) {
					g.hide ^= 1;
					optShow(bp);
					menuShow(16, 5);  //!

					// we draw the master board BEFORE we flipped the parity!
					g.par ^= (bp->cnt == g.loop);
					oxoBig(bp);  // Draw the main board
					g.par ^= (bp->cnt == g.loop);

//					continue;
				}

			} else if (in == KEY_CTRL_C) {                      // ^C quit
				ink(NORM);
				MSGFYX(g.yy+3,0, "Quit\e[K");
				return 0;

			} else if (in == KEY_CTRL_R) {                      // ^R new game
				seqClear();
				return 1;    // trigger reentry to tictac()

			} else if (in == KEY_RIGHT) {                       // -> redo
				if (g.last >= g.move) {
					in = g.play[g.move-1].in;
					goto redo;  // oh no, a scary goto! Quick: Call the 'goto' police!
				}

			} else if ( ((in == KEY_BKSP) || (in == KEY_LEFT))  // <-- undo : ASCII(DEL) [not ASCII(BS)]
			            && (g.move > 1) && (g.botID == BOT_PVP)) {
				// we want to end up on move-1, but the loop iterator does move++
				g.move -= 2;

				g.par  ^= (g.move+1 == g.loop);

				bp      = g.play[g.move].bp;
				seqShow(g.yy, g.move);

				goto continue2;
			}

			//----------------------------------------------
			if (!over) {
				// valid move?
				in -= '0';                             // convert digit to value
				if ((in >= st) && (in < nd))  break ;  // valid move?
				if ((in >= 0 ) && (in <= 8))
					MSGFYX(g.yy+3,0, "Bad move: %d\e[K", in);
			}

			in = -1;
		}

		// make the move!
		g.last              = g.move;        // update end of the redo-buffer
redo:
		g.play[g.move-1].in = in;            // mainboard option selected
		g.play[g.move].bp   = bp->chld[in];  // this selection (about to become mainboard)
		bp                  = bp->chld[in];  // board in play

		// Show move sequence
		seqShow(g.yy, g.move);

		overkill(NULL);

continue2:
		MSGFYX(g.yy+3,0, "\e[K");  // clear status line
	}//never exits

}

//+============================================================================ =======================================
void  cleanup (void)
{
	mouse(MRPT_NONE) ;
	(void)termRestore(NULL);
	goyx(g.yy+5,1);
}

//++=========================================================================== =======================================
int main (int argc,  char* argv[])
{
	cls();

	srand(time(NULL));

	printf("# %s %s .. Copyright %s, %s\n", TOOLNAME, VER_STR, AUTHOR, DATE);
//	printf("# Use: %s [5|6|7|8|9] [-a]\n", argv[0]);

	printf("# Game Keys: ");
	ink(BYEL);
	printf("<-:Undo, ->:Redo, ^A:Analysis, ^R:Restart, ^C:Quit\n");
	ink(NORM);

	printf("# Analysis Key: ");
	ink(C_INVALID);  printf("invalid, ");
	ink(C_LOSE   );  printf("lose, ");
	ink(C_WEAK   );  printf("weak, ");
	ink(C_FAIR   );  printf("fair, ");
	ink(C_STRONG );  printf("strong, ");
	ink(C_WIN    );  printf("win");
	ink(NORM);       printf("\n");

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// initialise global variables
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	memset(&g, 0, sizeof(g));

	g.yOpt = 26;  // y coord for options
	g.yy   = 48;  // coord for sequence (other stuff below it)

	g.oxoY = 7;   // main board
	g.oxoX = 38;  // ...

	g.botY = 13;  // bot menu
	g.botX = 77;  // ...
	botSetup();

	// game style :
	//   5 = the 'online' version
	//   6 = the 'electronic game' version
	//   7 = unknown
	//   8 = unknown
	//   9 = normal game
	g.loop = 9;  // assume normal game

	g.hide = 1;  // hide analysis & disable hints
	g.hint = 0;  // enable hints in bot mode

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// build every possible game
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	printf("# Find complete solution set...");
	(void)build(9);
//	printf("%d possible game sequences\n", g.bn);

	// work out all the loop points
	for (int i = 5;  i <= 8;  i++) {
		printf("# Resolve loop points @%d ... \n", i);
		loopat(i);
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// parse CLI (badly)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	for (int i = 1;  i < argc;  i++) {
		if (strcmp(argv[i], "-a") == 0)  g.hide = 0 ;
		else                             g.loop = atoi(argv[i]) ;
	}
	if (!INRANGE(g.loop, 5, 9)) {
		printf("! Unknown game type: %d\n", g.loop);
		exit(99);
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// be helpful
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	printf("# Show analysis: %s\n", g.hide?"No":"Yes");

	modeShow(13, 5);  //!
	menuShow(16, 5);  //!
	botShow();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Configure everything
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// start keyboard driver (kills itself with atexit())
	kbdInit();

	(void)termSave(NULL);        // save window size
	(void)termSet(117, g.yy+3);  // set window size

	(void)mouse(MRPT_ALL);       // enable mouse

	atexit(cleanup);             // cleanup when the program exits

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// run the game
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	while ( tictac() ) ;    // run the game until it returns 0

	return 0;
}
