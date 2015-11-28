CC=clang
CFLAGS=-pedantic -Werror -Wall -std=c11 -g

amtredird: config.o main.o ini.o
	$(CC) $(CFLAGS) -o $@ $^

ini.o: inih/ini.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o amtredird
