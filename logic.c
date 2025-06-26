#include  "stdlib.h"

#include  "tictac.h"
#include  "logic.h"

/*
//+============================================================================
// {-1=No, 0..7=winID}
//
int  isWin (board_s* bp)
{
	for (int i = 0;  i < WIN_CNT;  i++)
		if (_win(bp, winl[i]))  return i ;
	return -1 ;
}
*/

//+============================================================================ =======================================
// Work out how every possible 7th move (from a source 6-move board) loops back to a target 6-move board
//
board_s*  findseq (seq_t seq,  int loop)
{
	static  int  from = 0 ;

	int  start;

	if (!loop)  return (from = 0), NULL ;

	// From looking at earlier debug output,
	// this "pick up where you left off" method happens to be quite efficient.
	// So, in the name of "simpler code", I ditched the memory hogging speed-ups

	for (start = from;  from < g.bn;  from++)
		if ((g.b[from].cnt == loop) && (g.b[from].seq == seq))  return &g.b[from] ;

	// wrap around
	for (from = 0;  from < start;  from++)
		if ((g.b[from].cnt == loop) && (g.b[from].seq == seq))  return &g.b[from] ;

	return NULL;  // not found
}

//+============================================================================ =======================================
// return a string to display as the piece which is at position 'pos' on board 'bp'
// (this (may) include ANSI colour codes)
//
const char* const  plo[5] = {
	[0] = "\e[1;31mO\e[0m",     // bright O (ucs) [newest]
	[1] = "\e[0;31mO\e[0m",     //        O (ucs)    .
	[2] = "\e[0;31mo\e[0m",     //        o (lcs)    .
	[3] = "\e[2;31mo\e[0m",     //    dim o (lcs) [oldest]
	[4] = "\e[1;31;44mO\e[0m",  // bright O (ucs) on blue background
};

const char* const  plx[5] = {
	[0] = "\e[1;36mX\e[0m",     // bright X (ucs) [newest]
	[1] = "\e[0;36mX\e[0m",     //        X (ucs)    .
	[2] = "\e[0;36mx\e[0m",     //        x (lcs)    .
	[3] = "\e[2;36mx\e[0m",     //    dim x (lcs) [oldest]
	[4] = "\e[1;36;44mX\e[0m",  // bright X (ucs) on blue background
};

const char* const  ple = " "; //\e[0;37m.\e[0m";

const char* const  who (board_s* bp,  int pos)
{
	int  pl = occupier(bp, pos);
	if (!pl)  return ple ;  // empty
	pl ^= (g.par *3);       // flip winner if parity is odd

	// Current move
	if ((bp->seq &0xF) == pos)
		return (pl == 1) ? plo[4] : plx[4] ;

	// normal game
	if (g.hide || (g.loop == 9))
		return (pl == 1) ? plo[0] : plx[0] ;

	// shade by sequence
	int  seq;
	for (seq = 0;  seq < 4;  seq++)
		if ( (((bp->seq >> (((seq*2)  ) *4)) & 0xF) == pos) ||
		     (((bp->seq >> (((seq*2)+1) *4)) & 0xF) == pos)    )  break ;

	return (pl == 1) ? plo[seq] : plx[seq];
}

//+============================================================================ =======================================
seq_t  btrans (seq_t seq,  int cnt,  trans_e tr)
{
	int  trTbl[TR_CNT][9] = {
		//                0       1       2       3       4       5       6       7       8
		[TR_NONE]   = {POS_TL, POS_TC, POS_TR, POS_MR, POS_BR, POS_BC, POS_BL, POS_ML, POS_MC},
		[TR_ROT90]  = {POS_TR, POS_MR, POS_BR, POS_BC, POS_BL, POS_ML, POS_TL, POS_TC, POS_MC},
		[TR_ROT180] = {POS_BR, POS_BC, POS_BL, POS_ML, POS_TL, POS_TC, POS_TR, POS_MR, POS_MC},
		[TR_ROT270] = {POS_BL, POS_ML, POS_TL, POS_TC, POS_TR, POS_MR, POS_BR, POS_BC, POS_MC},
		[TR_FLIPH]  = {POS_TR, POS_TC, POS_TL, POS_ML, POS_BL, POS_BC, POS_BR, POS_MR, POS_MC},
		[TR_FLIPV]  = {POS_BL, POS_BC, POS_BR, POS_MR, POS_TR, POS_TC, POS_TL, POS_ML, POS_MC},
	};

	for (uint32_t  i = 0,  mask = 0xF;  i < cnt;  i++,  mask <<= 4) {
		int  sh  = i *4;
		int  pos = (seq & mask) >>sh;
		seq = (seq & ~mask) | (trTbl[tr][pos] <<sh);
	}
	return seq;
}
