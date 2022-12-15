CC=clang
CFLAGS=-pedantic -Werror -Wall -std=c11 -g \
	-Wno-gnu-zero-variadic-macro-arguments \
	-D_POSIX_SOURCE -D_POSIX_C_SOURCE=201500 -O2

LIBS=ssl crypto pthread stdc++ uuid
AMTLIB=amt-redir-libs/lib/libimrsdkUbuntu.a

amtredird: amt.o cmd.o cmp.o config.o main.o ini.o server.o ssl.o
	$(CC) $(CFLAGS) -o $@ $^ $(AMTLIB) $(foreach lib,$(LIBS),-l$(lib))

sol: amt.o cmd.o cmp.o config.o ini.o sol.o ssl.o
	$(CC) $(CFLAGS) -o $@ $^ $(AMTLIB) $(foreach lib,$(LIBS),-l$(lib))

cmp.o: cmp/cmp.c
	$(CC) $(CFLAGS) -c -o $@ $^

ini.o: inih/ini.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o amtredird sol
