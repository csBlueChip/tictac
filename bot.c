#include  "tictac.h"
#include  "macro.h"
#include  "bot.h"
#include  "gfx.h"

#include  <stdlib.h>

//+============================================================================ =======================================
// can only play a normal game - will never lose
//
// FFS, the little shit loses!!  You play: middle-right, bottom-left, bottom-right
// .../ME introduces lose2 analysis in analyse()
//
int  bot_jacob (int* in,  int st,  int nd)
{
	int  pick[9] = {0};
	int  pcnt    = 0;
	int  key     = -1;

	int  str[]   = {C_WIN, C_STRONG, C_FAIR, C_WEAK, C_LOSE2, C_LOSE};  // strength

	if (g.move > 9)  return -1 ;

	for (int s = 0;  (s < ARRCNT(str)) && !pcnt;  s++)
		for (int i = st;  i < nd;  i++)
			if (g.pref[i].ink == str[s])  pick[pcnt++] = i ;

	key = pick[rand()%pcnt] +'0';
	if (in)  *in = key;
	return         key;
}

//+============================================================================
// JUANIN : considers immediate wins & loses, but otherwise play randomly
//
int  bot_juanin (int* in,  int st,  int nd)
{
	int  pick[9] = {0};
	int  pcnt    = 0;

	if ((g.loop == 9) && (g.move > 9))  return -1 ;

	// dont miss the chance to win
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink == C_WIN)  return (*in = i +'0') ;

	// dont lose (if you can avoid it)
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink != C_LOSE)  pick[pcnt++] = i ;

	// all losers?  random:-
	if (!pcnt)  return (*in = st +(rand()%(nd-st)) +'0') ;

	// pick something that doesn't lose
	return  (*in = pick[rand()%pcnt] +'0') ;
}

//+============================================================================
// DAVID : Juanin + consider WIN2 > FAIR
//
int  bot_david (int* in,  int st,  int nd)
{
	int  pick[9] = {0};
	int  pcnt    = 0;

	if ((g.loop == 9) && (g.move > 9))  return -1 ;

	// dont miss the chance to win
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink == C_WIN)  return (*in = i +'0') ;

	// dont miss the chance to win NEXT time around
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink == C_WIN2)  return (*in = i +'0') ;

	// dont lose (if you can avoid it)
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink != C_LOSE)  pick[pcnt++] = i ;

	// all losers?  random:-
	if (!pcnt)  return (*in = st +(rand()%(nd-st)) +'0') ;

	// pick something that doesn't lose
	return  (*in = pick[rand()%pcnt] +'0') ;
}

//+============================================================================
// FALKEN : David + consider LOSE2 < FAIR
//
int  bot_falken (int* in,  int st,  int nd)
{
	int  pick[9]  = {0};
	int  pcnt     = 0;

	if ((g.loop == 9) && (g.move > 9))  return -1 ;

	// dont miss the chance to win now
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink == C_WIN)  return (*in = i +'0') ;

	// eschew all losers
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink != C_LOSE)  pick[pcnt++] = i ;

	// all losers?  random:-
	if (!pcnt)  return (*in = st +(rand()%(nd-st)) +'0') ;

	// dont miss the chance to win NEXT time around
	for (int i = st;  i < nd;  i++)
		if (g.pref[i].ink == C_WIN2)  return (*in = i +'0') ;

	// eschew all lose-2 options
	pcnt = 0;
	for (int i = st;  i < nd;  i++)
		if ((g.pref[i].ink != C_LOSE) && (g.pref[i].ink != C_LOSE2))  pick[pcnt++] = i ;

	// all lose-2?  random:-
	if (!pcnt)  return (*in = st +(rand()%(nd-st)) +'0') ;

	// pick something that doesn't lose
	return  (*in = pick[rand()%pcnt] +'0') ;
}

//+============================================================================ =======================================
// Activate a Bot/PvP
//
void  botSet (bot_e id)
{
	static  int  prevHide = 1;

//	if (id == BOT_NONE)  return ;

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
	g.bot[BOT_JUANIN] = (bot_s){.fn=bot_juanin, .name="Juanin",  .loop=-1};
	g.bot[BOT_DAVID]  = (bot_s){.fn=bot_david , .name="David ",  .loop=-1};
	g.bot[BOT_FALKEN] = (bot_s){.fn=bot_falken, .name="Falken",  .loop=-1};

	g.botID = BOT_PVP;
}
