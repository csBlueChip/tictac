------------------------------------------------------------------------------- ---------------------------------------
anal.c - state analysis
	analyse
		simpleMinMax
	lookahead

build.c - move table(s)
	build  - master game table
	loopat - loop points

conio.c - Terminal, keyboard, & mouse routines
	see conio.h

gfx.c - display functions
	menu          - including mouse check
	move options  - including mouse check
	mode options  - including mouse check
	oxo big       - including mouse check

	play sequence - display only
	oxo small     - display only

logic.c - logical operations
	who     - who is in position X of board B
	findseq - find a given sequence in the full game-set
	btrans  - rotate, flip, or mirror a board

macro.h - useful macros
	INRANGE
	MSGF{YX}

tictac.c - the game
	main   - setup stuff
	tictac - main game loop

------------------------------------------------------------------------------- ---------------------------------------
Normal rules:
	There are 986,410 sequences which end with all 9 squares filled  (incl. empty-board)
	       or 549,946 if you stop placing pieces when someone "wins" (incl. empty-board)
	 of which 340,858 sequences end in a draw
	      and 209,088 end with someone winning
	 of which 131,184 will be won by player-1
	      and  77,904 will be won by player-2

	There are 8 winning lines                {top, middle, bottom, left, centre, right, backslash, forward-slash}
	 of which 4 pass through the epicente    {     middle,               centre,        backslash, forward-slash}
	          3 pass through each corner eg. {top,                 left,                backslash,              }
	      and 2 pass through each edge   eg. {top,                       centre,                                }

	There are 6 ways to lay each winning line {123, 132, 213, 231, 312, 321}

	p1, opens with centre : wins[p1] = 15,648 .. wins[p2] =  5,616  .. delta +10,032
	               corner : wins[p1] = 14,652 .. wins[p2] =  7,869  .. delta + 6,756
	                 edge : wins[p1] = 14,232 .. wins[p2] = 10,176  .. delta + 4,056

------------------------------------------------------------------------------- ---------------------------------------
Looping rules:
	A "Loop-N" game is where only (at most) N pieces may be present on a resting board.

	Loop-5 can be found online   : https://xogone.com
	Loop-6 can be found as a toy : https://www.amazon.co.uk/dp/B0DNLYSN7J

	The main difference here is (once the loop point is reached):
		In the Loop-5 game:
			remove oldest piece
			get new move
			check win
		In the Loop-6 game:
			get new move
			remove oldest piece
			check win
	So in the Loop-5 game, you could (theoretically) hold ground on three squares
	whereas the Loop-6 game forces progression to a new square.

	Loop-7 & Loop-8 have been implemented as it was trivial to add this functionality
		The basic game-play functionality works, but no "play-testing" has been done.

	Loop-4,3,2,1 have not been considered.

	"Loop-9" is not a looping game, it is the classic "tic tac toe" game

------------------------------------------------------------------------------- ---------------------------------------
