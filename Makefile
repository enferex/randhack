APPS = randhack randhack_small
CC = gcc
CFLAGS = -g3 -Wall -pedantic -std=c99 

all: $(APPS)

%: %.c
	$(CC) $(CFLAGS) $^ -o $@

test: $(APPS)
	./randhack

clean:
	rm -fv $(APPS)
