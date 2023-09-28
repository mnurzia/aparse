CFLAGS := -g --std=c89 -Wall -Werror -Wextra -pedantic

all: test

test: test.c aparse.c aparse.h
	$(CC) test.c aparse.c $(CFLAGS) -o test

clean:
	rm -rf test
