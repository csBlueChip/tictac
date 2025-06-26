#include  <string.h>

#include  "tictac.h"
#include  "logic.h"

//----------------------------------------------------------------------------- ---------------------------------------
// Winning lines
//

// search order
#define  LIN_DB  (0)         // Diag tl \ br  (Backslash)
#define  LIN_DF  (1)         //      tr / bl  (Forwardslash)

#define  LIN_C   (2)         //          Centre
#define  LIN_M   (3)         //          Middle

#define  LIN_T   (4)         // horz row Top
#define  LIN_B   (5)         //          Bottom
#define  LIN_L   (6)         // vert col Left
#define  LIN_R   (7)         //          Right

#define  WIN_CNT (8)         // 8 ways to win

static  const int  winl[WIN_CNT][4] = {
	[LIN_L]  = ((int[4]){LIN_L,  POS_TL, POS_ML, POS_BL}),  // vert col Left
	[LIN_C]  = ((int[4]){LIN_C,  POS_TC, POS_MC, POS_BC}),  //          Centre
	[LIN_R]  = ((int[4]){LIN_R,  POS_TR, POS_MR, POS_BR}),  //          Right

	[LIN_T]  = ((int[4]){LIN_T,  POS_TL, POS_TC, POS_TR}),  // horz row Top
	[LIN_M]  = ((int[4]){LIN_M,  POS_ML, POS_MC, POS_MR}),  //          Middle
	[LIN_B]  = ((int[4]){LIN_B,  POS_BL, POS_BC, POS_BR}),  //          Bottom

	[LIN_DB] = ((int[4]){LIN_DB, POS_TL, POS_MC, POS_BR}),  // Diag tl \ br  (Backslash)
	[LIN_DF] = ((int[4]){LIN_DF, POS_TR, POS_MC, POS_BL}),  //      tr / bl  (Forwardslash)
};

//+============================================================================ =======================================
// Check if this board has a winning line on it - return the winner ID {0:none, 1:A, 2:B}
// ...If it has, recusrively ripple the win count back up the tree
//
// This is only used during board generation ... DO NOT call `win()` again on a board
// During game play check `bp->win`
//
static
void  ripple_(board_s* bp,  uint8_t pl)
{
	bp->wins[pl]++;
	if (bp->prnt)  ripple_(bp->prnt, pl) ;
	return;
}

//+============================================================================
static inline
int  _win (board_s* bp,  const int* pos)
{
	// pos[0] is the winID - not used here

	int  p1, p2, p3;

	// if any indicated position is empty - we're done
	if ( !(p1 = occupier(bp, pos[1])) ||
	     !(p2 = occupier(bp, pos[2])) ||
	     !(p3 = occupier(bp, pos[3]))    )  return 0 ;

	// if they are not all the same player - we're done
	if ((p1 != p2) || (p1 != p3))  return 0 ;

	return p1;  // occupier of "p1" wins
}

//+============================================================================
static inline
int  win (board_s* bp)
{
	for (int i = 0;  i < WIN_CNT;  i++)
		if ( (bp->win = _win(bp, winl[i])) ) {
			bp->lin = winl[i][0];  // id of winning line
			ripple_(bp->prnt, bp->win -1);
			return  bp->win;
		}
	return 0;
}

//+============================================================================ =======================================
// Build every board as far as "max" moves
//
static
void  build_ (board_s* prnt)
{
	if (prnt->cnt == g.max)  return ;

	// play next piece
	for (int pos = 0;  pos < 9;  pos++) {
		if (occupier(prnt, pos))  continue ;  // avoid occupied squares

		board_s*  bp = &g.b[++g.bn];          // start a new board

		// link this board to it's parent
		bp->prnt                 = prnt;
		prnt->chld[prnt->cCnt++] = bp;

		// grab a copy of the parent state
		bp->cnt = prnt->cnt;
		bp->occ = prnt->occ;
		bp->seq = prnt->seq;

		// place next piece
		bp->occ |= (((++bp->cnt) & 1) ? 0x1 : 0x2) << (pos *2);
		bp->seq  = (bp->seq << 4) | pos;
/*
		// We do not need to track any state after a win
		int  w = isWin(bp);
		if (w >= 0) {
			bp->lin           = winl[i][0];  // store id of winning line
			bp->prnt->lose[0] = 1;           // parent is (must be) a loser
			ripple_(bp->prnt, bp->win -1);   // recursively ripple the win up to the root
			continue ;  //!  ((comment out this line to build EVERY board))
		}
*/
		if (win(bp)) {
			bp->prnt->lose[0] = 1;
			continue ;  //!  ((comment out this line to build EVERY board))
		}

		build_(bp);  // recurse
	}

	return;
}

//+============================================================================
// Max is the max number of pieces allowed on the board
// at the point in time the next player is considering their move
// 9 = find EVERY winning combo (there are 9 pieces on the board when considering if anyone won)
// 6 = SIX  pieces may be visible when considering a move [ https://www.amazon.co.uk/dp/B0DNLYSN7J ]
// 5 = FIVE pieces may be visible when considering a move [ https://xogone.com/ ]
//     ...at ths time loop5() has not been written!
//
int  build (int max)
{
	memset(&g.b, 0, sizeof(g.b));
	g.max = max;
	build_(&g.b[(g.bn = 0)]);
	return ++g.bn;
}


//+============================================================================ =======================================
// find loop points
//
int  loopat (int loop)
{
	seq_t  seek = 0;
	seq_t  mask = ((seq_t)0x1 <<(loop*4)) -1;  // mask for 'loop' moves

	// read the comments in `struct board_s`
	if ((loop <= 4) || (loop >= 9))  return -1 ;  //exit(101) ;

	(void)findseq(0, 0);  // reset search

	for (int i = 0;  i < g.bn;  i++) {
		board_s*  bp = &g.b[i];

		if (bp->cnt != loop)  continue ;  // Only process move-N boards
		if (bp->win)          continue ;  // winners do not have children

		//  6666'5555'4444'3333'2222'1111  ->  5555'4444'3333'2222'1111'xxxx
		//       5555'4444'3333'2222'1111  ->       4444'3333'2222'1111'xxxx
		seek = (bp->seq <<4) & mask;

		// find the three matching 6-move boards
		for (int pos = 0;  pos < 9;  pos++) {
			if (occupier(bp, pos))  continue ;               // ignore 'loop' out of 9 moves
			bp->chld[bp->cCnt] = findseq(seek | pos, loop);  // find the loop board
			if (bp->chld[bp->cCnt]->win)  bp->lose[1] = 1 ;  // winning children, have losing parents
			bp->cCnt++;                                      // parent has looping child
		}
	}
}
