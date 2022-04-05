COMPILER=gcc
SRC=Code

all: clean client.out server.out

client.out: ${SRC}/client.c
	${COMPILER} ${SRC}/client.c -o $@
server.out: ${SRC}/client.c
	${COMPILER} ${SRC}/server.c -o $@

clean:
	rm -f *.out