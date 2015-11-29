CC=clang
CFLAGS=-pedantic -Werror -Wall -std=c11 -g \
	-Wno-gnu-zero-variadic-macro-arguments

LIBS=crypto pthread stdc++ ssl uuid
AMTLIB=amt-redir-libs/lib/libimrsdkUbuntu.a

amtredird: amt.o cmd.o cmp.o config.o main.o ini.o server.o
	$(CC) $(CFLAGS) $^ -o $@ $(foreach lib,$(LIBS),-l$(lib)) $(AMTLIB)

cmp.o: cmp/cmp.c
	$(CC) $(CFLAGS) -c -o $@ $^

ini.o: inih/ini.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o amtredird
