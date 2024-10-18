CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -lrt -g
LDFLAGS = -Wall -Wextra -Werror -pthread -lrt
LIBS = -lm

.DEFAULT_GOAL := all

.PHONY: all run clean ulimit

all: proj2

#pro volání make run
run: ulimit all
	./proj2 19999 10 100 10000 1000
# odstranění souborů
clean:
	rm -f proj2 proj2.o

proj2: proj2.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

proj2.o: proj2.c 
	$(CC) $(CFLAGS) -c -o $@ $^

