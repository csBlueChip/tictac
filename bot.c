#include  "tictac.h"
#include  "macro.h"
#include  "bot.h"
#include  "gfx.h"

#include  <stdlib.h>

//+============================================================================ =======================================
static  int  pick[9] = {0};
static  int  pcnt    = 0;

static
int  choose (int st,  int nd,  int c1,  int c2)
{
	pcnt = 0;

	for (int i = st;  i < nd;  i++)
		if ((g.pref[i].ink == c1) || (g.pref[i].ink == c2))  pick[pcnt++] = i ;

	return pcnt;
}

//+============================================================================
static
int  chooseNot (int st,  int nd,  int c1,  int c2)
{
	pcnt = 0;

	for (int i = st;  i < nd;  i++)
		if ((g.pref[i].ink != c1) && (g.pref[i].ink != c2))  pick[pcnt++] = i ;

	return pcnt;
}

//+============================================================================ =======================================
// can only play a normal game - will never lose
//
// FFS, the little shit loses!!  You play: middle-right, bottom-left, bottom-right
// .../ME introduces lose2 analysis in analyse()
//
int  bot_jacob (int* in,  int st,  int nd)
{
	int  col[] = {C_WIN, C_STRONG, C_FAIR, C_WEAK, C_LOSE2, C_LOSE};

	if (g.move > 9)  return -1 ;

	for (int c = 0;  c < ARRCNT(col);  c++)
		if (choose(st, nd, col[c], C_INVALID))  return (*in = pick[rand()%pcnt] +'0') ;

	return -1;
}

//+============================================================================
// CABOT : considers immediate wins & loses, but otherwise play randomly
//
int  bot_cabot (int* in,  int st,  int nd)
{
	if ((g.loop == 9) && (g.move > 9))  return -1 ;

	// take the first win you see
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink == C_WIN)  return (*in = i +'0') ;

	// if you can ONLY play a losing move - just pick one
	if (choose(st, nd, C_LOSE, C_INVALID) == (nd - st))  return (*in = pick[rand()%pcnt] +'0') ;

	// pick something that doesn't lose
	(void)chooseNot(st, nd, C_LOSE, C_INVALID);
	return (*in = pick[rand()%pcnt] +'0') ;
}

//+============================================================================
// DAVID : Cabot + consider WIN2 > FAIR
//
int  bot_david (int* in,  int st,  int nd)
{
	if ((g.loop == 9) && (g.move > 9))  return -1 ;

	// dont miss the chance to win or win2
	if (choose(st, nd, C_WIN, C_WIN2))     return (*in = pick[rand()%pcnt] +'0') ;

	// pick a "strong" or "fair" move
	if (choose(st, nd, C_STRONG, C_FAIR))  return (*in = pick[rand()%pcnt] +'0') ;

	// if you can ONLY play a losing move - just pick one
	if (choose(st, nd, C_LOSE, C_INVALID) == (nd - st))  return (*in = pick[rand()%pcnt] +'0') ;

	// pick something that doesn't lose
	(void)chooseNot(st, nd, C_LOSE, C_INVALID);
	return (*in = pick[rand()%pcnt] +'0') ;
}

//+============================================================================
// WATSON : David + consider LOSE2 < FAIR
//
int  bot_watson (int* in,  int st,  int nd)
{
	if ((g.loop == 9) && (g.move > 9))  return -1 ;

	// dont miss the chance to win now
	if (choose(st, nd, C_WIN, C_INVALID))     return (*in = pick[rand()%pcnt] +'0') ;

	// dont miss the chance to win NEXT time around
	if (choose(st, nd, C_WIN2, C_INVALID))    return (*in = pick[rand()%pcnt] +'0') ;

	// pick a "strong" move
	if (choose(st, nd, C_STRONG, C_INVALID))  return (*in = pick[rand()%pcnt] +'0') ;

	// if you can ONLY play a losing move - just pick one
	// we prefer a lose_2 on the slim chance it might push us past the point of "DRAW"
	if (choose(st, nd, C_LOSE, C_LOSE2) == (nd - st)) {
		if      (choose(st, nd, C_LOSE2, C_INVALID))  return (*in = pick[rand()%pcnt] +'0') ;
		else if (choose(st, nd, C_LOSE,  C_INVALID))  return (*in = pick[rand()%pcnt] +'0') ;
	}

	// if you can ONLY play a losing move - just pick one
	if (choose(st, nd, C_LOSE, C_INVALID) == (nd - st))  return (*in = pick[rand()%pcnt] +'0') ;

	// pick something that doesn't lose
	(void)chooseNot(st, nd, C_LOSE, C_LOSE2);
	return (*in = pick[rand()%pcnt] +'0') ;
}

//+============================================================================
// FALKEN : You will NOT win!
//
//	int  bot_falken (int* in,  int st,  int nd)
//	{
//		return 9;
//	}

//+============================================================================ =======================================
// Activate a Bot/PvP
//
void  botSet (bot_e id)
{
	static  int  prevHide = 1;

//.	if (id == BOT_NONE)  return ;

	if (g.bot[id].fn) {  // isBot
		if (!g.bot[g.botID].fn) {  // wasPVP
			prevHide = g.hide;
			g.hide   = 1;
		}

		if (g.bot[id].loop != -1) {
			g.loop = g.bot[id].loop;
		}

	} else {             // isPVP
		if (g.bot[g.botID].fn) {  // wasBot
			g.hide = prevHide;
		}
	}

	g.botID = id;
	botShow();
}

//+============================================================================ =======================================
// system config
//
void  botSetup (void)
{
	g.bot[BOT_NONE]   = (bot_s){.fn=NULL      , .name=" ---  ",  .loop= 0};
	g.bot[BOT_PVP]    = (bot_s){.fn=NULL      , .name=" PvP  ",  .loop=-1};
	g.bot[BOT_JACOB]  = (bot_s){.fn=bot_jacob , .name="Jacob ",  .loop= 9};
	g.bot[BOT_CABOT]  = (bot_s){.fn=bot_cabot , .name="Cabot ",  .loop=-1};
	g.bot[BOT_DAVID]  = (bot_s){.fn=bot_david , .name="David ",  .loop=-1};
	g.bot[BOT_WATSON] = (bot_s){.fn=bot_watson, .name="Watson",  .loop=-1};
//	g.bot[BOT_FALKEN] = (bot_s){.fn=bot_falken, .name="Falken",  .loop=-1};

	g.botID = BOT_PVP;
}
