#ifndef  CONIO_H_
#define  CONIO_H_

#include  <stdbool.h>

#ifndef BPT
#	define  BPT  __asm__("int3");
#endif

//----------------------------------------------------------------------------- ----------------------------------------
typedef
	enum {
		RPT_UNK         = 0xFF,

		// Terminal size can be requested with : \e[18t
		// Reply will be piped to stdin as     : \e[8;{h};{w}t  ...Height/Width
		// where {h, w} are ASCII-encoded variable-length, decimal values
		//
		// Read "DECISIONS WERE MADE" in the Mouse comments!
		//
		// To stay in line with Mouse Reporting [below] we will report the dimentions as 0xhhww12--
		// We COULD use the bottom byte to add 4 bits to each {h, w} value if we wish, so "--" is "reserved"

		TRPT            = 0x1200,
		TRPT_UNK        = TRPT | RPT_UNK,

		// Adding mouse events.
		//
		// Due to limitations of the old/original/non-SGR system, we WILL be using SGI mode, so:
		// The report format is:  \e [  < {event} ; {x=column} ; {y=row} {m|M}
		//                        ^^^^ == "CSI"    [ALL spaces are for claity - they do NOT exist in real reports]
		// where {event, x, y} are ASCII-encoded variable-length, decimal values
		// ...Terminal Window Top-Left is {1,1}
		//
		// DECISIONS/CHOICES/RESTRICTIONS/LIMITATIONS/WHATEVER WERE MADE !!
		//
		// I have allowed {0..127} for {event} (despite 67 being the biggest number I have found todate)
		// The Top Bit (2^7) of the lowest byte/8bits is used to store the mouse button state
		// Known events are defined as MEV_???  .. This list is (very) incomplete
		//
		// I needed some way to pass the {x, y} of the Event(s)
		// ...which, preferably, didn't break the existing keyboard code
		//
		// The keyboard code is 16bit - and *I think* "everything" has an `int` >=32bit nowdays
		// So: I decided to put the X & Y in the top Word/Short/16bits of the Report
		// Thus: Keyboard events will still be valid on a Word/Short/16bit computer,
		// But: Mouse Reports are require DWord/Long/32bits
		// And: Of course, (without screwing around with "-1"), this means we limit ourselves to 
		//      coordinates in the range {1..255}
		// ...I DID consider 9+7 bits to give {1<=x<=511, 1<=y<=127}, but ultimately decided against it
		//    as I think you are more likely to have 128 lines, than 256 columns.
		// ...I also considered 64 bit values, but I do a lot of microcontroller work - so: No.
		// Hence: Any screen coordinate > 255 will wrap/overflow (ie. 255+1 == 0)
		//
		// Mouse events are reported as keystrokes (because they are!!)  .. NB. "CSI" --> \e[
		//     CSI < event ; column ; row {m|M}  ...  CSI -> \e[  ...  {'m'=Button-Up, 'M'=Everything else}
		//
		// See below for some useful MACROs

		MRPT            = 0x1100,
		MRPT_UNK        = MRPT | RPT_UNK,  // 0xFF

		// The Mouse Reporting mode you select ..  eg. `mouse(MRPT_ALL);`
		MRPT_NONE       = MRPT | 0xFE,
		MRPT_CLICK      = MRPT | 0xFD,
		MRPT_DRAG       = MRPT | 0xFC,
		MRPT_ALL        = MRPT | 0xFB,

		// Mouse Event values
		MEV_BTN_L       = 0,
		MEV_BTN_M       = 1,
		MEV_BTN_R       = 2,

		MEV_WHL_UP      = 64,
		MEV_WHL_DOWN    = 65,
		MEV_WHL_LEFT    = 66,
		MEV_WHL_RIGHT   = 67,

		MEV_DRG_L       = 32,
		MEV_DRG_M       = 33,
		MEV_DRG_R       = 34,

		MEV_POS         = 35,

		// Mouse Report Events - this is what you will see in a Report
		MRPT_BTN_DOWN   = MRPT,                       // "M"
		MRPT_BTN_UP     = MRPT | 0x80,                // "m" (lcs)

		MRPT_BTN_L_DOWN = MRPT_BTN_UP   | MEV_BTN_L,  // "M"
		MRPT_BTN_L_UP   = MRPT_BTN_DOWN | MEV_BTN_L,  // "m" (lcs)

		MRPT_BTN_M_DOWN = MRPT_BTN_UP   | MEV_BTN_M,  // "M"
		MRPT_BTN_M_UP   = MRPT_BTN_DOWN | MEV_BTN_M,  // "m" (lcs)

		MRPT_BTN_R_DOWN = MRPT_BTN_UP   | MEV_BTN_R,  // "M"
		MRPT_BTN_R_UP   = MRPT_BTN_DOWN | MEV_BTN_R,  // "M"

		MRPT_WHL_UP     = MRPT | MEV_WHL_UP,          // "M"
		MRPT_WHL_DOWN   = MRPT | MEV_WHL_DOWN,        // "M"
		MRPT_WHL_LEFT   = MRPT | MEV_WHL_LEFT,        // "M"
		MRPT_WHL_RIGHT  = MRPT | MEV_WHL_RIGHT,       // "M"

		MRPT_DRG_L      = MRPT | MEV_DRG_L,           // "M"
		MRPT_DRG_M      = MRPT | MEV_DRG_M,           // "M"
		MRPT_DRG_R      = MRPT | MEV_DRG_R,           // "M"

		MRPT_POS        = MRPT | MEV_POS,             // "M"

		// ------------------------------------------------
		// System paramters
		KEY_MAX   = 5,       // longest keyboard 'scancode'

		KEY_TIMER = 200000,  // multikey timeout - 1000uS = 1mS

		// ------------------------------------------------
		// System messages
		KEYM_SYS  = 0xFF00,

		KEY_NONE  = KEYM_SYS | 0x01,
		KEY_WAIT  = KEYM_SYS | 0x02,
		KEY_TRUNC = KEYM_SYS | 0x03,
		KEY_UNK   = KEYM_SYS | 0x99,
		KEY_ERR   = KEYM_SYS | 0xFF,

		// ------------------------------------------------
		// International keys
		KEYM_INTL = 0x0C00,

		KEY_GBP   = KEYM_INTL | 0x01,

		// ------------------------------------------------
		// Cursors
		KEYM_CUR  = 0x0100,

		KEY_LEFT  = KEYM_CUR | 0x08,
		KEY_DOWN  = KEYM_CUR | 0x04,
		KEY_UP    = KEYM_CUR | 0x02,
		KEY_RIGHT = KEYM_CUR | 0x01,

		KEY_HOME  = KEYM_CUR | 0x18,
		KEY_PGDN  = KEYM_CUR | 0x14,
		KEY_PGUP  = KEYM_CUR | 0x12,
		KEY_END   = KEYM_CUR | 0x11,

		KEY_INS   = KEYM_CUR | 0x21,
		KEY_DEL   = KEYM_CUR | 0x22,  // Key: Del (aka. Delete) | No ASCII value

		// ------------------------------------------------
		// Function keys
		KEYM_FN   = 0x0200,

		KEY_F1    = KEYM_FN |  1,
		KEY_F2    = KEYM_FN |  2,
		KEY_F3    = KEYM_FN |  3,
		KEY_F4    = KEYM_FN |  4,
		KEY_F5    = KEYM_FN |  5,
		KEY_F6    = KEYM_FN |  6,
		KEY_F7    = KEYM_FN |  7,
		KEY_F8    = KEYM_FN |  8,
		KEY_F9    = KEYM_FN |  9,
		KEY_F10   = KEYM_FN | 10,
		KEY_F11   = KEYM_FN | 11,
		KEY_F12   = KEYM_FN | 12,
		KEY_F13   = KEYM_FN | 13,
		KEY_F14   = KEYM_FN | 14,
		KEY_F15   = KEYM_FN | 15,
		KEY_F16   = KEYM_FN | 16,
		KEY_F17   = KEYM_FN | 17,
		KEY_F18   = KEYM_FN | 18,
		KEY_F19   = KEYM_FN | 19,
		KEY_F20   = KEYM_FN | 20,

		// ------------------------------------------------
		// === ASCII keys ===

		// Ctrl keys      dec     ^key  alt   ASCII   C   Name           Signal
		//                ---     ----  ---   -----  ---  ----------     ------
		KEY_CTRL_QM     = 127,  // ^?   <--   DEL
		KEY_CTRL_AT     =   0,  // ^@         NUL    \0   NUL

		KEY_CTRL_A      =   1,  // ^A         SOH
		KEY_CTRL_B      =   2,  // ^B         STX
		KEY_CTRL_C      =   3,  // ^C         ETX                        [intr]
		KEY_CTRL_D      =   4,  // ^D         EOT                        [eof]

		KEY_CTRL_E      =   5,  // ^E         ENQ
		KEY_CTRL_F      =   6,  // ^F         ACK
		KEY_CTRL_G      =   7,  // ^G         BEL    \a   Alarm
		KEY_CTRL_H      =   8,  // ^H         BS     \b   Backspace

		KEY_CTRL_I      =   9,  // ^I   ->|   TAB    \t   Tab
		KEY_CTRL_J      =  10,  // ^J         LF     \n   Newline
		KEY_CTRL_K      =  11,  // ^K         VT     \v   Vertical-Tab
		KEY_CTRL_L      =  12,  // ^L         FF     \f   Form Feed

		KEY_CTRL_M      =  13,  // ^M         CR     \r   Return
		KEY_CTRL_N      =  14,  // ^N         SO
		KEY_CTRL_O      =  15,  // ^O         SI                         [discard]
		KEY_CTRL_P      =  16,  // ^P         DLE

		KEY_CTRL_Q      =  17,  // ^Q         DC1                        [start]
		KEY_CTRL_R      =  18,  // ^R         DC2                        [rprnt]
		KEY_CTRL_S      =  19,  // ^S         DC3                        [stop]
		KEY_CTRL_T      =  20,  // ^T         DC4

		KEY_CTRL_U      =  21,  // ^U         NAK                        [kill]
		KEY_CTRL_V      =  22,  // ^V         SYN                        [lnext]
		KEY_CTRL_W      =  23,  // ^W         ETB                        [werase]
		KEY_CTRL_X      =  24,  // ^X         CAN

		KEY_CTRL_Y      =  25,  // ^Y         EM
		KEY_CTRL_Z      =  26,  // ^Z         SUB                        [susp]

		KEY_CTRL_BRA    =  27,  // ^[         ESC    \e   escape
		KEY_CTRL_BSLS   =  28,  // ^\         FS                         [quit]
		KEY_CTRL_KET    =  29,  // ^]         GS
		KEY_CTRL_CFLEX  =  30,  // ^^         RS
		KEY_CTRL_USCORE =  31,  // ^_         US

		KEY_BKSP        = KEY_CTRL_QM,   // <-- (aka. Backspace) ... ASCII(DEL) [not ASCII(BS)]
		KEY_TAB         = KEY_CTRL_I,
		KEY_ESC         = KEY_CTRL_BRA,
		KEY_RETURN      = KEY_CTRL_M,

	}
keycap_t;

//============================================================================= ========================================
#define  MOUSE_ISRPT(r)   ( ((r) & 0x0000FF00) == MRPT)
#define  MOUSE_EVENT(r)   (  (r) & 0x0000007F )
#define  MOUSE_ISUP(r)    (  (r) & 0x00000080 )        // only relevant to buttons
#define  MOUSE_ISDOWN(r)  ( !MOUSE_ISUP(r) )
#define  MOUSE_X(r)       ( ((r) >>16) & 0xFF )
#define  MOUSE_Y(r)       ( ((r) >>24) & 0xFF )

#define  TERM_ISRPT(r)    ( ((r) & 0x0000FF00) == TRPT)
#define  TERM_W(r)        ( MOUSE_X(r) )
#define  TERM_H(r)        ( MOUSE_Y(r) )

#define  KEY_ISSYS(w)     ( ((w) & 0x0000FF00) == KEYM_SYS  )
#define  KEY_ISINTL(w)    ( ((w) & 0x0000FF00) == KEYM_INTL )
#define  KEY_ISCUR(w)     ( ((w) & 0x0000FF00) == KEYM_CUR  )
#define  KEY_ISFN(w)      ( ((w) & 0x0000FF00) == KEYM_FN   )

//============================================================================= ========================================
bool  kbdInit (void) ;
bool  kbdKill (void) ;

bool  kbhit   (void) ;
int   getch   (void) ;
int   getchw  (void) ;

bool  mouse   (int mode) ;

bool  termGet     (int* rpt) ;
bool  termSet     (int w,  int h) ;
bool  termSave    (int* rpt) ;
bool  termRestore (int* rpt) ;

void  curon  (void) ;
void  curoff (void) ;

#endif //CONIO_H_

