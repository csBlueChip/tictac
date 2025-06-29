#ifndef  BOT_H_
#define  BOT_H_

typedef
	enum bot_e {
		BOT_NONE = 0,  // do not edit

		BOT_PVP,
		BOT_JACOB,
		BOT_DAVID,

		BOT_CNT        // must be last
	}
bot_e;

typedef  int(bot_fn)(int*, int, int) ;

typedef
	struct bot_s {
		bot_fn*  fn;
		char     name[10];
		int      loop;
	}
bot_s;

void  botSetup (void) ;
void  botSet   (bot_e id) ;

#endif //BOT_H_
