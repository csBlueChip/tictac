//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// qv. https://stackoverflow.com/a/448982/2476535

#include  <stdbool.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  <sys/time.h>
#include  <string.h>
#include  <unistd.h>
#include  <sys/select.h>
#include  <termios.h>
#include  <sys/time.h>

#include  "conio.h"

//----------------------------------------------------------------------------- ----------------------------------------
// Save original state
//
static  int             kbdLive = 0;          // Driver is live and running
static  struct termios  kbdOrig;              // Original console status
static  int             mMode   = MRPT_NONE;  // mouse mode MRPT_{NONE|CLICK|DRAG|ALL}
static  int             termW   = 0;
static  int             termH   = 0;

//+============================================================================ ========================================
// Cursor ON
//
void  curon (void)
{
	printf("\e[?25h");
	fflush(stdout);
}

//+============================================================================
// Cursor OFF
//
void  curoff (void)
{
	printf("\e[?25l");
	fflush(stdout);
}

//+============================================================================ ========================================
/*
static inline
unsigned int  tmDiff_ms (struct timeval t0,  struct timeval t1)
{
    return ((t1.tv_sec - t0.tv_sec) *1000) + ((t1.tv_usec - t0.tv_usec) / 1000);
}
*/

//+============================================================================
// Retrieve terminal-window dimensions
//
// The reply is redirected to stdin (NOT stdout)
//
bool  termGet (int* rpt)
{
	printf("\e[18t");
	fflush(stdout);

	if (rpt) {
		int  r;
		*rpt = 0;

		struct timeval  st;
		gettimeofday(&st, NULL);

		while (!TERM_ISRPT((r = getchw()))) {
			struct timeval  now;
			gettimeofday(&now, NULL);
			if ( (((now.tv_sec - st.tv_sec) *1000) + ((now.tv_usec - st.tv_usec) /1000)) > TERM_TMO_MS )
				return false ;
		};
		return (*rpt = r), true ;
	}

	return true;
}

//+============================================================================
bool  termSet (int w,  int h)
{
	if ((w < 1) || (h < 1))  return false ;

	printf("\e[8;%d;%dt", h, w);
	fflush(stdout);

	int  r;
	termGet(&r);

	return ((TERM_W(r) == w) && (TERM_H(r) == h)) ;
}

//+============================================================================
bool  termSave (int* rpt)
{
	int r;
	if (!termGet(&r))  return false ;

	termW = TERM_W(r);
	termH = TERM_H(r);

	if (rpt)  *rpt = r ;
	return true;

}

//+============================================================================
bool  termRestore (int* rpt)
{
	printf("\e[8;%d;%dt", termW, termH);
	fflush(stdout);

	if (rpt)  return termGet(rpt) ;

	return true;
}

//+============================================================================ ========================================
// Is there a keystroke waiting?
//
bool  kbhit (void)
{
	struct timeval  tv  = { 0L, 0L };
	fd_set          fds;

	if (!kbdLive)  return 0 ;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	return (select(1, &fds, NULL, NULL, &tv) > 0);
}

//+============================================================================ ========================================
// Get the oldest keystroke from srdin
//
int  getch (void)
{
	int            r;
	unsigned char  c;

	if (!kbdLive)  return KEY_ERR ;

	return ((r = read(0, &c, sizeof(c))) < 0) ?  r : c ;
}

//+============================================================================ ========================================
// This will resolve multi-keycode keystrokes
// ...it is very possible/probable this is (very) incomplete - please report omissions!
//
int  getchw (void)
{
	static  int              buf[KEY_MAX] = {0};  // keystroke buffer
	static  int*             ip           = buf;  // current input pointer

	// used my mouse & terminal reports to track decimal values
	static  int              rVal[3]      = {0};  // report values
	static  int              rIdx         = -2;   // value index  -2 not running, -1 start request

	static  struct  timeval  st           = {0};  // timeout timers
	        struct  timeval  nd           = {0};  // ...

	if (!kbdLive)  return KEY_ERR ;

	if (!kbhit()) {
		// empty buffer?
		if (ip == buf)  return KEY_NONE ;

		// timeout?
		gettimeofday(&nd, NULL);  // check timer
		if (((nd.tv_sec * 1000000) + nd.tv_usec) - 
		    ((st.tv_sec * 1000000) + st.tv_usec) >= KEY_TIMER) {
			if (ip == buf + 1)  return (ip = buf),  *buf ;                     //  1 chr waiting
			else                return (ip = buf),  (rIdx = -2),  KEY_TRUNC ;  // >1 chr waiting
		}

		// wait some more
		return KEY_WAIT;

	// keystroke available
	} else {
		int  chr;

		if ((chr = getch()) < 0)  return (ip = buf),  chr ;
		else                      *ip = chr ;

		gettimeofday(&st, NULL);

		// --------------------------------------------------------------------
		// We are reading a Mouse/Terminal Report
		// --------------------------------------------------------------------
		if (rIdx >= -1) {      // A Report is incoming
			static  int  rType = 0;
			if (rIdx == -1) {  // Setup pending
				memset(rVal, 0, sizeof(rVal));
				switch (ip[-1]) {
					case '8' : rType = TRPT;  break ;  // Terminal
					case '<' : rType = MRPT;  break ;  // Mouse
					badMsg   : // 'cos I love trolling the "purists"
					default  : return (ip = buf),  (rIdx = -2), (rType | RPT_UNK) ;
				}
				rIdx++;
			}

			// Mouse:    ^[[<  {e};{x};{y}{M|m}    [sans spaces]
			// Terminal: ^[[8     ;{x};{y}t
			if (*ip == ';')  {
				if (rIdx >= 2)  goto badMsg ;                // too many args
				else            return  rIdx++,  KEY_WAIT ;  // next param
			}

			if ( (*ip == 't') || ((*ip | 0x20) == 'm') ) {   // end marker
				if (rIdx != 2)  goto badMsg ;                // too few args
				int  eom = *ip;
				return (ip = buf),  (rIdx = -2),  (0         // Build+Send the report
					| ((rVal[2] & 0xFF) <<24)                //   y/h
					| ((rVal[1] & 0xFF) <<16)                //   x/w
					| rType                                  //   mouse/terminal
					| ((eom == 'm') ? 0x80 : 0)              //   button UP event
					| (*rVal & 0x7F)                         //   event
					) ;
			}

			if ((*ip < '0') || (*ip > '9'))  goto badMsg ;   // only digits allowed
			rVal[rIdx] = (rVal[rIdx] *10) + (*ip -'0');      // tally the value

			return KEY_WAIT ;
		}
		// --------------------------------------------------------------------
		// --------------------------------------------------------------------

		// Key event
		switch (ip - buf) {
			case 0 :  // 1st character
				if      (*buf == '\e')  return ip++,  KEY_WAIT ;  // esc
				else if (*buf <= 0x7F)  return *buf;              // ASCII
				else if (*buf == 0xC2)  return ip++,  KEY_WAIT ;  // £ [UK shift+3] == 0xC2, 0xA3
				else                    return KEY_UNK;

			case 1 :  // 2nd character
				// £ [UK shift+3] == 0xC2, 0xA3
				if (ip[-1] == 0xC2) {
					if (*ip == 0xA3)  return (ip = buf),  KEY_GBP ;
					else              return (ip = buf),  KEY_UNK ;
				}

				//  \e \e  ->  esc
				//  \e [   -> CSI  ... be warned this is identical to 'Alt ['

				//  \e A   -> ALT A
				if ((ip[-1] == '\e') && ((*ip == '\e') || (*ip != '[' )))
					return (ip = buf),  KEY_ALT(chr) ;
				else  // we got \e[
					return ip++,  KEY_WAIT ;  // CSI ...wait for more

			// CSI detected
			// from here we have \e[***
			case 2 :  // 3rd character
				switch (*ip) {
					case 'A' :  return (ip = buf),  KEY_UP ;
					case 'B' :  return (ip = buf),  KEY_DOWN ;
					case 'C' :  return (ip = buf),  KEY_RIGHT ;
					case 'D' :  return (ip = buf),  KEY_LEFT ;
					case 'H' :  return (ip = buf),  KEY_HOME ;   // Kali
					case 'F' :  return (ip = buf),  KEY_END ;    // Kali
					case 'O' :  //...(capital oh)                // Kali
					case '1' :  //...
					case '2' :  //...
					case '3' :  //...
					case '4' :  //...
					case '5' :  //...
					case '6' :  return  ip++,  KEY_WAIT ;
					case '8' :  return  ip++,  (rIdx = -1),  KEY_WAIT ;
					case '<' :  return  ip++,  (rIdx = -1),  KEY_WAIT ;
					default:    return (ip = buf),  KEY_UNK ;
				}

			case 3 :  // 4th character
				if (*ip == '~')  switch (ip[-1]) {  // (*, tilde)
					case '1' :  return (ip = buf),  KEY_HOME ;
					case '2' :  return (ip = buf),  KEY_INS ;
					case '3' :  return (ip = buf),  KEY_DEL ;
					case '4' :  return (ip = buf),  KEY_END ;
					case '5' :  return (ip = buf),  KEY_PGUP ;
					case '6' :  return (ip = buf),  KEY_PGDN ;
					default:    return (ip = buf),  KEY_UNK ;
				}

				if (ip[-1] == 'O')  switch (*ip) {  // {capital oh, *}
					case 'P' :  return (ip = buf),  KEY_F1 ;  // Kali
					case 'Q' :  return (ip = buf),  KEY_F2 ;  // Kali
					case 'R' :  return (ip = buf),  KEY_F3 ;  // Kali
					case 'S' :  return (ip = buf),  KEY_F4 ;  // Kali
					default:    return (ip = buf),  KEY_UNK ;
				}

				switch (ip[-1]) {
					case '1' :
						if ((*ip <= '0') || (*ip >  '9') || (*ip == '6'))
							return (ip = buf),  KEY_UNK ;
						else
							return ip++,  KEY_WAIT ;
					case '2' :
						if ((*ip <  '0') || (*ip >  '9') || (*ip == '2') || (*ip == '7'))
							return (ip = buf),  KEY_UNK ;
						else
							return ip++,  KEY_WAIT ;
					case '3' :
						if ((*ip <= '0') || (*ip >  '4'))
							return (ip = buf),  KEY_UNK ;
						else
							return ip++,  KEY_WAIT ;
					default :
						return (ip = buf),  KEY_UNK ;
				}

			case 4 :  // 5th character
				if (*ip != '~')  return (ip = buf),  KEY_UNK ;
				switch ((ip[-2] << 8) | ip[-1]) {
					case (('1' << 8) | '1') :  return (ip = buf),  KEY_F1 ;
					case (('1' << 8) | '2') :  return (ip = buf),  KEY_F2 ;
					case (('1' << 8) | '3') :  return (ip = buf),  KEY_F3 ;
					case (('1' << 8) | '4') :  return (ip = buf),  KEY_F4 ;
					case (('1' << 8) | '5') :  return (ip = buf),  KEY_F5 ;
					case (('1' << 8) | '7') :  return (ip = buf),  KEY_F6 ;
					case (('1' << 8) | '8') :  return (ip = buf),  KEY_F7 ;
					case (('1' << 8) | '9') :  return (ip = buf),  KEY_F8 ;
					case (('2' << 8) | '0') :  return (ip = buf),  KEY_F9 ;
					case (('2' << 8) | '1') :  return (ip = buf),  KEY_F10 ;
					case (('2' << 8) | '3') :  return (ip = buf),  KEY_F11 ;
					case (('2' << 8) | '4') :  return (ip = buf),  KEY_F12 ;
					case (('2' << 8) | '5') :  return (ip = buf),  KEY_F13 ;
					case (('2' << 8) | '6') :  return (ip = buf),  KEY_F14 ;
					case (('2' << 8) | '8') :  return (ip = buf),  KEY_F15 ;
					case (('2' << 8) | '9') :  return (ip = buf),  KEY_F16 ;
					case (('3' << 8) | '1') :  return (ip = buf),  KEY_F17 ;
					case (('3' << 8) | '2') :  return (ip = buf),  KEY_F18 ;
					case (('3' << 8) | '3') :  return (ip = buf),  KEY_F19 ;
					case (('3' << 8) | '4') :  return (ip = buf),  KEY_F20 ;
					default : return (ip = buf),  KEY_UNK ;
				}

			default :
				return (ip = buf),  KEY_UNK ;
		}
	}
}

//+============================================================================ ========================================
// EN/DISable mouse reports
//    MRPT_NONE < MRPT_CLICK < MRPT_DRAG < MPRT_ALL
//    disable     click        click+drag  click+drag+posn

static  const char* const  mrptOld = "\e[?1006l";  // old method (limitations)
static  const char* const  mrptSGR = "\e[?1006h";  // new method

static  const char* const  mrptClk = "\e[?1000h";  // Click events only
static  const char* const  mrptDrg = "\e[?1002h";  // + Drag events
static  const char* const  mrptPos = "\e[?1003h";  // + movement/position events

static  const char* const  mrptOff = "\e[?1003l";  // can use: {1000, 1002, 1003} "l"

#define  BPT  __asm__("int3");
bool  mouse (int mode)
{
	if (mMode == mode)  return true ;

	if (mMode != MRPT_NONE)  printf(mrptOff) ;

	if (mode != MRPT_NONE) {
		// Mouse reports are returned as keystrokes
		// So the keyboard needs to be in ConIO mode
		if (!kbdLive)  (void)kbdInit() ;

		// AFAICT, the only way to know the "current" mode is to
		//         wait for a mouse event, and see what type it was ...For so MANY reasons: Sod that!
		printf(mrptSGR);  // We need the mouse reports to be in SGR mode

		// I think I'm funny!
		switch (mode) {
			case MRPT_CLICK :  printf(mrptClk);  goto after;
			case MRPT_DRAG  :  printf(mrptDrg);  goto after;
			case MRPT_ALL   :  printf(mrptPos);
			after           :  mMode = mode;
			default         :  break;
		}
	}
	else  mMode = MRPT_NONE ;

	// We need the cursor to be on
	// There is (I know of) no way to know its initial state - so: oh well!
	curon();

	fflush(stdout);
	return true;
}

//+============================================================================ ========================================
// Put things back as we found them
// ...this will automatically be called with atexit()
//
static
bool  kbdReset (void)
{
	if (!kbdLive)  return false ;

	tcsetattr(0, TCSANOW, &kbdOrig);
	kbdLive = 0;

	return true;
}

//+============================================================================ ========================================
// Reset stdin as we found it
//
bool  kbdKill (void)
{
	// Killing ConIO will break mouse reporting - so disable it!
	if (mMode != MRPT_NONE)  mouse(MRPT_NONE) ;

	return kbdReset() ;
}

//+============================================================================ ========================================
// This trampoline allows us to hook kbdKill with atexit();
//
void  _kbdKill (void)  {  (void)kbdKill();  }

//+============================================================================ ========================================
// Set stdin [file handle #0] to run in "raw" mode
// ...so we can read individual keystrokes
//
static
bool  kbdConIO (void)
{
	struct termios  kbdNew;

	// Backup the current settings
	tcgetattr(0, &kbdOrig);
	memcpy(&kbdNew, &kbdOrig, sizeof(kbdNew));

	// Make sure we clean up when we leave
	atexit(_kbdKill);

	// Set to single character mode
	cfmakeraw(&kbdNew);
	tcsetattr(0, TCSANOW, &kbdNew);
	kbdLive = 1;

	return true;
}

//+============================================================================
// Configure stdin as raw, and add the reset code to the atexit() stack
//
bool  kbdInit (void)
{
	if (kbdLive)  return true ;

	kbdConIO();

	return true;
}



















//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// I thought it might be nice to leave this here as an example use case...
//+============================================================================ ========================================
#if 0
bool  cb_play (mplayer_t* pPLy,  smf_t* pSmf)
{
	static int  sel = 0;
	       int  old;
	       int  ch;

	switch ((ch = getchw())) {
		case CTRL('C') :
			drawEos();
			return false ;

		case '1'     :  _kbd[ 0].mute = !_kbd[ 0].mute;  drawKbd(&_kbd[ 0]);  break ;
		case '2'     :  _kbd[ 1].mute = !_kbd[ 1].mute;  drawKbd(&_kbd[ 1]);  break ;
		case '3'     :  _kbd[ 2].mute = !_kbd[ 2].mute;  drawKbd(&_kbd[ 2]);  break ;
		case '4'     :  _kbd[ 3].mute = !_kbd[ 3].mute;  drawKbd(&_kbd[ 3]);  break ;
		case '5'     :  _kbd[ 4].mute = !_kbd[ 4].mute;  drawKbd(&_kbd[ 4]);  break ;
		case '6'     :  _kbd[ 5].mute = !_kbd[ 5].mute;  drawKbd(&_kbd[ 5]);  break ;
		case '7'     :  _kbd[ 6].mute = !_kbd[ 6].mute;  drawKbd(&_kbd[ 6]);  break ;
		case '8'     :  _kbd[ 7].mute = !_kbd[ 7].mute;  drawKbd(&_kbd[ 7]);  break ;
		case '9'     :  _kbd[ 8].mute = !_kbd[ 8].mute;  drawKbd(&_kbd[ 8]);  break ;
		case '0'     :  _kbd[ 9].mute = !_kbd[ 9].mute;  drawKbd(&_kbd[ 9]);  break ;
		case '!'     :  _kbd[10].mute = !_kbd[10].mute;  drawKbd(&_kbd[10]);  break ;
		case '"'     :  _kbd[11].mute = !_kbd[11].mute;  drawKbd(&_kbd[11]);  break ;
		case KEY_GBP :  _kbd[12].mute = !_kbd[12].mute;  drawKbd(&_kbd[12]);  break ;  // £ == {194,163}
		case '$'     :  _kbd[13].mute = !_kbd[13].mute;  drawKbd(&_kbd[13]);  break ;
		case '%'     :  _kbd[14].mute = !_kbd[14].mute;  drawKbd(&_kbd[14]);  break ;
		case '^'     :  _kbd[15].mute = !_kbd[15].mute;  drawKbd(&_kbd[15]);  break ;

		case '_'     :                          midpSpeed(pSmf, 100            ) ;  break ;
		case '-'     :  if (pSmf->speed > 1  )  midpSpeed(pSmf, pSmf->speed - 1) ;  break ;
		case '='     :  if (pSmf->speed < 300)  midpSpeed(pSmf, pSmf->speed + 1) ;  break ;

		case ' ' :  // Pause|Continue
			if (pSmf->running) {
				if (pSmf->paused) {
					intrCont(pSmf);
					pSmf->paused = false;
				} else {
					intrPause(pSmf);
					midpPanic(pSmf);
					pSmf->paused = true;
					LOGF("<Paused>");
				}
			}
			break;
			
		case 'm' :  _kbd[sel].mute = !_kbd[sel].mute;  drawKbd(&_kbd[sel]);  break ;

		case KEY_DOWN:
			if (sel < 15) {
				sel++;
				drawSel(&_kbd[sel - 1], &_kbd[sel]);
			}
			break;
			
		case KEY_UP:
			if (sel > 0) {
				sel--;
				drawSel(&_kbd[sel + 1], &_kbd[sel]);
			}
			break;
			
		case KEY_PGUP:
			old = sel;
			sel = 0;
			drawSel(&_kbd[old], &_kbd[sel]);
			break;
			
		case KEY_PGDN:
			old = sel;
			sel = 15;
			drawSel(&_kbd[old], &_kbd[sel]);
			break;
			
		case KEY_NONE: 
		case KEY_WAIT: 
		case KEY_TRUNC: 
		case KEY_UNK: 
		case KEY_ERR: 
		default:
			break;
	}

	return true;  // return 'false' to stop playback
}
#endif