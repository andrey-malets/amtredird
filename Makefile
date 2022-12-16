CC=clang
INCLUDE=amt-redir-libs/include
CFLAGS=-pedantic -Werror -Wall -std=c11 -g \
	-Wno-gnu-zero-variadic-macro-arguments \
	-I$(INCLUDE) -D_POSIX_SOURCE -D_POSIX_C_SOURCE=201500 -O2

STDLIBS=dl pthread stdc++ uuid
AMTLIB=amt-redir-libs/lib/libimrsdkUbuntu.a
SSLLIBS=amt-redir-libs/lib/libssl.a amt-redir-libs/lib/libcrypto.a

all: amtredird sol

amtredird: amt.o cmd.o cmp.o config.o main.o ini.o server.o ssl.o
	$(CC) $(CFLAGS) -o $@ $^ \
		$(AMTLIB) $(SSLLIBS) \
		$(foreach lib,$(STDLIBS),-l$(lib))

sol: amt.o cmd.o cmp.o config.o ini.o sol.o ssl.o
	$(CC) $(CFLAGS) -o $@ $^ \
		$(AMTLIB) $(SSLLIBS) \
		$(foreach lib,$(STDLIBS),-l$(lib))

cmp.o: cmp/cmp.c
	$(CC) $(CFLAGS) -c -o $@ $^

ini.o: inih/ini.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o amtredird sol
