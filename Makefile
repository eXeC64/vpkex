.PHONY: clean

vpkex: vpkex.c
	gcc -o vpkex vpkex.c

clean:
	rm vpkex
