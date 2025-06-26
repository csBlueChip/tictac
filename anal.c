#include  <string.h>

#include  "macro.h"
#include  "tictac.h"
#include  "gfx.h"

//----------------------------------------------------------------------------- ---------------------------------------
static  int  laWin[2] = {0};  // {0=player-A, 1=player-B}

//+============================================================================ =======================================
// Look ahead a maximum of 'depth' moves and tally all the odd and even wins
//
static
void  lookahead_ (board_s* bp,  int depth,  int par)
{
	if (bp->win) {
		laWin[par]++;
		return;
	}

	if (!depth)  return ;

	int  st  = (bp->cnt == g.loop) ? (9 - bp->cnt) : 0 ;
	int  nd  = (bp->cnt != g.loop) ? (9 - bp->cnt) : bp->cCnt ;

	for (int i = st;  i < nd;  i++)
		lookahead_(bp->chld[i], depth-1, !par);
}

//+============================================================================
// NON-RE-ENTRANT
//   returns a pointer to an array of 2 integers
//   qv. laWin [above]
//
int*  lookahead (board_s* bp,  int depth)
{
	memset(laWin, 0, sizeof(laWin));
	lookahead_(bp, depth, 0);
	return laWin;
}

//+============================================================================ =======================================
// a minus b
// this is of debatable value at the start of a looping game
// ...of little-or-no notable value once it is looping
//
void  simpleMinMax (board_s* bp,  int st,  int nd)
{
	int  ambH = -999999;
	int  ambL =  999999;

	// pass 1: find difference-of-wins (for each child) (in favour of active player)
	// track highest & lowest results
	for (int i = st;  i < nd;  i++) {
		int  ply = (g.move&1)^1;
		g.pref[i].amb = bp->chld[i]->wins[ply] - bp->chld[i]->wins[ply^1];
		if (g.pref[i].amb > ambH)  ambH = g.pref[i].amb ;
		if (g.pref[i].amb < ambL)  ambL = g.pref[i].amb ;
	}

	// negative numbers are just not friendly!
	if (ambL < 0) {
		for (int i = st;  i < nd;  i++)  g.pref[i].amb += (-ambL) ;
		ambH += (-ambL) ;
		ambL  = 0;
	}

	// pass 2: Highlight highest and lowest differences
	for (int i = st;  i < nd;  i++) {
		if      (g.pref[i].amb == ambH)  g.pref[i].ink = C_STRONG ;
		else if (g.pref[i].amb == ambL)  g.pref[i].ink = C_WEAK   ;
	}
}

//+============================================================================ =======================================
// The "AI" (whatever tha means nowdays!?)
// Here it means: The computer uses some maths to have an opinion
//
void  analyse (board_s* bp,  int st,  int nd)
{
//	if ((g.loop == 9) || (g.move < g.loop -1)) {
	if (g.loop == 9) {
		simpleMinMax(bp, st, nd);
		return;
	}

	// Spot obvious wins & loses
	for (int i = st;  i < nd;  i++) {
		if (0) (void*)NULL ;
		else if (bp->chld[i]->win)                             g.pref[i].ink = C_WIN  ;  // Win this move
		else if (bp->chld[i]->lose[(g.move>=g.loop)])          g.pref[i].ink = C_LOSE ;  // Lose after this move
		else if (lookahead(bp->chld[i], 2)[0] == 9 - bp->cnt)  g.pref[i].ink = C_WIN2 ;  // Win next move
	}
}

//+============================================================================ =======================================
void  oxoAnal (int id,  board_s* bp,  int x)
{
	if (bp != g.b) {
		if ((g.loop != 9) && (g.move >= g.loop -1))  ink(LGRY) ; //!!!
		goyx(g.yOpt+7,x);  printf("%d/%d", bp->wins[0], bp->wins[1]);
		goyx(g.yOpt+8,x);  printf(" = %d", g.pref[id].amb);  //bp->wins[0] - bp->wins[1]);

//		goyx(g.yOpt+9,x);  printf("%d/%d", bp->win, bp->lin);

		if (g.loop == 9)  return ;

		if (g.pref[id].ink != C_INVALID) {
			for (int i = 1;  i <= 6;  i++) {
				int yy = g.yOpt +9 +i +(i>=5) +(i>=6);
				goyx(yy, x);
				if (!bp->win) {
					int*  w = lookahead(bp, i);

					if (i==1)      ink(BWHT) ;                          // Immediate-child
					else if (i&1)  ink(((g.move+i)&1) ?  RED :  CYN) ;  // RED=A/O, CYN=B/X
					else           ink(((g.move+i)&1) ? BRED : BCYN) ;  // BRT=my move, DIM=their move

					printf("%d: %d/%d", i, w[0^(i&1)], w[1^(i&1)]);
					if (i >= 4)  MSGFYX(yy+1, x, "==%d", w[0^(i&1)] - w[1^(i&1)]);
				}
			}
		}
	}
}
