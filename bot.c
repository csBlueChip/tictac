#include  "tictac.h"
#include  "macro.h"
#include  "bot.h"
#include  "gfx.h"

#include  <stdlib.h>

//+============================================================================
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

//+============================================================================
int  bot_jacob (int* in,  int st,  int nd)
{
	int  pick[9] = {0};
	int  pcnt    = 0;
	int  key     = -1;

	int  str[5]  = {C_WIN, C_STRONG, C_FAIR, C_WEAK, C_LOSE};  // strength

	if (g.move > 9)  return key ;

	for (int s = 0;  (s < ARRCNT(str)) && !pcnt;  s++)
		for (int i = st;  i < nd;  i++)
			if (g.pref[i].ink == str[s])  pick[pcnt++] = i ;

	key = pick[rand()%pcnt] +'0';
	if (in)  *in = key;
	return         key;
}

//+============================================================================
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
void  botSetup (void)
{
	g.bot[BOT_NONE]  = (bot_s){.fn=NULL      , .name=" ---  ",  .loop= 0};
	g.bot[BOT_PVP]   = (bot_s){.fn=NULL      , .name=" PvP  ",  .loop=-1};
	g.bot[BOT_JACOB] = (bot_s){.fn=bot_jacob , .name="Jacob ",  .loop= 9};
	g.bot[BOT_DAVID] = (bot_s){.fn=bot_juanin, .name="Juanin",  .loop=-1};

	g.botID = BOT_PVP;
}

