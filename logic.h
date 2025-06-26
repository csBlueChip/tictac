#ifndef  LOGIC_H_
#define  LOGIC_H_

#include  "tictac.h"

//----------------------------------------------------------------------------- ---------------------------------------
typedef
	enum trans {
		TR_NONE,
		TR_ROT90,   // +90
		TR_ROT180,  // +180
		TR_ROT270,  // -90
		TR_FLIPH,   // <->
		TR_FLIPV,   // ^-v

		TR_CNT
	}
trans_e;

//----------------------------------------------------------------------------- ---------------------------------------
extern  const char* const  plo[] ;
extern  const char* const  plx[] ;

//============================================================================= =======================================
const char* const  who     (board_s* bp,  int pos) ;
board_s*           findseq (seq_t seq,  int loop) ;
seq_t              btrans  (seq_t seq,  int cnt,  trans_e tr) ;

#endif //LOGIC_H_

