#ifndef  TICTAC_H_
#define  TICTAC_H_

#include  <stdint.h>

#include  "bot.h"

//----------------------------------------------------------------------------- ---------------------------------------
// board positions
// various iterations of the code have used different layouts (eg. to simplify rotation)
// at this point, I think the code doesn't care any more - but, if you edit this, BE CAREFUL !
//
#define  POS_TL  (0)   //
#define  POS_TC  (1)   //  left cntr right        grid is divided in to 9 positions
#define  POS_TR  (2)   //
                       //
#define  POS_ML  (7)   //    0 | 1 | 2    top
#define  POS_MC  (8)   //   ---+---+---           diagonal backslash    (tl,br)
#define  POS_MR  (3)   //    7 | 8 | 3    middle
                       //   ---+---+---           diagonal forwardslash (tr,bl)
#define  POS_BL  (6)   //    6 | 5 | 4    bottom
#define  POS_BC  (5)   //
#define  POS_BR  (4)   //

//----------------------------------------------------------------------------- ---------------------------------------
// A single board
//
// Every time you loop from a move-N board to another move-N board, the parity of the game inverts
// At the beginning of the game player-A is player-1 (O's),
//                          and player-B is player-2 (X's)
// When the parity inverts, A and B swap players
// The first time the parity inversion happens is when player-1 lays their (N/2+1)th piece (ie. piece N+1)
// ...from thereonin it happens EVERY turn
//
// We can say:  
//   if      (move < 7)  A=1/O, B=2/X
//   else if (move & 1)  A=2/X, B=1/O
//   else                A=1/O, B=2/X
//
//   if (!(move&1) || (move<7))  A=1, B=2;  else  A=2, B=1;
//            after move 6         even             odd

// You only need uint64 if you resolve every possible game (ie. max_moves > 8, ie. max_moves == 9)
// If you are looping before move-9, you only need a unit32_t
typedef  uint64_t  seq_t ;

typedef
	struct board {
		// occupier() : return ((occ >> (pos*2)) & 3)
		uint32_t       occ;      // grid bitpair mask {00=empty, 01=plyA, 10=plyB, 11=resvd}

		// addmove(position) : seq = (seq <<4) | position
		seq_t          seq;      // sequence (nybbles) 9*4

		uint8_t        cnt;      // number of occupied squares/moves {0..9}

		uint8_t        win;      // winning board? (0=no, 1=plyA, 2=plyB)
		uint8_t        lin;      // winning combo ID (qv. `winl[0..7]`)
		uint32_t       wins[2];  // number of descendants who win for {0:plyA, 1:plyB}

		int            lose[2];  // has a child that wins - [0]=9move, [1]=loopN

		// When the loops are put in place, this will remain as the "original" parent
		// There will now also be {1, 2, or 3} loop points which lead to here
		// NOT "always 3" ...because some loop points will have been "win" states
		//                ...winners, don't loop - they declare "game over"
		struct board*  prnt;     // 1 parent


		// A board in a loop-6 game will have 3+3 children
		// The 3 possible moves that would progress a normal game (-> 7-piece board)
		// ...and the 3 that progress to another 6-piece board
		// The first 3 (normal games moves) are always in the array in positions [0,1,2]
		// ...If `loopat(6)` was run, these will be followed by ANOTHER 3 moves
		//    which lead to the "loop-6" board
		//
		// The same can be said for loop-5 games, but with 4+4 children
		//
		// I can't think of anything right now that would stop you from running `loopat(7 or 8)`
		// ...could be interesting to see whather they're fun games or not!?
		// `loopat(4)` (or lower) simply makes no sense as it would be impossible for anyone to win!
		struct board*  chld[9];  // max 9 childen (start with empty board)
		int            cCnt;     // how many children (both standard AND loop children)
	}
board_s;

// every possible way to lay <=5 O's and <=4 X's (9 pieces) on a 3x3 grid
#define  BMAX      (986410)  // WHAT is the maths that generates this number?? // sum/product of factorials??

//----------------------------------------------------------------------------- ---------------------------------------
//	 X | X | X
//	---+---+---
//	 X | X | X
//	---+---+---
//	 X | X | X
//
//
// each board will gain some stats when we consider its strength
//
// "amb" (a minus b) is the difference of how many win states exist in the FULL (9-move) game, from this point
// ie. For any given move, there are 'A' pathways where player-A wins, and 'B' pathways where player-B wins
// the idea is to keep this balance in your favour
//
// "ink" is simply the colour which the board will be painted when rendered
//
typedef
	struct pref {
		int  ink;
		int  amb;  // win[a] - win[b]
		int  laWin[2];
	}
pref_t;

//----------------------------------------------------------------------------- ---------------------------------------
// This'll do for now...
//
#define  MOVE_MAX  36  // Declare a 'draw' after N moves

typedef
	struct  play {
		board_s*  bp;  // board
		int       in;  // option played
	}
play_t;

//----------------------------------------------------------------------------- ---------------------------------------
// Global variables ...Everybody has access to the game-state
//
typedef
	struct {
		board_s  b[BMAX];         // boards
		int      bn;              // number of populated boards (<= BMAX)
		int      max;             // max pieces allowed on a board
		int      par;             // game parity {0->{A=1/O, B=2/X}, 1->{A=2/X, B=1/O}}
		int      loop;            // game mode (loop=9 is a normal/classic game)

		int      unhide;          // hide override (mostly for Bot debug)
		int      hide;            // hide ananlysis
		pref_t   pref[10];        // (up to) 9 possible moves + 1 for the game board

		int      last;            // last move (for "redo")
		int      move;            // move number
		play_t   play[MOVE_MAX];  // undo/redo buffer

		int      mev;             // mouse event
		int      my, mx;          // mouse x,y

		int      oxoY, oxoX;      // big board y, x
		int      modeY, modeX;    // coords of game mode menu {6..9}
		int      mnuY, mnuX;      // coords of menu
		int      plmY, plmX;      // coords of player menu
		int      optY, optW;      // Y coordinate & Width of options
		int      analH;           // Analysis height
		int      seqY;            // Y coord for sequence output (first status line)

		int      botY, botX;      // Bot menu
		bot_e    botID;           // active bot
		bot_s    bot[BOT_CNT];    // bot details
		int      hint;            // hint mode active [qv. menu]

		int      botT;            // 0='O', 1='X'
	}
global_s;

extern  global_s  g;

//+============================================================================ =======================================
// Who occupies the specified position on the specified board
//   {0=nobody, 1=player-A, 2=Player-B}
//
static inline  int  occupier (board_s* bp,  int pos)  {  return (bp->occ >> (pos *2)) & 0x3;  }

#endif //TICTAC_H_
