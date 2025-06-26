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
	play sequence
	oxo big
	oxo small

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
