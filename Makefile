.phony : run
run: 9

.phony: dbg
dbg:
	rm -f brute
	gcc -g -lc tictac.c conio.c gfx.c build.c logic.c anal.c -o tictac

.phony: all
all:
	rm -f brute
	gcc -lc tictac.c conio.c gfx.c build.c logic.c anal.c -o tictac
	strip -s tictac

.phony : 5 6 7 8 9
5 6 7 8 9: all
	./tictac $@ || stty sane >/dev/null 2>&1

