CC=gcc
CFLAGS= -g -Werror -Wall
BIN=320sh
MODULES=part1.c utils.c part2.c linkedlist.c stack.c job_control_list.c job_control.c

all: $(BIN)

clean:
	rm -f *.o *.out $(BIN)

$(BIN): clean
	$(CC) $(CFLAGS) -o $(BIN) $(BIN).c $(MODULES)

compiledebug: clean
	$(CC) $(CLFAGS) -DDEBUG -o $(BIN) $(BIN).c $(MODULES)

run: $(BIN)
	. ./launcher.sh

# required debug
d: $(BIN)
	. ./launcher-d.sh

# testing debug
debug: compiledebug
	./$(BIN)
