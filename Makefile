CC=clang
CFLAGS=-pedantic -Werror -Wall -std=c11 -g

LIBS=crypto pthread stdc++ ssl uuid
AMTLIB=amt-redir-libs/lib/libimrsdkUbuntu.a

amtredird: amt.o config.o main.o ini.o
	$(CC) $(CFLAGS) $^ -o $@ $(foreach lib,$(LIBS),-l$(lib)) $(AMTLIB)

ini.o: inih/ini.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o amtredird
