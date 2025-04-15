SRCS=shell.c tokenizer.c ./tests/testExec.c
EXECUTABLES=shell testExec

CC=gcc
CFLAGS=-g -Wall -std=gnu99

OBJS=$(SRCS:.c=.o)

all: $(EXECUTABLES)

shell: shell.o tokenizer.o
	$(CC) $(CFLAGS) shell.o tokenizer.o -o shell

testExec: ./tests/testExec.o
	$(CC) $(CFLAGS) ./tests/testExec.o -o testExec

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)
