C=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LDLIBS=-lpthread -pthread -lrt
proj2: proj2.c
clean:
	-rm proj2

